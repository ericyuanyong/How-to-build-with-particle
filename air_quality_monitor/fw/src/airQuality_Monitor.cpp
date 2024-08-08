/* 
 * Project myProject
 * Author: Your Name
 * Date: 
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include "./lib/SHT31.h"


//definition for air sensor
#define START_FRAME1  0x42
#define START_FRAME2  0x4D
#define OFFSET_PM1    10
#define OFFSET_PM2_5  12
#define OFFSET_PM10   14

#define PWR_EN_5V   D8
#define PWR_OUT_CTL D23

#define USER_BUTTON D7


//SYSTEM_MODE(MANUAL);
//SYSTEM_THREAD(ENABLED);
SerialLogHandler logHandler(LOG_LEVEL_INFO);


struct air_sensor_struct{
  uint16_t pm1 = 0;
  uint16_t pm2_5 = 0;
  uint16_t pm10 = 0;  
};


struct air_sensor_struct sensor_struct;

bool parse_air_sensor_data(uint8_t *input_buf,struct air_sensor_struct *sensor_struct);
bool read_air_sensor(void);



bool read_air_sensor(void){

  static uint8_t rx_buf[40];    //the normal datastream will only be 32 bytes.
  static uint8_t ptr_buf = 0;
  static unsigned long rx_start_time = millis();
  static bool count_it = false;

  if(Serial1.available()){
    rx_buf[ptr_buf++] = Serial1.read();
    //Serial.printf("%x",rx_buf[ptr_buf-1]);
    count_it = true;
    rx_start_time = millis();
    if(ptr_buf>=40){  
      ptr_buf = 39;
    }
  }
  if(count_it){
    if(millis()-rx_start_time>=200){    //a datastream will always be less than 200ms
      ptr_buf = 0;                      //clear the ptr for the rx buffer. 
      count_it = false;
      rx_start_time = 0;  
      if(parse_air_sensor_data(rx_buf,&sensor_struct)){
        return true;
      }else{
        return false;
      }
    }
  }
  return false;
}


bool parse_air_sensor_data(uint8_t *input_buf,struct air_sensor_struct *sensor_struct){
  if(input_buf[0] ==START_FRAME1 && input_buf[1] == START_FRAME2){
    uint16_t check_sum = 0;
    /*
    for(char i=0;i<32;i++){
      Serial.printf("%x-",input_buf[i]);
    }
    Serial.printf("\r\n");
    */
    for(char i=0;i<30;i++){
      check_sum += input_buf[i];
    }
    
    if(check_sum == ((input_buf[30]<<8)+input_buf[31])){
      sensor_struct->pm1  = (input_buf[OFFSET_PM1]<<8) +input_buf[OFFSET_PM1+1];
      sensor_struct->pm2_5 = (input_buf[OFFSET_PM2_5]<<8)+input_buf[OFFSET_PM2_5+1];
      sensor_struct->pm10 = (input_buf[OFFSET_PM10]<<8)+input_buf[OFFSET_PM10+1];
      return true;
    }else{
      return false;
    }
  }
  return false;
}



//disable the 5v booster for air quality sensorin default.
void gpio_init(){
  pinMode(PWR_EN_5V,OUTPUT);
  digitalWrite(PWR_EN_5V,LOW);
  pinMode(PWR_OUT_CTL,OUTPUT);
  digitalWrite(PWR_OUT_CTL,LOW);
  pinMode(USER_BUTTON,INPUT);
}



SHT31 sht31;

void setup(){
  STARTUP(RGB.mirrorTo(RGBG,RGBB,RGBR,true,true));
  Serial.begin(9600);
  Serial1.begin(9600);
  gpio_init();
  delay(3000);
  sht31.begin();
  if(!sht31.isConnected()){
    Serial.print("sht31 sensor isn't attached");
  }else{
    Serial.print("****************sht31 sensor is attached********************\r\n");
  }
  digitalWrite(PWR_EN_5V,HIGH);  //enable 5V booster for air quality sensor
  digitalWrite(PWR_OUT_CTL,HIGH);




}



void loop(){

  static unsigned long publish_interval = millis();  //it will fire the first publish 5 seconds later in loop.
  static unsigned long sample_interval = millis(); //make sure it will execute the first time in loop.
  static bool button_pressed = false;

  //update the air quality message every 1 minutes
  if(millis()-sample_interval>=60000){
    if(read_air_sensor()){
      sht31.read();
      Serial.printf("pm10:%d,pm1:%d,pm2.5:%d",sensor_struct.pm10,sensor_struct.pm1,sensor_struct.pm2_5);
      Serial.printf("humidity:%3f,temperature:%3f\r\n",sht31.getHumidity(),sht31.getTemperature());
      sample_interval = millis();
    }
  }

  //publish the message out to cloud
  if((millis()-publish_interval>=1800000)||(button_pressed==true)){   //30m*60s*1000. update every 30minutes
    button_pressed = false;
    publish_interval = millis();
    char buf[100];
    memset(buf, 0, sizeof(buf));
    JSONBufferWriter writer(buf, sizeof(buf));
    writer.beginObject();
        writer.name("pm1").value(sensor_struct.pm1);
        writer.name("pm2_5").value(sensor_struct.pm2_5);
        writer.name("pm10").value(sensor_struct.pm10);
        writer.name("temperature").value(sht31.getTemperature());
        writer.name("humidity").value(sht31.getHumidity());
    writer.endObject();
    bool success = false;
    success = Particle.publish("device/data", buf);
  }
  
  //fire one publish everytime when user button pressed.
  if(digitalRead(USER_BUTTON)==0){
    delay(20);
    while(digitalRead(USER_BUTTON)==0){}
    button_pressed = true;
  }
}
