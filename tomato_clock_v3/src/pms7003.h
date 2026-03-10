

#ifndef _PMS7003_H_
#define _PMS7003_H_


#include <Stream.h>
#include "pinMap.h"

#define START_FRAME1  0x42
#define START_FRAME2  0x4D

#define OFFSET_PM1    10
#define OFFSET_PM25   12
#define OFFSET_PM10   14





struct pm_struct{
  uint16_t pm1 = 0;
  uint16_t pm25 = 0;
  uint16_t pm10 = 0;  
};



class PMS7003{
    public:
        PMS7003(void);
        void power_on(bool stat);
        void reset(void);
        bool parse_sensor_data(uint8_t *input_buf,struct pm_struct *sensor_struct);
        bool read_air_sensor(Stream &ser);
        uint16_t get_pm1(void);
        uint16_t get_pm10(void);
        uint16_t get_pm25(void);
};



#endif