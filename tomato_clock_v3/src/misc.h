

#include "application.h"
#include "pinMap.h"
#include "button.h"

#define DCDC_PSM        1
#define DCDC_FIX_PWM    2

class MiscClass{
    public:
        void initiaize(void);
        void dcdc_mode(uint8_t mode);
        void ext_power_on(bool stat);
        uint8_t get_current_display_item(uint8_t direction);
        bool charge_service(void);
};