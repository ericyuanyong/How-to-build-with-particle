

#ifndef _DISPLAY_H_
#define _DISPLAY_H_


#include <Wire.h>
#include "./lib/Adafruit_GFX.h"
#include "./lib/Adafruit_SSD1306.h"



#define AIR_PM1     0
#define AIR_PM25    1
#define AIR_PM10    2
#define TEMPERATURE 3
#define HUMIDITY    4

extern Adafruit_SSD1306 ssd1306;

class OLEDClass{
    public:
        void clear_display();
        void display_darw_rectangle();
        void display_tomato_clock(uint8_t time_in_minutes);
        void display_info(uint8_t type,uint8_t value);

};

#endif