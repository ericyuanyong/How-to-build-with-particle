
#include "app.h"


ACS37800 powerSensor; //Create an object of the ACS37800 class



unsigned long APP::_publish_interval        = 10000; //publish every 10 s in default
unsigned long APP::_read_sensor_interval    =100;     //update the sensor's value every 100ms in default
unsigned long APP::_power_flush_interval    = 60000;  //record the power consumption every 1 minute in default

String APP::relayState = "OFF";

struct eeprom_object {
  float watt;
};

APP::APP(){

}

bool APP::hw_initialize(){

    Wire.begin(); //for ACS37800
    SPI.begin();  //for ADS

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
        delay(20);
        //analogWrite(RELAY_LIVE, 2500,30000);    //frequency is 10k and duty cycle is 60%
        //analogWrite(RELAY_NEUTRAL,2500,30000);
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
int APP::clear_power_record(String initial_data){   
    if(initial_data =="0"){
        eeprom_object eeprom_obj;
        eeprom_obj.watt = 0;
        EEPROM.put(0x00,eeprom_obj);
        return 0;
    }
    return -1;
}






bool APP::read_sensor(void){
    return powerSensor.readRMS(&voltage, &current); //Read the RMS voltage and current
}

//the power is based on voltage*current*second.
//need to convert to the standard watt later like kwh?
bool APP::update_accumulated_power(){
    static unsigned long sensor_udpate_interval = millis();
    static unsigned long power_update_interval = millis();
    static float period_power = 0;

    //record the power everytime when sensor's value get upated.
    if(millis()-sensor_udpate_interval>=_read_sensor_interval){
        sensor_udpate_interval = millis();
        period_power+=voltage*current*_read_sensor_interval*0.001;   //voltage*current*interval_in_second
        period_power = period_power/3600;
    }
    //store the power consumption 
    if(millis()-power_update_interval>=_power_flush_interval){
        power_update_interval = millis();
        float power_in_eeprom = read_accumulated_power_from_eeprom();
        eeprom_object eeprom_obj;
        eeprom_obj.watt = period_power+power_in_eeprom;
        EEPROM.put(0,eeprom_obj);  //store the updated value every 1 minute
    }
    return true;
}

//the unit is watt per second not kWh
float APP::read_accumulated_power_from_eeprom(){
    eeprom_object eeprom_obj;
    EEPROM.get(0x00,eeprom_obj);
    watt = eeprom_obj.watt;
    //watt = 1000.1;
    return watt;
}

float APP::get_voltage(void){
    return voltage;
}

float APP::get_current(void){
    return current;
}

float APP::get_accumulated_power(void){
    //watt = 100.23;
    return watt;
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
