
#include "him_led.h"

// 我自己买的开发版上的板载led引脚是 IO2，而且是高电平亮灯
#define HIM_BLINK_GPIO 2

/* 初始化提示灯 */
void led_init(void)
{
    /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO. Consult the
       Technical Reference for a list of pads and their default
       functions.)
    */
    gpio_pad_select_gpio(HIM_BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(HIM_BLINK_GPIO, GPIO_MODE_OUTPUT);
}

/* 提示灯亮 */
void led_on(void)
{
    gpio_set_level(HIM_BLINK_GPIO, 1);
}

/* 提示灯灭 */
void led_off(void)
{
    gpio_set_level(HIM_BLINK_GPIO, 0);
}

