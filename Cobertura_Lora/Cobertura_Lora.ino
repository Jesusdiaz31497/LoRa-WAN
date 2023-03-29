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

bool      flag_ext, flag_timer, flag_ack, flag_timer_send, flag_sleep;
uint8_t   tx_payload[2];

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
hw_timer_t *first_send = NULL;
hw_timer_t *Timer_sleep = NULL;

void IRAM_ATTR onTimer() {
  flag_timer = true;
}

void IRAM_ATTR Disparo() {
  flag_timer_send = true;
}

void IRAM_ATTR Sleep() {
  flag_sleep = true;
}


#include "driver/rtc_io.h"
#define Led  4

void setup() {
  AX192_init();
  Init_Lora();

  Serial.begin(115200);
  pinMode(Led, OUTPUT);  digitalWrite(Led, HIGH);

  pinMode(38, INPUT);
  attachInterrupt(digitalPinToInterrupt(38), tx_prueba, RISING);        // FALLING RISING CHANGE  HIGH  LOW

  // ---------------------------------- TIMER OK ACK -- > SLEEP -----------------------------------//

  My_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(My_timer, &onTimer, true);
  timerAlarmWrite(My_timer, 4000000, true);                            // (80M*SEG)/80
  timerAlarmEnable(My_timer);

  // ---------------------------------- TIMER DELAY SEND --------------------------------------//

  first_send = timerBegin(1, 80, true);
  timerAttachInterrupt(first_send, &Disparo, true);
  timerAlarmWrite(first_send, 9000000, true);                            // (80M*SEG)/80
  timerAlarmEnable(first_send);
  timerStop(first_send);

  // ---------------------------------- TIMER NOT ACK ---> SLEEP MODE ------------------------//

  Timer_sleep = timerBegin(2, 80, true);
  timerAttachInterrupt(Timer_sleep, &Sleep, true);
  timerAlarmWrite(Timer_sleep, 20000000, true);                            // (80M*SEG)/80
  timerAlarmEnable(Timer_sleep);

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_38, 0);
}

void loop() {
  os_runloop_once();

  if (flag_ext) {
    timerStop(My_timer);
    axp.setChgLEDMode(AXP20X_LED_OFF);
    timerStart(first_send);
    LMIC_sendAlive ();                          // do_send_first(&sendjob);  //
    Led_Red();
    flag_ext = false;
  }

  if (flag_timer_send) {
    timerStop(first_send);
    do_send(&sendjob);
    flag_timer_send = false;
  }

  if (flag_ack) {
    Serial.println("--LoRaWAN ACK OK--");
    axp.setChgLEDMode(AXP20X_LED_LOW_LEVEL);   //axp.setChgLEDMode(AXP20X_LED_BLINK_1HZ);
    timerRestart(My_timer);
    timerStart(My_timer);
    LMIC_clrTxData ();
    flag_ack = false; flag_timer = false;
  }

  if (flag_timer) {
    timerStop(My_timer);
    axp.setChgLEDMode(AXP20X_LED_OFF);
    LMIC_shutdown();
    AX192_off();
    Serial.println("ESP32 SLEEP MODE OK ACK");
    Serial.flush();
    esp_deep_sleep_start();
    flag_timer = false;
  }

  if (flag_sleep) {
    LMIC_shutdown();
    AX192_off();
    Serial.println("ESP32 SLEEP MODE NO ACK");
    Serial.flush();
    esp_deep_sleep_start();
    flag_sleep = false;
  }
}

void Led_Red() {
  for (uint8_t x = 0; x < 5; x++) {
    digitalWrite(Led, LOW);
    delay(25);
    digitalWrite(Led, HIGH);
    delay(25);
  }
}




/*
  if (flag_rx) {
    for (uint8_t x = 0; x < lenght; x++) {
      Serial.print(prueba[x], HEX); Serial.print(' ');
    }
    flag_rx = false;
  }

   if (LMIC.dataLen) {
    lenght = LMIC.dataLen;
    for (int i = 0; i < LMIC.dataLen; i++) {
      prueba[i] = LMIC.frame[LMIC.dataBeg + i];
    }
    flag_rx = true;
   }

     //rtc_gpio_pullup_en(GPIO_NUM_38);
  //rtc_gpio_pulldown_dis(GPIO_NUM_38);

   // esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
   //#define uS_TO_S_FACTOR 1000000ULL
  //#define TIME_TO_SLEEP  60

   //os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(10), do_send);
*/
