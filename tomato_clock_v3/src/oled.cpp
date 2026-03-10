

#include "oled.h"
#include "pinMap.h"



Adafruit_SSD1306 ssd1306 = Adafruit_SSD1306(128, 32, &Wire);


void OLEDClass:: clear_display(){
    ssd1306.clearDisplay();
}

void OLEDClass::display_darw_rectangle(){
  ssd1306.drawRect(0,0,128,32,SSD1306_WHITE);
  ssd1306.display(); // actually display all of the above

}

void OLEDClass::display_info(uint8_t type,uint8_t value){
  String title = "";
  if(type==AIR_PM1){
    title = "PM1.0:";
  }else if(type == AIR_PM25){
    title = "PM2.5:";
  }else if(type == AIR_PM10){
    title = "PM10:";
  }else if(type == TEMPERATURE){
    title = "TEMP:";
  }else if(type == HUMIDITY){
    title = "HUMI:";
  }

  ssd1306.setTextSize(2);
  ssd1306.setTextColor(SSD1306_WHITE);
  ssd1306.setCursor(1,10);
  ssd1306.print(title+String(value));
  uint8_t current_cursor_x = ssd1306.getCursorX();
  ssd1306.setTextSize(1);
  ssd1306.setTextColor(SSD1306_WHITE);
  ssd1306.setCursor(current_cursor_x,15);
  if(type == AIR_PM1 || type == AIR_PM25 || type == AIR_PM10){
    ssd1306.print("ug/m3");
  }else if(type == TEMPERATURE){
    ssd1306.print(" c");
  }else if(type == HUMIDITY){
    ssd1306.print(" %");
  }
  ssd1306.display(); // actually display all of the above
 
}

//dispaly progress bar to act like a tomato clock
//there are totally 128*32=4096's dot. 
//And for the tomato clock, the time will be divided by 4096. 
//and the minimal time is 10minutes and the maximum is 1 hours,which is 60minutes
void OLEDClass::display_tomato_clock(uint8_t time_in_minutes){ 
  uint8_t correct_time_in_minutes = time_in_minutes;
  if(time_in_minutes<10){
    correct_time_in_minutes = 10;
  }
  if(time_in_minutes>60){
    correct_time_in_minutes = 60;
  }
  ssd1306.clearDisplay();
  display_darw_rectangle();
  uint16_t time_per_dot = (correct_time_in_minutes*1000)>>12;

  //for now I just hardcode it to around 30 minutes. 
  //and once in this clock. all the other action will be no responsable.
  for(char column=1;column<128;column++){   //128
    for(char row =1;row<32;row++){
      ssd1306.drawPixel(column, row, SSD1306_WHITE);
      ssd1306.display();
      delay(401);   //401
    }
  }
}

