
#ifndef __MCPWM_H__
#define __MCPWM_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

#define PULSE_REV 6400          // 步进电机驱动器设置，参数 pulse_rev ,代表一圈需多少脉冲。
#define MAX_ANGULAR_V (2) // 最大速度

extern void him_mcpwm_init(void);
extern void mcpwm_set_hz_a(float frequency);
extern void mcpwm_set_hz_b(float frequency);



#endif /* __MCPWM_H__ */