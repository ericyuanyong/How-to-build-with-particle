
#include "pms7003.h"





struct pm_struct pm_air_struct;



PMS7003::PMS7003(void){
    pinMode(PMS7003_RST,OUTPUT);
    digitalWrite(PMS7003_RST,LOW);
    pinMode(PERI_EN,OUTPUT);
    digitalWrite(PERI_EN,LOW);
}


void PMS7003::power_on(bool stat){
    if(stat){
        digitalWrite(PERI_EN,HIGH);
    }else{
        digitalWrite(PERI_EN,LOW);
    }
}

void PMS7003::reset(void)
{
    digitalWrite(PMS7003_RST,LOW);
    delay(150);
    digitalWrite(PMS7003_RST,HIGH);
}


bool PMS7003::parse_sensor_data(uint8_t *input_buf,struct pm_struct *sensor_struct){

  if(input_buf[0] ==START_FRAME1 && input_buf[1] == START_FRAME2){
    uint16_t check_sum = 0;
    for(char i=0;i<30;i++){
      check_sum += input_buf[i];
    } 
    if(check_sum == ((input_buf[30]<<8)+input_buf[31])){
      sensor_struct->pm1 = (input_buf[OFFSET_PM1]<<8)+input_buf[OFFSET_PM1+1];
      sensor_struct->pm25 = (input_buf[OFFSET_PM25]<<8)+input_buf[OFFSET_PM25+1];
      sensor_struct->pm10 = (input_buf[OFFSET_PM10]<<8)+input_buf[OFFSET_PM10+1];
      
      return true;
    }else{
      return false;
    }
  }
  return false;
}


bool PMS7003::read_air_sensor(Stream &ser){

  static uint8_t rx_buf[40];    //the normal datastream will only be 32 bytes.
  static uint8_t ptr_buf = 0;
  static unsigned long rx_start_time = millis();
  static bool count_it = false;

  if(ser.available()){
    rx_buf[ptr_buf++] = ser.read();
    count_it = true;
    rx_start_time = millis();
    if(ptr_buf>=40){  
      ptr_buf = 39;
    }
  }
  if(count_it){
    if(millis()-rx_start_time>=200){    //a datastream will alway be less than 200ms
      ptr_buf = 0;                      //clear the ptr for the rx buffer. 
      count_it = false;
      rx_start_time = 0;  
      if(parse_sensor_data(rx_buf,&pm_air_struct)){
        return true;
      }else{
        return false;
      }
    }
  }
  return false;
}



uint16_t PMS7003::get_pm1(void){
    return pm_air_struct.pm1;
}

uint16_t PMS7003::get_pm10(void){
    return pm_air_struct.pm10;
}

uint16_t PMS7003::get_pm25(void){
    return pm_air_struct.pm25;
}




