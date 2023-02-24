// --------------- LIBRARIES FOR CONTROL POWER ------------------- //

#include "axp20x.h"
AXP20X_Class axp;

// ------------------  LIBRARIES FOR LORA    -------------------- //

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

//-------------------  VARIABLES GLOBALES    -------------------- //

uint8_t   NWKSKEY[16], APPSKEY[16], KEY[16], tx_payload[11], tx_payload_1[11], prueba[4];
uint32_t  DEVADDR;

bool      flag_ext, flag_timer, flag_ack, flag_rx, flag_rx_n, flag_rx_p, flag_esp;
uint8_t   lenght, Mes, Dia, Hor, Min, Temperatura, Humedad, Error, Ano, Separador;

float     PM10, PM25, muestra;
String    S_PM10, S_PM25, S_Ano, S_Mes, S_Dia, S_Hor, S_Min, S_Error, S_Temp, S_Hum;
char      data;

uint16_t  var;


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

const uint8_t Led = 4;

void setup() {

  Serial.begin(9600);
  pinMode(Led, OUTPUT);  digitalWrite(Led, HIGH);

  AX192_init();
  Init_Lora();

  pinMode(38, INPUT);
  attachInterrupt(digitalPinToInterrupt(38), tx_prueba, RISING);        // FALLING RISING CHANGE  HIGH  LOW

  My_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(My_timer, &onTimer, true);
  timerAlarmWrite(My_timer, 60000000, true);                            // (80M*SEG)/80
  timerAlarmEnable(My_timer);
  timerStop(My_timer);

}

void loop() {
  os_runloop_once();

  if (flag_ext) {
    if (Mes != 0) {
      timerRestart(My_timer);
      timerStart(My_timer);
      do_send(&sendjob);
    }
    axp.setChgLEDMode(AXP20X_LED_OFF);
    flag_ext   = false;
    flag_timer = false;
  }

  if (flag_ack) {
    axp.setChgLEDMode(AXP20X_LED_BLINK_1HZ);
    Serial.println("--LoRaWAN ACK OK--");
    muestra = 0; PM10 = 0; PM25 = 0;
    flag_ack = false;
  }

  if (flag_rx) {
    for (uint8_t x = 0; x < lenght; x++) {
      Serial.print(prueba[x], HEX); Serial.print(' ');
    }
    Serial.println();
    flag_rx = false;
  }

  if (flag_timer) {
    timerStop(My_timer);
    axp.setChgLEDMode(AXP20X_LED_OFF);
    flag_timer = false;
  }

  Rx_Data();

}


void lcd() {
  Serial.print("Fecha: ");   Serial.print(Dia);  Serial.print('-');  Serial.print(Mes);       Serial.print('-');  Serial.print(Ano);
  Serial.print(' ');         Serial.print(Hor);  Serial.print(':');  Serial.print(Min);       Serial.print(' ');
  Serial.print("PM10: ");    Serial.print(PM10 / muestra, 1);   Serial.print(" PM25: ");      Serial.print(PM25 / muestra, 1);
  Serial.print(" T&H: ");    Serial.print((Temperatura - 32) * 0.56, 1); Serial.print(" C "); Serial.print(Humedad / 1.2585, 1);  Serial.print(" %");
  Serial.print(" Error: ");  Serial.println(Error);
}
