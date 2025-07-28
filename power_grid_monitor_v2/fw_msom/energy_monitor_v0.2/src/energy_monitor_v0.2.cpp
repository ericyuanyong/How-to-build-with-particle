

// Include Particle Device OS APIs
#include "Particle.h"
#include "./lib/SparkFun_ACS37800_Arduino_Library.h"
#include "pinMap.h"
#include <Wire.h>
#include <SPI.h>
#include "app.h"


SYSTEM_MODE(AUTOMATIC);
APP app_power_monitor;

SerialLogHandler logHandler(LOG_LEVEL_INFO);


//particle variables
double watt                             = 3.213;
unsigned long sensor_read_interval_ms   = 40;
unsigned long power_flush_interval_ms   = 50000;
String relay_state                      = "OFF";
String sensor_health                    = "";


void setup(){

  Serial.begin(115200);
  if(!app_power_monitor.hw_initialize()){
    RGB.control(true);
    RGB.color(255,0,0);
    sensor_health = "unhealth";
  }else{
    sensor_health = "health";
  }
  pinMode(D13,OUTPUT);

}


void loop(){

  static unsigned long publish_interval = millis();
  static unsigned long button_timeout = 0;
  static bool state = false;
  
  app_power_monitor.accumulate_power(); //update energy usage every second

  if (millis() - publish_interval >= 60000) {
    publish_interval = millis();
    char data[64];
    snprintf(data, sizeof(data), "{\"energy_wh\":%.2f}", app_power_monitor.get_accumulated_power());
    bool success = Particle.publish("energy_hourly", data, PRIVATE | WITH_ACK);
    if (success) {
      Serial.println("Data published successfully (with ledger)");
      app_power_monitor.clear_accumulated_power();  // Reset energy accumulator
    } else {
      Serial.println("Publish failed. Will try again later.");
    }
  }

  if(millis()-button_timeout>=BUTTON_CHECK_INTERVAL){
    digitalWrite(D13,state);
    state = !state;
    button_timeout = millis();
    if(app_power_monitor.button_isr()==BTN_SINGLE_CLICK){
      if(relay_state=="OFF"){
          relay_state = "ON";
      }else if(relay_state=="ON"){
          relay_state = "OFF";
      }
      app_power_monitor.relay_action(relay_state);
    }
  }
}


