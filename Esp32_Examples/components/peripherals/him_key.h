
#ifndef __HIM_KEY_H__
#define __HIM_KEY_H__

#include "sdkconfig.h"
#include "driver/gpio.h"

void key_init(void);
int key_get(void);



#endif /* __HIM_KEY_H__ */