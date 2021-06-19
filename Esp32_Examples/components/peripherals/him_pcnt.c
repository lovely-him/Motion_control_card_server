
#include "him_pcnt.h"

#define HIM_PCNT_UNIT     PCNT_UNIT_0           // 单元号
#define PHASE_A_GPIO_NUM  15                    // AB相引脚
#define PHASE_B_GPIO_NUM  4                    // 
#define HIM_PCNT_DEFAULT_HIGH_LIMIT  (10)      // 最大/小值
#define HIM_PCNT_DEFAULT_LOW_LIMIT   (-10)

int him_accumu_count = 0;                       // 累加溢出数
//user's ISR service handle 用户的ISR服务句柄 好像没什么用啊，只在初始化时用了
pcnt_isr_handle_t him_user_isr_handle = NULL; 

// 获取计数
int him_get_counter_value(void)
{
    int16_t val = 0;
    pcnt_get_counter_value(HIM_PCNT_UNIT, &val);
    printf("val = %d; him_accumu_count = %d;\r\n",val,(val + him_accumu_count));
    return val + him_accumu_count;
}

// 中断服务函数，就是不需要传入参数也要设置应该指针
void IRAM_ATTR him_pcnt_handler(void *arg)
{
    uint32_t intr_status = PCNT.int_st.val;                 // 读取寄存器

    if(intr_status & (BIT(HIM_PCNT_UNIT)))                  // 判断中断标志位
    {
        if (PCNT.status_unit[HIM_PCNT_UNIT].val & PCNT_EVT_H_LIM)   // 查看是哪个标志 
        {                      
            him_accumu_count+= HIM_PCNT_DEFAULT_HIGH_LIMIT;     
        } 
        else if (PCNT.status_unit[HIM_PCNT_UNIT].val & PCNT_EVT_L_LIM) 
        {
            him_accumu_count += HIM_PCNT_DEFAULT_LOW_LIMIT;
        }
        PCNT.int_clr.val = BIT(HIM_PCNT_UNIT);              // 清楚中断标志位
    }
}

// 初始化函数
void him_pcnt_init(void)
{
    /*准备配置的PCNT单位*/
    /* Prepare configuration for the PCNT unit */
    pcnt_config_t pcnt_config = {
        //设置PCNT输入信号和控制gpio
        // Set PCNT input signal and control GPIOs
        .pulse_gpio_num = PHASE_A_GPIO_NUM,
        .ctrl_gpio_num = PHASE_B_GPIO_NUM,
        .channel = PCNT_CHANNEL_0,
        .unit = HIM_PCNT_UNIT,
        //在脉冲输入的正/负边缘上做什么?
        // What to do on the positive / negative edge of pulse input?
        .pos_mode = PCNT_COUNT_INC,   // Count up on the positive edge 数一数积极的一面
        .neg_mode = PCNT_COUNT_DIS,   // Keep the counter value on the negative edge 保持计数器值在负边缘
        //当控制输入低或高时怎么办?
        // What to do when control input is low or high?
        .lctrl_mode = PCNT_MODE_REVERSE, // Reverse counting direction if low 如果计数方向低，则反向计数
        .hctrl_mode = PCNT_MODE_KEEP,    // Keep the primary counter mode if high 如果高，则保持主计数器模式
        //设置最大和最小限制值来监视
        // Set the maximum and minimum limit values to watch
        .counter_h_lim = HIM_PCNT_DEFAULT_HIGH_LIMIT,
        .counter_l_lim = HIM_PCNT_DEFAULT_LOW_LIMIT,
    };
    /*初始化PCNT单元*/
    /* Initialize PCNT unit */
    pcnt_unit_config(&pcnt_config);

#if 1   // 如果要开启正交解码就打开。
    pcnt_config.pulse_gpio_num = PHASE_A_GPIO_NUM,
    pcnt_config.ctrl_gpio_num = PHASE_B_GPIO_NUM,
    pcnt_config.channel = PCNT_CHANNEL_0,
    pcnt_config.pos_mode = PCNT_COUNT_DEC,   
    pcnt_config.neg_mode = PCNT_COUNT_INC, 
    pcnt_unit_config(&pcnt_config);

    pcnt_config.pulse_gpio_num = PHASE_B_GPIO_NUM,
    pcnt_config.ctrl_gpio_num = PHASE_A_GPIO_NUM,
    pcnt_config.channel = PCNT_CHANNEL_1,
    pcnt_config.pos_mode = PCNT_COUNT_INC,   
    pcnt_config.neg_mode = PCNT_COUNT_DEC, 
    pcnt_unit_config(&pcnt_config);
#endif

    /*配置和启用输入过滤器*/
    /* Configure and enable the input filter */
    pcnt_set_filter_value(HIM_PCNT_UNIT, 100);
    pcnt_filter_enable(HIM_PCNT_UNIT);

    /*设置阈值0和1，并启用事件监视*/
    /* Set threshold 0 and 1 values and enable events to watch */
    // pcnt_set_event_value(HIM_PCNT_UNIT, PCNT_EVT_THRES_1, PCNT_THRESH1_VAL);
    // pcnt_event_enable(HIM_PCNT_UNIT, PCNT_EVT_THRES_1);
    // pcnt_set_event_value(HIM_PCNT_UNIT, PCNT_EVT_THRES_0, PCNT_THRESH0_VAL);
    // pcnt_event_enable(HIM_PCNT_UNIT, PCNT_EVT_THRES_0);
    /*在0，最大和最小限值上启用事件*/
    /* Enable events on zero, maximum and minimum limit values */
    // pcnt_event_enable(HIM_PCNT_UNIT, PCNT_EVT_ZERO);
    pcnt_event_enable(HIM_PCNT_UNIT, PCNT_EVT_H_LIM);
    pcnt_event_enable(HIM_PCNT_UNIT, PCNT_EVT_L_LIM);

    /*初始化PCNT的计数器*/
    /* Initialize PCNT's counter */
    pcnt_counter_pause(HIM_PCNT_UNIT);
    pcnt_counter_clear(HIM_PCNT_UNIT);

    /*为PCNT单元注册ISR处理程序并启用中断*/
    /* Register ISR handler and enable interrupts for PCNT unit */
    pcnt_isr_register(him_pcnt_handler, NULL, 0, &him_user_isr_handle);
    pcnt_intr_enable(HIM_PCNT_UNIT);

    /*所有的设置，现在去计数*/
    /* Everything is set up, now go to counting */
    pcnt_counter_resume(HIM_PCNT_UNIT);
}