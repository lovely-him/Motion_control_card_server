
#include "him_key.h"
#include "him_led.h"

#define KEY_A   15
#define KEY_B   16
#define KEY_C   17

/* 按键初始化 */
void key_init(void)
{
    /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO. Consult the
       Technical Reference for a list of pads and their default
       functions.)
    */
    gpio_pad_select_gpio(KEY_A); 
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(KEY_A, GPIO_MODE_INPUT);
    
    gpio_pad_select_gpio(KEY_B); 
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(KEY_B, GPIO_MODE_INPUT);

    gpio_pad_select_gpio(KEY_C); 
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(KEY_C, GPIO_MODE_INPUT);
}

/* 读取按键 */
int key_get(void)
{
    /* 按顺序判断执行 */
    if (gpio_get_level(KEY_A) == 0)
    {
        return 1;
    }
    else if (gpio_get_level(KEY_B) == 0)
    {
        return 2;
    }
    else if (gpio_get_level(KEY_C) == 0)
    {
        return 3;
    }

    return 0;
}

