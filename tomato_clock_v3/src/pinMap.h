

#ifndef _PINMAP_H_
#define _PINMAP_H_






#define USB_DET     A1     

//buzzer
#define BUZZER      A5     

//swtich
#define BTN_LEFT    S0
#define BTN_RIGHT   S1
#define BTN_MID     D10     //this pin will be used as a wakeup source

//3.3v and 5V peripherial enable
#define PERI_EN     S3

//PSW or fix pwm mode selection for 3.3v dcdc regulator
#define DCDC_MODE   S5

//charger IC
#define CHARGE      S6

//accel IC
#define IMU_INT     D2

//pms7003
#define PMS7003_RST D6     
#define PMS7003_TXD D8
#define PMS7003_RXD D9


#endif