
#ifndef __LED_H__
#define __LED_H__

#include "sdkconfig.h"
#include "driver/gpio.h"

/* Can use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/

void led_init(void);
void led_on(void);
void led_off(void);

#endif /* __LED_H__ */

