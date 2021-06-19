
#include "him_mcpwm.h"

/* 电机1 */
#define HIM_MCPWM_PINA_PUL  33       // 脉冲引脚
#define HIM_MCPWM_PINA_DIR  32      // 方向引脚
#define HIM_MCPWM_PINA_EN   14      // 使能引脚

/* 电机2 */
#define HIM_MCPWM_PINB_PUL  13       // 脉冲引脚
#define HIM_MCPWM_PINB_DIR  12       // 方向引脚
#define HIM_MCPWM_PINB_EN   4        // 使能引脚

// gpio: io_num=35 can only be input 只能输入
// gpio: io_num=34 can only be input 只能输入

/* 两个电机的旋转方向 */
#define HIM_MCPWM_DIRA_H     0      // 方向引脚，正转输出
#define HIM_MCPWM_DIRA_L     1      // 方向引脚，反转输出

#define HIM_MCPWM_DIRB_H     0      // 方向引脚，正转输出
#define HIM_MCPWM_DIRB_L     1      // 方向引脚，反转输出

#define HIM_MCPWM_EN_H     0       // 使能引脚，能输出
#define HIM_MCPWM_EN_L     1       // 使能引脚，不输出

/* 在头文件里修改 */
// #define PULSE_REV 6400          // 步进电机驱动器设置，参数 pulse_rev ,代表一圈需多少脉冲。
// #define MAX_ANGULAR_V (2) // 最大速度

/* 初始化脉冲输出（含方向输出） */
void him_mcpwm_init(void)
{
    // 电机使能 
    gpio_pad_select_gpio(HIM_MCPWM_PINA_EN);
    gpio_set_direction(HIM_MCPWM_PINA_EN, GPIO_MODE_OUTPUT);
    gpio_set_level(HIM_MCPWM_PINA_EN, HIM_MCPWM_EN_L);

    // 初始化方向引脚
    gpio_pad_select_gpio(HIM_MCPWM_PINA_DIR);
    gpio_set_direction(HIM_MCPWM_PINA_DIR, GPIO_MODE_OUTPUT);

    // 单元0模块0通道A
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, HIM_MCPWM_PINA_PUL);      

    mcpwm_config_t pwm_config_a = {
        .frequency = 1000,                  // 频率
        .cmpr_a = 50.0,                     // A通道初始占空比
        .cmpr_b = 0.0,                     // B通道初始占空比
        .counter_mode = MCPWM_UP_COUNTER,   // 计数器类型
        .duty_mode = MCPWM_DUTY_MODE_0      // 占空比类型
    };

    // 初始化配置
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config_a);   

    
    // 电机使能 
    gpio_pad_select_gpio(HIM_MCPWM_PINB_EN);
    gpio_set_direction(HIM_MCPWM_PINB_EN, GPIO_MODE_OUTPUT);
    gpio_set_level(HIM_MCPWM_PINB_EN, HIM_MCPWM_EN_L);

    // 初始化方向引脚
    gpio_pad_select_gpio(HIM_MCPWM_PINB_DIR);
    gpio_set_direction(HIM_MCPWM_PINB_DIR, GPIO_MODE_OUTPUT);

    // 单元0模块1通道A
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, HIM_MCPWM_PINB_PUL);      

    mcpwm_config_t pwm_config_b = {
        .frequency = 1000,                  // 频率
        .cmpr_a = 50.0,                     // A通道初始占空比
        .cmpr_b = 0.0,                     // B通道初始占空比
        .counter_mode = MCPWM_UP_COUNTER,   // 计数器类型
        .duty_mode = MCPWM_DUTY_MODE_0      // 占空比类型
    };

    // 初始化配置
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &pwm_config_b);   
}

/* 设置电机A的频率 */
void mcpwm_set_hz_a(float frequency)
{
    /* 限幅输入 */
    if (frequency > MAX_ANGULAR_V)
        frequency = MAX_ANGULAR_V;
    else if (frequency < -MAX_ANGULAR_V)
        frequency = -MAX_ANGULAR_V;

    // 不加个中间值，计算得到整数再判断，直接判断小数会在取整时还是会出现0，频率不可以配置0.会报错复位
    int32_t frequency_i = (int32_t)(frequency * PULSE_REV); 

    if (frequency_i > 0)
    {
        gpio_set_level(HIM_MCPWM_PINA_DIR, HIM_MCPWM_DIRA_H); // 方向引脚
        gpio_set_level(HIM_MCPWM_PINA_EN, HIM_MCPWM_EN_H); // 使能引脚 使能
        // mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); // 恢复原来的占空比
        mcpwm_set_frequency(MCPWM_UNIT_0, MCPWM_TIMER_0, (uint32_t)frequency_i); // 改变频率
    }
    else if (frequency_i < 0)
    {
        frequency_i = -frequency_i; // 改为正值
        gpio_set_level(HIM_MCPWM_PINA_DIR, HIM_MCPWM_DIRA_L);
        gpio_set_level(HIM_MCPWM_PINA_EN, HIM_MCPWM_EN_H); 
        // mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); 
        mcpwm_set_frequency(MCPWM_UNIT_0, MCPWM_TIMER_0, (uint32_t)frequency_i);
    }
    else
    {
        gpio_set_level(HIM_MCPWM_PINA_EN, HIM_MCPWM_EN_L); // 使能引脚 失能
        // mcpwm_set_frequency(MCPWM_UNIT_0, MCPWM_TIMER_0, (1000)); // 改变频率
        // mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A);      // 完全置低
    }
}

/* 设置电机B的频率 */
void mcpwm_set_hz_b(float frequency)
{
    /* 限幅输入 */
    if (frequency > MAX_ANGULAR_V)
        frequency = MAX_ANGULAR_V;
    else if (frequency < -MAX_ANGULAR_V)
        frequency = -MAX_ANGULAR_V;

    int32_t frequency_i = (int32_t)(frequency * PULSE_REV); 

    if (frequency_i > 0)
    {
        gpio_set_level(HIM_MCPWM_PINB_DIR, HIM_MCPWM_DIRB_H); // 方向引脚
        gpio_set_level(HIM_MCPWM_PINB_EN, HIM_MCPWM_EN_H); // 使能引脚 使能
        // mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); // 恢复原来的占空比
        mcpwm_set_frequency(MCPWM_UNIT_0, MCPWM_TIMER_1, (uint32_t)frequency_i); //改变频率
    }
    else if (frequency_i < 0)
    {
        frequency_i = -frequency_i; // 改为正值
        gpio_set_level(HIM_MCPWM_PINB_DIR, HIM_MCPWM_DIRB_L);
        gpio_set_level(HIM_MCPWM_PINB_EN, HIM_MCPWM_EN_H); // 使能引脚 失能
        // mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); 
        mcpwm_set_frequency(MCPWM_UNIT_0, MCPWM_TIMER_1, (uint32_t)frequency_i);
    }
    else
    {
        gpio_set_level(HIM_MCPWM_PINB_EN, HIM_MCPWM_EN_L); // 使能引脚 失能
        // mcpwm_set_frequency(MCPWM_UNIT_0, MCPWM_TIMER_1, (1000)); //改变频率
        // mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A);      // 完全置低
    }
}


/* 外部调用 */