//01-03-23 V2.0.1

//Every min gets temperature and humidity. After 15min or when button is pressed
//send the avarage of the colected data. 

//DHT libraries
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

//LoRa libraries
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

//OLED libraries
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 oled(128, 32, &Wire, -1);

//Definitions and variables
#define DHTPIN     12
#define DHTPIN1    13
#define DHTPIN2    15
#define DHTTYPE   DHT22

uint8_t     NWKSKEY[16], APPSKEY[16], KEY[16], PAYLOAD[6], count_temp[3], count_hum[3];    
uint32_t    DEVADDR;
bool      timer_event = false, timer_event1 = false, ext_event = false;
float     temp[3], hum[3], crnt_temp[3], crnt_hum[3], prom_temp, prom_hum;     //Temperature, humidity. One element per DHT
const uint8_t led = 25;

static const unsigned char PROGMEM image_data_Cenicana[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xf0, 0x0f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xc0, 0x03, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x80, 0x01, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x00, 0x3f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x00, 0x1f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xf8, 0x00, 0x00, 0x1f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xf0, 0x00, 0x00, 0x1f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x01, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x03, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xf0, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xf8, 0x00, 0x00, 0x0f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xf8, 0x00, 0x00, 0x1f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xfc, 0x00, 0x00, 0x3f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xfc, 0x00, 0x00, 0x7f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xfe, 0x00, 0x00, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x03, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x07, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x80, 0x07, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xe0, 0x1f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xf0, 0x3f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
//Create one objetct per DHT
DHT dht[]{
          {DHTPIN, DHTTYPE},
          {DHTPIN1, DHTTYPE},
          {DHTPIN2, DHTTYPE},
         };

//LMIC pins
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

//Interrupts
hw_timer_t *timer0 = NULL;          //Counts 1 min
hw_timer_t *timer1 = NULL;          //Counts 15 min

void IRAM_ATTR tx_prueba() {
  ext_event = true;
}

void IRAM_ATTR onTimer() {
  timer_event = true;
}

void IRAM_ATTR onTimer1() {
  timer_event1 = true;
}

void setup() {
  Serial.begin(9600);        //PC port

  //OLED initialization
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);                                                 // Don't proceed, loop forever
  }

  pinMode(led, OUTPUT);  digitalWrite(led, LOW);              //Turn led off and initialize LoRa
  Init_Lora();
  
  // Initialize DHTs
  for(int i=0;i<=2;i++){
    dht[i].begin(); 
  }

  //Set timmers 0&1, prescaler:80, autoreload, interrupt enable, count up.
  timer0 = timerBegin(0, 80, true);
  timerAttachInterrupt(timer0, &onTimer, true);
  timerAlarmWrite(timer0, 60000000, true);                            //(80M*SEG)/80
  timerAlarmEnable(timer0);

  timer1 = timerBegin(1, 80, true);
  timerAttachInterrupt(timer1, &onTimer1, true);
  timerAlarmWrite(timer1, 900000000, true);                           //(80M*SEG)/80
  timerAlarmEnable(timer1);

  pinMode(38, INPUT);
  attachInterrupt(digitalPinToInterrupt(38), tx_prueba, RISING);
}

void loop() {
  os_runloop_once(); //Make one job in the queue per loop execution
    
  //Get data;
  if(timer_event){
    digitalWrite(led,HIGH);
    
    read_dht();
    display_data();
    
    timer_event = false;
    digitalWrite(led,LOW);
  }

  //Send data
  if(timer_event1){
    digitalWrite(led,HIGH);

    for(int i=0;i<=2;i++){
      temp[i]=temp[i]/count_temp[i];
      hum[i]=hum[i]/count_hum[i];
    }
    
    display_media();
    
    for(int i=0;i<=2;i++){
      temp[i] = temp[i]*5;
      hum[i] = hum[i]*2.5;
    }
    
    do_send(&sendjob);
    
    temp[0] = 0;  hum[0] = 0; temp[1] = 0;  hum[1] = 0; temp[2] = 0;  hum[2] = 0; count_temp[0] = 0; count_hum[0] = 0; count_temp[1] = 0; count_hum[1] = 0; count_temp[2] = 0; count_hum[2] = 0;

    timer_event1 = false;
    digitalWrite(led,LOW);
  }

  if(ext_event){
    digitalWrite(led,HIGH);
    
    read_dht();
    display_data();
   
    for(int i=0;i<=2;i++){
      temp[i]=temp[i]/count_temp[i];
      hum[i]=hum[i]/count_hum[i];
      
      temp[i] = temp[i]*5;
      hum[i] = hum[i]*2.5;
    }  

    do_send(&sendjob);

    for(int i=0;i<=2;i++){
      temp[i] = temp[i]/5;
      hum[i] = hum[i]/2.5;
    }

    count_temp[0] = 1; count_hum[0] = 1; count_temp[1] = 1; count_hum[1] = 1; count_temp[2] = 1; count_hum[2] = 1;
    ext_event = false;
    digitalWrite(led,LOW);
  }
}

void read_dht(){
  for(int i=0;i<=2;i++){
        // Get temperature event
        crnt_temp[i] = dht[i].readTemperature();
        
        temp[i] = temp[i] + crnt_temp[i];

        if (!isnan(crnt_temp[i])) {
          count_temp[i]++;
        }
    
        // Get humidity event
        crnt_hum[i] = dht[i].readHumidity();
        
        hum[i] = hum[i] + crnt_hum[i];

        if (!isnan(crnt_hum[i])) {
          count_hum[i]++;
        }
   }
}

void display_data(){   
  oled.clearDisplay();            // clear display
  oled.setTextSize(1);            // set text size
  oled.setTextColor(WHITE);       // set text color 
        
  for(int i=0;i<=2;i++){
        oled.clearDisplay();             // clear display
        
        oled.setCursor(40, 0);           // set position to display   x,y 
        oled.println("Cenicana");        // set text
        
        Serial.print(F("Sensor "));Serial.print(i+1);
        oled.setCursor(0, 8);
        oled.println("Sensor");
        oled.setCursor(40, 8);
        oled.println(i+1);
        
        if (isnan(crnt_temp[i])) {
          Serial.print(F(" - Error T°"));
          oled.setCursor(0, 16);
          oled.println("Error T");
        }
        else{
          Serial.print(F(" - T°C: ")); Serial.print(crnt_temp[i]);
          oled.setCursor(0, 16);
          oled.println("T.C:");
          oled.setCursor(40, 16);
          oled.println(crnt_temp[i]);
        }

        if (isnan(crnt_hum[i])) {
          Serial.println(F(" - Error HR"));
          oled.setCursor(0, 24);
          oled.println("Error HR");
        }
        else{
          Serial.print(F(" HR: ")); Serial.println(crnt_hum[i]);
          oled.setCursor(0, 24);
          oled.println("HR:");
          oled.setCursor(40, 24);
          oled.println(crnt_hum[i]);
        }

        oled.display();       // display on OLED
        delay(2500);
  }

  oled.clearDisplay();            
  oled.setTextSize(1);            
  oled.setTextColor(WHITE);       

  oled.drawBitmap(0, 0, image_data_Cenicana, 128, 32, SSD1306_WHITE);
  oled.display();
}

void display_media(){   
  oled.clearDisplay();            
  oled.setTextSize(1);            
  oled.setTextColor(WHITE);        
        
  for(int i=0;i<=2;i++){
        oled.clearDisplay();             
        
        oled.setCursor(10, 0);            
        oled.println("Prom 15 min");     
        
        Serial.print(F("Sensor "));Serial.print(i+1);
        oled.setCursor(0, 8);
        oled.println("Sensor");
        oled.setCursor(40, 8);
        oled.println(i+1);
        
        if (isnan(temp[i])) {
          Serial.print(F(" - Error T°"));
          oled.setCursor(0, 16);
          oled.println("Error T");
        }
        else{
          Serial.print(F(" - T°C Prom: ")); Serial.print(temp[i]);
          oled.setCursor(0, 16);
          oled.println("T.C Prom:");
          oled.setCursor(60, 16);
          oled.println(temp[i]);
        }

        if (isnan(hum[i])) {
          Serial.println(F(" - Error HR"));
          oled.setCursor(0, 24);
          oled.println("Error HR");
        }
        else{
          Serial.print(F(" HR Prom: ")); Serial.println(hum[i]);
          oled.setCursor(0, 24);
          oled.println("HR Prom:");
          oled.setCursor(60, 24);
          oled.println(hum[i]);
        }

        oled.display();       
        delay(2500);
  }

  oled.clearDisplay();            
  oled.setTextSize(1);            
  oled.setTextColor(WHITE);       

  prom_temp = (temp[1]+temp[2]+temp[3])/3;
  prom_hum  = (hum[1]+hum[2]+hum[3])/3;

  oled.setCursor(10, 0);           
  oled.println("Prom 15 min");        
  
  Serial.print(F("General"));
  oled.setCursor(0, 8);
  oled.println("General");
          
  Serial.print(F(" - T°C Prom: ")); Serial.print(prom_temp);
  oled.setCursor(0, 16);
  oled.println("T.C Prom:");
  oled.setCursor(50, 16);
  oled.println(prom_temp);

  Serial.print(F(" HR Prom: ")); Serial.println(prom_hum);
  oled.setCursor(0, 24);
  oled.println("HR Prom:");
  oled.setCursor(50, 24);
  oled.println(prom_hum);
}
