
#ifndef __HIM_PCNT_H__
#define __HIM_PCNT_H__


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "driver/gpio.h"
#include "driver/pcnt.h"


int him_get_counter_value(void);
void him_pcnt_init(void);


#endif /* __HIM_PCNT_H__ */