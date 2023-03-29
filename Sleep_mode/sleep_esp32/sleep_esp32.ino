#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  60        /* Time ESP32 will go to sleep (in seconds) */

// ------------------  LIBRARIES FOR LORA    -------------------- //

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

//-------------------  VARIABLES GLOBALES    -------------------- //

uint8_t     NWKSKEY[16], APPSKEY[16], KEY[16], cnt;    
uint32_t    DEVADDR;
float       temp;

bool      flag_ack = false, timer_event = false, flag_tx = false;

hw_timer_t *timer0 = NULL; 
hw_timer_t *timer1 = NULL;

uint8_t PAYLOAD[] = {'I','N','G'}; 

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

void IRAM_ATTR onTimer() {
  timer_event = true;
}

void IRAM_ATTR onTimer1() {
  timer_event = true;
}

void setup(){
  Serial.begin(115200);

  timer0 = timerBegin(0, 80, true);
  timerAttachInterrupt(timer0, &onTimer, true);
  timerAlarmWrite(timer0, 20000000, true);                            //(80M*SEG)/80
  timerAlarmEnable(timer0);

  timer1 = timerBegin(1, 80, true);
  timerAttachInterrupt(timer1, &onTimer1, true);
  timerAlarmWrite(timer1, 60000000, true);                            //(80M*SEG)/80
  timerAlarmEnable(timer1);

  timerStop(timer1);
  
  delay(1000); //Take some time to open up the Serial Monitor

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  /*
  Next we decide what all peripherals to shut down/keep on
  By default, ESP32 will automatically power down the peripherals
  not needed by the wakeup source, but if you want to be a poweruser
  this is for you. Read in detail at the API docs
  http://esp-idf.readthedocs.io/en/latest/api-reference/system/deep_sleep.html
  Left the line commented as an example of how to configure peripherals.
  The line below turns off all RTC peripherals in deep sleep.
  */
  //esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  //Serial.println("Configured all RTC Peripherals to be powered down in sleep");

  /*
  Now that we have setup a wake cause and if needed setup the
  peripherals state in deep sleep, we can now start going to
  deep sleep.
  In the case that no wake up sources were provided but deep
  sleep was started, it will sleep forever unless hardware
  reset occurs.
  */

  Init_Lora();
  do_send(&sendjob);
  
}

void loop(){
  os_runloop_once();
  
//  if(flag_ack){
//    timerRestart(timer0);
//    flag_ack = false;
//  }
  
  if(flag_tx){
    Serial.println("Aqui");
    cnt = cnt+1;
    flag_tx = false;
    Serial.println(cnt);
  }

  if(flag_ack){
      cnt = 0;
      timerRestart(timer0);
      flag_ack = false;
  }
  else if(cnt = 8){
      cnt = 0;
      timerStop(timer0);
      
      os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(30), do_send);
      timerRestart(timer1);
      timerStart(timer1);
  }

  if(timer_event){
    Serial.println("Going to sleep now");
    Serial.flush(); 
    timer_event=false;
    esp_deep_sleep_start();
  }
}
