
// --------------- LIBRARIES FOR CONTROL POWER ------------------- //

#include "axp20x.h"
AXP20X_Class axp;

// ------------------  LIBRARIES FOR LORA    -------------------- //

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

//-------------------  VARIABLES GLOBALES    -------------------- //

uint8_t   NWKSKEY[16], APPSKEY[16], KEY[16], tx_payload[12], tx_payload_1[12], prueba[4];
uint32_t  DEVADDR;

uint8_t   data_request[] = {2, 68, 65, 3};
uint8_t   days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

float     divisor[] = {1000, 100, 10};
bool      flag_ext, flag_timer, flag_rx,  flag_ack;

float     Pm10, PM25, Temp, Hum, Err, Pre, Muestra;

uint8_t   pos, Hor, Min, lenght, Dia, Mes, Ano;
uint16_t  var;
char      data;
String    Cadena;

// ------------------- LMIC - PIN USED -------------------------- //

void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }
static osjob_t sendjob;

const lmic_pinmap lmic_pins = {
  .nss = 18,                        // CS
  .rxtx = LMIC_UNUSED_PIN,
  .rst = 23,                        // RST
  .dio = {26, 33, 32},              // DIO
};


// ------------------ INTERRPCION EXTERNA   ---------------------- //

void IRAM_ATTR tx_prueba() {
  flag_ext = true;
}

// ------------------ INTERRUPCION TIMER OVF --------------------- //

hw_timer_t *My_timer = NULL;

void IRAM_ATTR onTimer() {
  flag_timer = true;
}

#define SERIAL1_RX 13
#define SERIAL1_TX 2
#define Led  4

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600, SERIAL_8N1, SERIAL1_RX, SERIAL1_TX);

  pinMode(Led, OUTPUT);  digitalWrite(Led, HIGH);

  AX192_init();
  Init_Lora();

  pinMode(38, INPUT);
  attachInterrupt(digitalPinToInterrupt(38), tx_prueba, RISING);        // FALLING RISING CHANGE  HIGH  LOW

  My_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(My_timer, &onTimer, true);
  timerAlarmWrite(My_timer, 60000000, true);                            // (80M*SEG)/80
  timerAlarmEnable(My_timer);

  Hor = 7;
  Min = 34;
  Mes = 2;
  Dia = 28;
  Ano = 23;
}

void loop() {
  os_runloop_once();

  if (flag_ext) {
    flag_ext = false;
    if (Muestra != 0) {
      do_send(&sendjob);
    }
  }

  if (flag_timer) {
    flag_timer = false;
    axp.setChgLEDMode(AXP20X_LED_OFF);

    for (uint8_t x = 0; x < 4; x++) {
      Serial1.write(data_request[x]);
    }

    Min++;
    if (Min == 60) {
      Min = 0;
      Hor ++;
      if (Hor == 24) {
        Hor = 0;
        Dia++;
        if (Dia >  days[Mes]) {
          Dia = 1;
          Mes++;
          if (Mes == 13) Mes = 1;
        }
      }
    }

    if (Min == 0) {
      tx_payload_1[0]  = Mes;                  //Mes;
      tx_payload_1[1]  = Dia;                  //Dia;
      tx_payload_1[2]  = Hor;
      tx_payload_1[3]  = Min;

      var = (Pm10 / Muestra) * 100;
      tx_payload_1[4] = ((var & 0xff00) >> 8);
      tx_payload_1[5] =  (var & 0x00ff);

      var = (PM25 / Muestra) * 100;
      tx_payload_1[6] = ((var & 0xff00) >> 8);
      tx_payload_1[7] =  (var & 0x00ff);

      tx_payload_1[8]  = Err;
      tx_payload_1[9]  = (Temp * 1.8) + 32;   //Temperatura;
      tx_payload_1[10] = Hum * 1.2585;        //Humedad;
      tx_payload_1[11] = Pre / 5;             //Humedad;

      flag_ext = true;
    }

    if ((Hor == 0) && (Min == 1)) {
      Muestra = 0; Pm10 = 0; PM25 = 0;
    }

    if ((Min == 15) || (Min == 30) || (Min == 45)) {
      if (tx_payload_1[11] > 0) {
        do_send_1(&sendjob);
      }
    }
  }

  Rx_Data();

  if (flag_ack) {
    axp.setChgLEDMode(AXP20X_LED_BLINK_1HZ);
    Serial.println("--LoRaWAN ACK OK--");
    Muestra = 0; Pm10 = 0; PM25 = 0;
    flag_ack = false;
  }

}



void Rx_Data() {
  while (Serial1.available()) {

    if (Serial1.available() > 0) {
      data = Serial1.read();

      if (data == 3) {
        flag_rx = false;                // END OF DATA RX
        Muestra++;
        pos = 0;
        lcd();
        Cadena = "";
      }

      if (flag_rx)   {                  // SAVE DATA RX
        Save_data();
        pos++;
      }

      if (data == 2) {
        flag_rx = true;                 // START OF DATA RX
        Cadena = "";
      }
    }
  }
}


void Save_data() {

  if ((pos > 39) && (pos < 44)) {
    Cadena +=  data;
  }

  if (pos == 46) {
    uint8_t num = data - 48;
    var = Cadena.toInt();
    PM25 += var / divisor[num];
    Cadena = "";
  }

  if ((pos > 69) && (pos < 74)) {
    Cadena +=  data;
  }

  if (pos == 76) {
    uint8_t num = data - 48;
    var = Cadena.toInt();
    Pm10 += var / divisor[num];
    Cadena = "";
  }

  if ((pos > 99) && (pos < 104)) {
    Cadena +=  data;
  }

  if (pos == 106) {
    uint8_t num = data - 48;
    var = Cadena.toInt();
    Temp = var / divisor[num];
    Cadena = "";
  }

  if ((pos > 129) && (pos < 134)) {
    Cadena +=  data;
  }

  if (pos == 136) {
    uint8_t num = data - 48;
    var = Cadena.toInt();
    Hum = var / divisor[num];
    Cadena = "";
  }

  if ((pos > 159) && (pos < 164)) {
    Cadena +=  data;
  }

  if (pos == 166) {
    uint8_t num = data - 48;
    var = Cadena.toInt();
    Pre = var / divisor[num];
    Cadena = "";
  }

  if ((pos > 219) && (pos < 224)) {
    Cadena +=  data;
  }

  if (pos == 166) {
    uint8_t num = data - 48;
    var = Cadena.toInt();
    Err = var / divisor[num];
    Cadena = "";
  }
}


void lcd() {
  Serial.print("Fecha: ");   Serial.print(Dia);  Serial.print('-');  Serial.print(Mes);       Serial.print('-');  Serial.print(Ano);
  Serial.print(' ');         Serial.print(Hor);  Serial.print(':');  Serial.print(Min);       Serial.print(' ');
  Serial.print("PM10: ");    Serial.print(Pm10 / Muestra, 1);        Serial.print(" PM25: ");      Serial.print(PM25 / Muestra, 1);
  Serial.print(" T&H: ");    Serial.print(Temp, 1); Serial.print(" C "); Serial.print(Hum, 1);  Serial.print(" %");
  Serial.print(" Error: ");  Serial.print(Err); Serial.print(" Presion: ");  Serial.println(Pre);
}
