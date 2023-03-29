
// ------------------  LIBRARIES FOR LORA    --------------------

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

//-------------------  VARIABLES GLOBALES    --------------------

uint8_t   NWKSKEY[16], APPSKEY[16], KEY[16], tx_payload[7];
uint32_t  DEVADDR;

bool      flag_timer;
uint16_t  Ext2, Ext1, Consumo, Min;

// ------------------- LMIC - PIN USED ---------------------------

void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }
static osjob_t sendjob;

const lmic_pinmap lmic_pins = {
  .nss = 18,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = 23,
  .dio = {26, 33, 32},
};

const uint8_t Led = 25;
const uint8_t S1  = 14;  /// Cambiar
const uint8_t S2  = 13;  /// Cambiar

// ------------------ INTERRUPCION TIMER OVF -------------------

hw_timer_t *My_timer = NULL;

void IRAM_ATTR onTimer() {
  flag_timer = true;
}

// ------------------ INTERRPCION EXTERNA   --------------------

void IRAM_ATTR Sensor1() {
  Ext2++;
}

void IRAM_ATTR Sensor2() {
  Ext1++;
}

void setup() {

  Serial.begin(115200);
  pinMode(Led,  OUTPUT);  digitalWrite(Led, LOW);

  str_key("17FD4903C02AAE4D2A8B5D8B6E9655B3");                      // LEER PARAMETROS LORAWAN
  memcpy_P(NWKSKEY, KEY, 16);

  str_key("7D464AD6BD678CE39B3CF360FD5C5B6C");
  memcpy_P(APPSKEY, KEY, 16);

  str_DEVADDR("3025B84E");

  Init_Lora();

  pinMode(S1, INPUT_PULLUP);
  pinMode(S2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(S1), Sensor1, FALLING);             // FALLING RISING CHANGE  HIGH  LOW
  attachInterrupt(digitalPinToInterrupt(S2), Sensor2, FALLING);             // FALLING RISING CHANGE  HIGH  LOW

  My_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(My_timer, &onTimer, true);
  timerAlarmWrite(My_timer, 60000000, true);
  timerAlarmEnable(My_timer);

}

void loop() {
  os_runloop_once();

  if (flag_timer) {
    Consumo += abs(Ext2 - Ext1);
    Min++;
    
    // almacenar en sd
    
    tx_payload[0] = (Consumo >> 8) & 0x00FF;
    tx_payload[1] =  Consumo & 0x00FF;

    tx_payload[2] = (Ext1 >> 8) & 0x00FF;
    tx_payload[3] =  Ext1 & 0x00FF;

    tx_payload[4] = (Ext2 >> 8) & 0x00FF;
    tx_payload[5] =  Ext2 & 0x00FF;

    tx_payload[6] = Min;
   
    do_send(&sendjob);
    flag_timer = false; Ext2  = 0;  Ext1 = 0;
  }

}
