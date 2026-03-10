
#include "misc.h"

void MiscClass::initiaize(void){
    pinMode(CHARGE,INPUT_PULLUP);
    pinMode(USB_DET,INPUT);
    pinMode(IMU_INT,INPUT);
    pinMode(BUZZER,OUTPUT);
    digitalWrite(BUZZER,LOW);
    pinMode(DCDC_MODE,OUTPUT);
    digitalWrite(DCDC_MODE,LOW);
}
void MiscClass::dcdc_mode(uint8_t mode){    
    if(mode==DCDC_PSM){
        digitalWrite(DCDC_MODE,LOW);
    }else{
        digitalWrite(DCDC_MODE,HIGH);
    }
}

void MiscClass::ext_power_on(bool stat){
    if(stat){
        digitalWrite(PERI_EN,HIGH);
    }else{
        digitalWrite(PERI_EN,LOW);
    }
}


uint8_t MiscClass::get_current_display_item(uint8_t direction){
  static uint8_t current_item = 0;
  if(direction == CW){
    current_item+=1;
    if(current_item>=5){
      current_item = 0;
    }
  }else if(direction == CCW){
    current_item-=1;
    if(current_item>5){   //since  currrent_time is unsigned char, so 0-1 ==255.
      current_item = 4;
    }
  }
  return current_item;
}


bool MiscClass::charge_service(void){
  if(digitalRead(USB_DET)){
    if(digitalRead(CHARGE)==0){
      return true;
    }
  }
  return false;
}