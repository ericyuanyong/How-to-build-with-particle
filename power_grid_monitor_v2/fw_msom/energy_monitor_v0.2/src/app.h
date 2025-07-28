
#ifndef _APP_H_
#define _APP_H_

#include "Particle.h"
#include "./lib/SparkFun_ACS37800_Arduino_Library.h"
#include "pinMap.h"
#include <Wire.h>
#include <SPI.h>




/*
the total power consumption will be stored in eeprom. 
in order to reduce the write cycle, it's not suggested to set the interval too low
however, increase the interval will also decrease the accuracy. Need to figure out how to keep the balance.
The current solution is calcuate the power every SENSOR_UPDATE_INTERVAL. and then flush that into eeprom every POWER_UPDATE_INTERVAL.
*/

#define POWER_UPDATE_INTERVAL   60000   //unit is ms
#define SENSOR_UPDATE_INTERVAL  100     //unit is ms

#define BUTTON_CHECK_INTERVAL   20
#define LONG_PRESS_TIMEOUT      2000

#define BTN_SINGLE_CLICK    1
#define BTN_LONG_CLICK      2


class APP{

    public:

        APP();
        
        static unsigned long _publish_interval;
        static unsigned long _read_sensor_interval;
        static unsigned long _power_flush_interval;
        
        bool hw_initialize();
        static int relay_action(String state);

        static int set_publish_interval(String interval);
        uint16_t get_publish_interval();
        
        static int set_sensor_read_interval(String interval);
        unsigned long get_sensor_read_interval();

        static int set_power_flush_interval(String interval);
        unsigned long get_power_flush_interval();

        static int clear_power_record(String initial_data);
        
        bool read_sensor(void);
        
        void accumulate_power(void);
        bool clear_accumulated_power(void);
        float get_accumulated_power(void);


        float get_voltage(void);
        float get_current(void);
        String get_current_relay_state(void);

        
        bool set_voltage_alarm(float voltage);
        bool set_current_alarm(float current);

        uint8_t button_isr(void);

        static String relayState;

    private:
        float voltage   = 0;
        float current   = 0;
        float watt      = 0;
        
        


};



#endif





