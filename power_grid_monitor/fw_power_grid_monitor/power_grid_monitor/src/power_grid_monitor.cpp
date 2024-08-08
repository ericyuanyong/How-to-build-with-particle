/* 
 * Project myProject
 * Author: Your Name
 * Date: 
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include "./lib/SparkFun_ACS37800_Arduino_Library.h"
#include "pinMap.h"
#include <Wire.h>
#include <SPI.h>
#include "app.h"


/*
Some basic design requirement:
1. MSoM will read out the current and voltage of the sensor every 100ms.
2. MSoM will report the current and voltage and the total power within a flash rate setted by UI.
3. alarm voltage and alarm current can be setted by UI.
4. once alarm voltage and current get trigged, then the relay will turn off.
5. relay can be controlled on and off by UI.
6. total power consumption will be stored in EEPROM every 1 hour. And can be clearned to zero bu UI.
7. single click of the button will triggle a publish.
8. double click of the button will toggle on and off the relay.
9. when relay is on. The led on button will on. And it will off when relay is off.

#note:
1. those relay's coil consume around 2w when working. Which turns out to be very hot in a sealed enclosure. 
    So I have to use PWM mode when the relay is triggered. Which may decrease some power on relay.
*/



SYSTEM_MODE(AUTOMATIC);
APP app_power_monitor;

SerialLogHandler logHandler(LOG_LEVEL_INFO);



//particle variables
unsigned long publish_interval_ms       = 5000;
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

  //register particle variables
  Particle.variable("self_check",sensor_health);
  Particle.variable("relayState",relay_state);
  Particle.variable("publish_interval_ms",publish_interval_ms);
  Particle.variable("sensor_read_interval_ms",sensor_read_interval_ms);
  Particle.variable("power_record_interval_ms",power_flush_interval_ms);

  //register particle functions.function must return a int value and take one String parameter
  Particle.function("relay_action",app_power_monitor.relay_action); //valid parameter for this function is ON or OFF
  Particle.function("set_publish_interval_second",app_power_monitor.set_publish_interval); 
  Particle.function("set_sensor_update_interval_ms",app_power_monitor.set_sensor_read_interval);
  Particle.function("set_power_record_interval_ms",app_power_monitor.set_power_flush_interval);
  Particle.function("Clear power record",app_power_monitor.clear_power_record);

  Watchdog.init(WatchdogConfiguration().timeout(30s));
  Watchdog.start();
}


void loop(){  
  static unsigned long publish_timeout = 0;
  static unsigned long read_sensor_timeout = 0;
  static unsigned long power_record_timeout = 0;
  static unsigned long button_timeout = 0;

  publish_interval_ms     = app_power_monitor.get_publish_interval();
  sensor_read_interval_ms = app_power_monitor.get_sensor_read_interval();
  power_flush_interval_ms = app_power_monitor.get_power_flush_interval();

  //update sensor read frequency
  if(millis()-read_sensor_timeout>=sensor_read_interval_ms){
    read_sensor_timeout = millis();
    app_power_monitor.read_sensor();
    //udpate the watchdog in readsensor update function.
    Watchdog.refresh(); 
    relay_state = app_power_monitor.get_current_relay_state();
  }

  //update power record frequency
  if(millis()-power_record_timeout>=power_flush_interval_ms){
    power_record_timeout = millis();
    app_power_monitor.update_accumulated_power();
  }
  
  //publish it.
  if(millis()-publish_timeout>=publish_interval_ms){
    publish_timeout = millis();
    float volts = app_power_monitor.get_voltage();
    float amps = app_power_monitor.get_current();
    float power = app_power_monitor.get_accumulated_power();
    //Particle.publish("power", String::format("%.2f,%.2f,%d",volts,amps,power));
    char buf[100];
    memset(buf, 0, sizeof(buf));
    JSONBufferWriter writer(buf, sizeof(buf));
    writer.beginObject();
        writer.name("volts").value(volts);
        writer.name("amps").value(amps);
        writer.name("power").value(power);
    writer.endObject();
    bool success = false;
    success = Particle.publish("device/data", buf);
    //success = Particle.publish("device/data", relay_state);
  }

  if(millis()-button_timeout>=BUTTON_CHECK_INTERVAL){
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