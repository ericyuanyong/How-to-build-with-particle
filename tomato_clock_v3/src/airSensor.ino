#include <Wire.h>
#include "./lib/Adafruit_GFX.h"
#include "./lib/Adafruit_SSD1306.h"
#include "./lib/Adafruit_SHT31.h"
#include "./lib/SHT31.h"
#include "./lib/BMI160Gen.h"
#include "./lib/BMI160.h"
#include "./lib/CurieIMU.h"

#include "pinMap.h"
#include "button.h"
#include "oled.h"
#include "pms7003.h"
#include "misc.h"

#define SHT31_ADDRESS   0x44

#define BTN_SCAN_INTERVAL 40

SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(DISABLED); //make sure app wil not start to work until the particle connecetion is done

Adafruit_SHT31 sht31;
//Adafruit_SSD1306 ssd1306 = Adafruit_SSD1306(128,32,&Wire);
BMI160Class bmi160_drv;
Button left_button(BTN_LEFT);
Button right_button(BTN_RIGHT);
Button mid_button(BTN_MID);
PMS7003 pms7003;
MiscClass misc;
OLEDClass oled;




SystemSleepConfiguration config;
const int bmi160_i2c_addr = 0x68;
const int bmi160_interrupt_pin = IMU_INT;   //D2 is the interrupt pin


void setup(){
  delay(1000);
  RGB.control(true);
  if(!Particle.connected()){
    RGB.color(100,0,0);
  }
  else{
    RGB.color(0,0,0);
  }

  Serial.begin(9600);
 // Serial1.begin(9600);
  misc.initiaize();
  misc.dcdc_mode(DCDC_FIX_PWM);
  misc.ext_power_on(true);

  if(!ssd1306.begin(SSD1306_SWITCHCAPVCC, 0x3c)){ 
    Serial.print("Couldn't find ssd1306");
  }
  //seems that dim mode is pretty bright to use.
  ssd1306.dim(true);
  ssd1306.clearDisplay();  
  ssd1306.display(); 

}


/*
only have tomato clock function now
only have three features:
1. press button to enter the tomato clock, it will at first publish a message to particle cloud. It will simply block all the other features as 30 minutes is a dead loop.
2. after clock finished. it will simply turn off the screen. 
3. it will periodically check the battery charge status and if there is a charge action. then the charge led will turns on.
4. it will periodically check if the WiFi connection is stable or not. if not the charge led will turns on.
*/
void loop(){

  static unsigned long button_scan_interval = 0;
  static unsigned long charge_scan_interval = 0;
  static unsigned long network_scan_interval = 0;

  if(millis()-button_scan_interval>=BTN_SCAN_INTERVAL){
    button_scan_interval = millis();
    uint8_t mid_btn = mid_button.pressAction();
    if(mid_btn==1){
      Serial.println("middle button short pressed");
      //around 30 minutes
      misc.ext_power_on(true);
      delay(200);
      if(Particle.connected()){
        RGB.color(0,0,0);
        Particle.publish("tomato/pressed", "start", PRIVATE); 
      }else{
        Particle.connect();
        RGB.color(100,0,0);
      }

      //this need to be initialized everytime when the clock started
      if(!ssd1306.begin(SSD1306_SWITCHCAPVCC, 0x3c)){ 
        Serial.print("Couldn't find ssd1306");
      }
      //seems that dim mode is pretty bright to use.
      ssd1306.dim(true);
      ssd1306.clearDisplay();  
      ssd1306.display(); 

      oled.display_tomato_clock(1);
      tone(BUZZER,400,100);
      ssd1306.clearDisplay();
      ssd1306.display();
      delay(1000);
      misc.ext_power_on(false);
    }
  }
  
  if(millis()-network_scan_interval>= 60000){
    network_scan_interval = millis();
    if(!Particle.connected()){
      RGB.color(100,0,0);
    }else{
      RGB.color(0,0,0);
    }
  }
}
