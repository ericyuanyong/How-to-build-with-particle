

#include "button.h"



Button::Button(uint8_t pin){
    pinMode(pin,INPUT);
    _pin = pin;
}

//short press will return 1
//long press will return 2
unsigned char Button::pressAction(){
    static unsigned char stage = 0;
    static unsigned char button_status = 0;
    static unsigned long long_press_timeout = 0;
    switch(stage){
        case 0:
            if(digitalRead(_pin)==0){
                stage = 1;  
            }   
            button_status = 0; 
            break;
        case 1:
            if(digitalRead(_pin)==0){      //a valid press action
                stage = 2;
                long_press_timeout = millis();  //start to record if there is a long press action for middle button
            }else{
                stage = 0;      //not a valid action. go back to original stage.
            }
            button_status = 0;
            break;
        case 2:
            if(digitalRead(_pin)){   //go to initial stage when released, otherwise just stay in this stage.
                stage = 0;
                button_status = 1;      //short press
            }else{
                if(millis()-long_press_timeout>=2000){
                    button_status = 2;   //long press 
                    stage = 3;
                }
            }
            break;
        case 3:
            if(digitalRead(_pin)){        //finally released.
                stage = 0;
            }
            button_status = 0;
            break;
        default:
            break;
    }
    return button_status;
}
