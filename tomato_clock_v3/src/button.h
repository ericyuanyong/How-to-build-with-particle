

#ifndef _BUTTON_H_
#define _BUTTON_H_

#include "Particle.h"
#include "pinMap.h"

//direction for rotary switch
#define NO_DIR      0
#define CW          1
#define CCW         2

class Button{
    public:
        Button(uint8_t pin);
        unsigned char  pressAction();

    private:
        uint8_t _pin;
};




#endif