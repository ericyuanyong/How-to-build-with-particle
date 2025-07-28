
#include "app.h"


ACS37800 powerSensor; //Create an object of the ACS37800 class



unsigned long APP::_publish_interval        = 10000; //publish every 10 s in default
unsigned long APP::_read_sensor_interval    =100;     //update the sensor's value every 100ms in default
unsigned long APP::_power_flush_interval    = 1000;  //record the power consumption every 1 second in default

const float SECONDS_TO_KWH = 1.0 / 3600000.0;       //sample voltage and current every 1s, then the kWh is voltage*current*1s/(3600s*1000) = kWh


String APP::relayState = "OFF";


APP::APP(){

}

bool APP::hw_initialize(){

    Wire.begin(); //for ACS37800

    pinMode(LED,OUTPUT);
    pinMode(RELAY_LIVE,OUTPUT);
    pinMode(RELAY_NEUTRAL,OUTPUT);
    pinMode(ADC_CS,OUTPUT);
    pinMode(DCDC_MODE,OUTPUT);
    pinMode(BUTTON,INPUT);

    digitalWrite(DCDC_MODE,HIGH);
    digitalWrite(LED,LOW);
    digitalWrite(RELAY_LIVE,LOW);
    digitalWrite(RELAY_NEUTRAL,LOW);
    digitalWrite(ADC_CS,HIGH);

    if (powerSensor.begin() == false)
    {
        Serial.print(F("ACS37800 not detected.Just stop here as this device is malfunction now"));
        return false;
    }
    powerSensor.setBypassNenable(false, true); 
    powerSensor.setDividerRes(4000000);   
    return true;
}

//static function
int APP::relay_action(String state){ 
    if(state=="ON"){
        digitalWrite(RELAY_LIVE,1);
        digitalWrite(RELAY_NEUTRAL,1);
        delay(100);
        //once the relay is on, we can use pwm signal to drive the realy as that means power on coil get decreased and the heat generated coil will also decreased.
        analogWrite(RELAY_LIVE,150);
        analogWrite(RELAY_NEUTRAL,150);
        digitalWrite(LED,HIGH);
        relayState = "ON";
    }else{
        digitalWrite(RELAY_LIVE,LOW);
        digitalWrite(RELAY_NEUTRAL,LOW);
        digitalWrite(LED,LOW);
        relayState = "OFF";
    }
  return true;
}


//static function
int APP::set_publish_interval(String interval){
    _publish_interval =  strtol(interval.c_str(), NULL, 10);
    return 0;
}
uint16_t APP::get_publish_interval(){
    return _publish_interval;
}

int APP::set_sensor_read_interval(String interval){
    _read_sensor_interval = strtol(interval.c_str(), NULL, 10);
    return 0;
}
unsigned long  APP::get_sensor_read_interval(){
    return _read_sensor_interval;
}

int APP::set_power_flush_interval(String interval){
    _power_flush_interval = strtol(interval.c_str(), NULL, 10);
    return 0;
}
unsigned long APP::get_power_flush_interval(){
    return _power_flush_interval;
}






bool APP::read_sensor(void){
    return powerSensor.readRMS(&voltage, &current); //Read the RMS voltage and current
}

//the power is based on voltage*current*second.in the program, it will read the power every 1 second
//need to convert to the standard watt later like kwh?
void APP::accumulate_power(){
    static unsigned long interval = millis();
    //record the power everytime when sensor's value get upated.
    if(millis()-interval>=_power_flush_interval){
        interval = millis();
        if(read_sensor()==0)
            watt+=voltage*current/3600.0;   //voltage*current= W,  and then /3600 =Wh
    } 
}


float APP::get_voltage(void){
    return voltage;
}

float APP::get_current(void){
    return current;
}

bool APP::clear_accumulated_power(void){
    watt = 0.0;
    return true;
}

float APP::get_accumulated_power(void){
    return watt;  //return Wh
}

bool APP::set_voltage_alarm(float voltage){
    return true;
}

bool APP::set_current_alarm(float current){
    return true;
}

String APP::get_current_relay_state(void){
    return relayState;
}



//run this every 20 ms
uint8_t APP::button_isr(void){
  static uint8_t stage = 0;
  static unsigned int long_press_count = 0;
  bool btn_state = digitalRead(BUTTON);
  switch(stage){
    case 0: if(!btn_state){
                stage = 1;
                long_press_count = 0;
            }
            break;

    case 1: if(!btn_state){
                stage = 2;
            }else{
                stage = 0;  //not a valid press. go back to stage 0
            }
            break;

    case 2: if(!btn_state){
                long_press_count+=1;
                stage = 3;
            }else{
                stage = 0;
                return BTN_SINGLE_CLICK;   //a valid short press. return 1
            }
            break;
    
    case 3: if(!btn_state){
                long_press_count+=1;
                if(long_press_count>=LONG_PRESS_TIMEOUT){
                    stage = 4;
                    return BTN_LONG_CLICK;   //a valid long press, return 2
                }
            }else{
                stage = 0;
                return BTN_SINGLE_CLICK;   //a valid short press.
            }
            break;
    case 4:
        if(!btn_state){
            //do nothing here
        }else{
            stage = 0;
        }
        break;

    default:break;
  }
  return 0; //no action.
}
