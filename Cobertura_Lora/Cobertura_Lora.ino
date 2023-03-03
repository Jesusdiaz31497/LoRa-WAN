// --------------- LIBRARIES FOR CONTROL POWER ------------------- //

#include "axp20x.h"
AXP20X_Class axp;

// ------------------  LIBRARIES FOR LORA    -------------------- //

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

//-------------------  VARIABLES GLOBALES    -------------------- //

uint8_t   NWKSKEY[16], APPSKEY[16], KEY[16];
uint32_t  DEVADDR;
uint8_t   tx_payload[4];

bool      flag_ext, flag_timer, flag_ack, flag_rx;
uint8_t   prueba[4];
uint8_t   lenght;

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

  Serial.begin(115200);
  pinMode(Led, OUTPUT);  digitalWrite(Led, HIGH);

  AX192_init();
  Init_Lora();

  pinMode(38, INPUT);
  attachInterrupt(digitalPinToInterrupt(38), tx_prueba, RISING);        // FALLING RISING CHANGE  HIGH  LOW

  My_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(My_timer, &onTimer, true);
  timerAlarmWrite(My_timer, 30000000, true);                            // (80M*SEG)/80
  timerAlarmEnable(My_timer);
  timerStop(My_timer);

}

void loop() {
  os_runloop_once();

  if (flag_ext) {
    timerRestart(My_timer);
    timerStart(My_timer);
    axp.setChgLEDMode(AXP20X_LED_OFF);
    do_send(&sendjob);
    flag_ext   = false;
    flag_timer = false;
  }

  if (flag_ack) {
    axp.setChgLEDMode(AXP20X_LED_BLINK_1HZ);
    Serial.println("--LoRaWAN ACK OK--");
    flag_ack = false;
  }

  if (flag_rx) {
    for (uint8_t x = 0; x < lenght; x++) {
      Serial.print(prueba[x], HEX); Serial.print(' ');
    }
    flag_rx = false;
  }


  if (flag_timer) {
    timerStop(My_timer);
    axp.setChgLEDMode(AXP20X_LED_OFF);
    flag_timer = false;
  }

}
