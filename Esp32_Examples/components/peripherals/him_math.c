

#include "him_math.h"

/*==============================================================
 * @brief
 *          void.定义所需的常量类型
 * @Note
 *          void.
 *==============================================================*/
static TimerHandle_t math_timer; // 定时器句柄
Math_s M; // 结构体变量

#define TIMER_PERIOD_MS 10 // 定时器中断周期 单位ms

/* 
（单螺纹）螺距 = 导程 = 5mm，形成约10.7cm，取整作10cm ，步进电机驱动器 PULSE_REV = 6400 ，
*/
#define MAX_TURNS_NUM (128000 / PULSE_REV) //最大脉冲数 约为 136960（取整） ，转换为圈数20 = 行程100mm
#define RESET_SPEED (1) // 复位速度

// 直接保留几位小数，num填10.0的次方得数，比如一位小数就填10.0，含四舍五入
#define FLOAT_DECIMAL(data, num) ((int)((num) * (data) + 0.5) / (num)) 

static const char *HIM = "him_math";

/*==============================================================
 * @brief
 *          void.定义所需的变量类型
 * @Note
 *          void.
 *==============================================================*/
typedef enum _math_e    // 
{
    /* 模式枚举 */
    mods_a,             // 模式A：手动寻找零点
    mods_b,             // 模式B：点位移 
    mods_c,             // 模式C：保留 
    mods_d,             // 模式D：开机模式

    /* 电机状态枚举 */
    motor_run_H,          // 电机运行
    motor_run_L,          // 电机运行
    motor_stop,           // 电机停止  
    
}Math_e;                // 所有标志枚举

/*==============================================================
 * @brief
 *          void.重置电机状态
 * @Note
 *          void.
 *==============================================================*/
void Motor_memset(void)
{
    memset(&M.actual, 0, sizeof(M.actual));
    M.actual[0].f = motor_stop;
    M.actual[1].f = motor_stop;
    memcpy(M.expect,M.actual,sizeof(M.expect));
}
/*==============================================================
 * @brief
 *          void.计算角速度
 * @Note
 *          void.根据期望距离和实际距离
 *==============================================================*/
float calculate_av(Motor_s e_i, Motor_s a_i)
{
    if (fabs(e_i.c - a_i.c + a_i.t) > (2 * e_i.t) && e_i.t != 0)
    {
        float ret_i = 0;
        
        /* 直线恒定加速 */
        ret_i = (float)e_i.t * 2 / MAX_ANGULAR_V ; // 根据 路程 和 目标速度 ，得出加速时间
        ret_i = (float)MAX_ANGULAR_V / (ret_i * 1000 / TIMER_PERIOD_MS); // 根据 加速时间 ，得出每次加速的变化量

        if (fabs(a_i.t) < e_i.t)
        {
            return (ret_i); // 直线加速，按着1秒后达到目标速度的程度返回
        }
        else if (fabs(e_i.c - a_i.c) <= e_i.t)
        {
            return (-ret_i); // 减速过程
        }
        else
        {
            return 0;
        }
    }

    return 0;
}
/*==============================================================
 * @brief
 *          void.根据目前坐标计算期望坐标
 * @Note
 *          void.
 *==============================================================*/
void Circle_coordinates(void)
{
    /* 写死标志位 */
    static int f_i = 0;

    /* 预设一堆数据 */
    const float feed_rate = (0.5 / 5); // 一圈等于5mm，进给0.5，算得就是0.1.

    const float C_r = 5; // 半径 r = 2.5cm = 5 c ，换算为电机需要转动的圈数
    
    float dot_c[2]; // 圆心位置，处于线性模组的中心，全长是 20 圈，
    dot_c[0] = 10; // 0 代表 x
    dot_c[1] = 10; // 1 代表 y

    float dot_a[2]; // 实际坐标
    dot_a[0] = FLOAT_DECIMAL(M.actual[0].c, 1000.0);
    dot_a[1] = FLOAT_DECIMAL(M.actual[1].c, 1000.0);

    float dot_e[2]; // 期望坐标
    dot_e[0] = M.expect[0].c; // 注意这里暂存了变量
    dot_e[1] = M.expect[1].c;

    float a_i = (dot_a[0] - dot_c[0]); // 临时变量
    float b_i = (dot_a[1] - dot_c[1]);
    
    /* 如果当前坐标不在圆上 */
    if (fabs(a_i * a_i + b_i * b_i - C_r * C_r) > 0.001 && f_i == 0)
    {
        dot_e[0] = 10;
        dot_e[1] = 5;
    }
    else
    {
        f_i = 1;
        if (dot_a[0] <= 10 && dot_a[1] < 15 && dot_a[1] >= 5) // y != 15 , y = 5 // 
        {

            /* 设定y轴偏移恒定，算x轴的数值 */
            b_i = dot_a[1] + feed_rate; // y往左偏移
            if (b_i > 15) // 防止超出边框
                b_i = 15;

            a_i = (b_i - dot_c[1]); // 中间变量计算
            a_i = C_r * C_r - a_i * a_i;

            if (a_i >= 0) // 开平方不能是负数
            {
                a_i = sqrt(a_i); // 开平方根
                a_i = (dot_c[0] - a_i); // 取圆心上方的点
                if (fabs(a_i - dot_a[0]) >= feed_rate) // 增加判断，当两轴移动不对称时就不赋值了
                {
                    dot_e[0] = a_i;
                    if (dot_e[0] < 5) // 防止超出边框
                        dot_e[0] = 5;
                }
                dot_e[1] = b_i; // 取固定偏移点
            }
            else // 如果不进入上面，期望值就不会更新
            {
                ESP_LOGI(HIM, "Circle_coordinates_sqrt_1"); // 调试打印
            }
        }
        else // 剩下的就是另一种情况
        {
            /* 设定y轴偏移恒定，算x轴的数值 */
            b_i = dot_a[1] - feed_rate; // y往左偏移
            if (b_i < 5) // 防止超出边框
                b_i = 5;

            a_i = (b_i - dot_c[1]); // 中间变量计算
            a_i = C_r * C_r - a_i * a_i;
            
            if (a_i >= 0)
            {
                a_i = sqrt(a_i); // 开平方根
                a_i = (dot_c[0] + a_i); // 取圆心下方的点
                if (fabs(a_i - dot_a[0]) >= feed_rate) // 增加判断，当两轴移动不对称时就不赋值了
                {
                    dot_e[0] = a_i;
                    if (dot_e[0] > 15) // 防止超出边框
                        dot_e[0] = 15;
                }
                dot_e[1] = b_i; // 取固定偏移点
            }
            else
            {
                ESP_LOGI(HIM, "Circle_coordinates_sqrt_2"); // 调试打印
            }
        }
    }

    /* 输出期望坐标 */
    M.expect[0].c = FLOAT_DECIMAL(dot_e[0], 100.0);
    M.expect[1].c = FLOAT_DECIMAL(dot_e[1], 100.0);
    
    // ESP_LOGI(HIM, "%d:%d", (int)(M.actual[0].c*10000), (int)(M.actual[1].c*10000)); // 调试打印
}
/*==============================================================
 * @brief
 *          void.点对点运动，无加减速
 * @Note
 *          void.指定期望运动坐标和电机。
 *==============================================================*/
void Motor_coordinates(void)
{
    /* 保存运动状态 */
    int motor_run_o= motor_stop;

    /* 累加圈数/累计脉冲，绝对坐标，不会清零 */
    M.actual[0].c += M.actual[0].w * TIMER_PERIOD_MS / 1000;
    M.actual[1].c += M.actual[1].w * TIMER_PERIOD_MS / 1000;
    M.actual[0].c = FLOAT_DECIMAL(M.actual[0].c, 1000.0); // 保留3位小数
    M.actual[1].c = FLOAT_DECIMAL(M.actual[1].c, 1000.0);

    /* 计算下一次期望坐标 */
    if (M.mods_cf == mods_b) // 只有在该标志位处于模式b时才计算
        Circle_coordinates();

    /* 判断运动状态 */
    for (int motor_n = 0; motor_n < 2; motor_n++)
    {
        // ESP_LOGI(HIM, "%d:%d:%d:%d", (int)(M.actual[motor_n].w), (int)(M.expect[motor_n].c*100), (int)(M.actual[motor_n].c*100), motor_run_o); // 调试打印

        /* 加个限定差大小条件，为了避免单片机数据精度问题 */
        if (M.expect[motor_n].c > M.actual[motor_n].c && 
        fabs(M.expect[motor_n].c - M.actual[motor_n].c) > 0.001) // 假设 角速度 不会小于 0.1，导致角度小于0.0001.
            motor_run_o = motor_run_H;
        else if (M.expect[motor_n].c < M.actual[motor_n].c && 
        fabs(M.expect[motor_n].c - M.actual[motor_n].c) > 0.001)
            motor_run_o = motor_run_L;
        else
            motor_run_o = motor_stop;
        
        /* 判断下一次角速度输出 */
        if (motor_run_o == motor_run_H)
        {
            M.actual[motor_n].w = 1; //添加判断，剩下的距离是否能满足速度要求，不能就按着距离算速度
            if ((M.expect[motor_n].c - M.actual[motor_n].c) * 1000 / TIMER_PERIOD_MS < M.actual[motor_n].w )
            {
                M.actual[motor_n].w  = (M.expect[motor_n].c - M.actual[motor_n].c) * 1000 / TIMER_PERIOD_MS;
            }
        }
        else if (motor_run_o == motor_run_L)
        {
            M.actual[motor_n].w = -1;
            if ((M.expect[motor_n].c - M.actual[motor_n].c) * 1000 / TIMER_PERIOD_MS > M.actual[motor_n].w )
            {
                M.actual[motor_n].w  = (M.expect[motor_n].c - M.actual[motor_n].c) * 1000 / TIMER_PERIOD_MS;
            }
        }
        else if (motor_run_o == motor_stop)
            M.actual[motor_n].w = 0;
    }

    /* 两轴电机输出：上下 */
    mcpwm_set_hz_a(M.actual[0].w);
    mcpwm_set_hz_b(M.actual[1].w);
}
/*==============================================================
 * @brief
 *          void.按键执行
 * @Note
 *          void.
 *==============================================================*/
/* 左键 */
void key_1_call(void)
{
    static int key_1_mods_2_s = mods_a; // 在模式二时，按键1也有多个模式

    if (M.mods == mods_a)               // A模式，按键直接设置角速度
    {
        if (M.actual[0].w > 0)          // 从正转切换到反转
        {
            M.actual[0].w = -RESET_SPEED;
        }
        else if (M.actual[0].w < 0)     // 从反转切换到停止
        {
            M.actual[0].w = 0;
        }
        else if (M.actual[0].w == 0)    // 从停止切换到正转
        {
            M.actual[0].w = RESET_SPEED;
        }

        ESP_LOGI(HIM, " M.actual[0].w =  %d", (int)(M.actual[0].w*10)); // 调试打印
        ESP_LOGI(HIM, " M.actual[0].c =  %d ", (int)(M.actual[0].c*100)); // 调试打印
    }
    else if (M.mods == mods_b)          // B模式，按键设置期望坐标，自动计算角速度
    {
        if (key_1_mods_2_s == mods_a)  // 走向中心点
        {
            key_1_mods_2_s = mods_b;
            M.expect[0].c = MAX_TURNS_NUM / 2; // 设置期望坐标
            M.expect[1].c = MAX_TURNS_NUM;
        }
        else if (key_1_mods_2_s == mods_b)  // 走向中心点
        {
            key_1_mods_2_s = mods_c;
            M.expect[0].c = MAX_TURNS_NUM; // 设置期望坐标
            M.expect[1].c = MAX_TURNS_NUM / 2;
        }
        else if (key_1_mods_2_s == mods_c)  // 走向中心点
        {
            key_1_mods_2_s = mods_d;
            M.expect[0].c = MAX_TURNS_NUM / 2; // 设置期望坐标
            M.expect[1].c = MAX_TURNS_NUM;
        }
        else if (key_1_mods_2_s == mods_d)  // 走向中心点
        {
            key_1_mods_2_s = mods_a;
            M.expect[0].c = 0; // 设置期望坐标
            M.expect[1].c = 0;
        }

        /* 设置其他参数 */
        for (int i = 0; i < 2; i++)
        {
            if (M.expect[i].c != M.actual[i].c) // 判断坐标是否不一样
            {
                if (M.expect[i].c > M.actual[i].c) // 设置期望正反转
                    M.expect[i].f = motor_run_H;
                else
                    M.expect[i].f = motor_run_L;

                M.expect[i].t = MAX_TURNS_NUM / 10; // 设置期望加速距离:总行程的10%
                M.actual[i].t = 0; // 设置相对距离/相对坐标
                M.actual[i].w = 0; // 重置速度
            }
        }
        
        ESP_LOGI(HIM, " key_1_mods_2_s =  %d", key_1_mods_2_s); // 调试打印
    }
    else if (M.mods == mods_c)          // C模式，切换运动模式
    {
        if (M.mods_cf == mods_a)        // 轮询模式
        {
            M.mods_cf = mods_b;         // 模式b，不点动，固定走圆弧
        }
        else if (M.mods_cf == mods_b)
        {
            M.mods_cf = mods_a;         // 模式a，点动，按键控制
        }

        ESP_LOGI(HIM, " M.mods_cf =  %d", M.mods_cf); // 调试打印
    }
}
/* 右键 */
void key_2_call(void)
{
    if (M.mods == mods_a)               // A模式
    {
        if (M.actual[1].w > 0)          // 同上
        {
            M.actual[1].w = -RESET_SPEED;
        }
        else if (M.actual[1].w < 0)
        {
            M.actual[1].w = 0;
        }
        else if (M.actual[1].w == 0)
        {
            M.actual[1].w = RESET_SPEED;
        }
        
        ESP_LOGI(HIM, " M.actual[1].w =  %d ", (int)(M.actual[1].w*10)); // 调试打印
        ESP_LOGI(HIM, " M.actual[1].c =  %d ", (int)(M.actual[1].c*100)); // 调试打印
    }
    else if (M.mods == mods_c)               // C模式
    {
        if (M.mods_cf == mods_a)            // 当标志位位模式a时，表示可以控制点运动
        {
            if (M.expect[1].c == 0)          // 同上
            {
                M.expect[1].c = MAX_TURNS_NUM / 2;
            }
            else if (M.expect[1].c == MAX_TURNS_NUM / 2)
            {
                M.expect[1].c = MAX_TURNS_NUM;
            }
            else                            // 如果是其他情况就重置，方便快捷，适应性强，666……
            {
                M.expect[1].c = 0;
            }

            M.expect[0].c = M.expect[1].c;
            ESP_LOGI(HIM, " M.expect[0].c =  %d ", (int)(M.expect[0].c*100)); // 调试打印
            ESP_LOGI(HIM, " M.expect[1].c =  %d ", (int)(M.expect[1].c*100)); // 调试打印
        }
    }
}
/* 中键 */
void key_3_call(void)
{
    if (M.mods == mods_d)           // 开机模式
    {
        Motor_memset();             // 开机运行，初始化参数，防止出错
        M.mods = mods_a;
    }
    else if (M.mods == mods_a)      // 正常模式
    {
        Motor_memset();             // 从模式A到模式B，复位
        M.mods = mods_b;
    }
    else if (M.mods == mods_b)
    {
        Motor_memset();             // 从模式B到模式C，复位
        M.mods = mods_c;
    }
    else if (M.mods == mods_c)
    {
        Motor_memset();             // 从模式C到模式A，复位
        M.mods = mods_a;
    }
    else
    {
        M.mods = mods_d;            // 意料之外，复位开机模式
    }

    ESP_LOGI(HIM, " M.mods =  %d ", M.mods); // 调试打印
}

/*==============================================================
 * @brief
 *          void.模式执行
 * @Note
 *          void.
 *==============================================================*/
/* 模式A：手动寻找零点 */
void mods_a_call(void)
{
    /* 省略计算步骤，转速直接用按键控制 */

    /* 最终输出pwm */
    mcpwm_set_hz_a(M.actual[0].w);
    mcpwm_set_hz_b(M.actual[1].w);
}
/* 模式B：点位移 */
void mods_b_call(void)
{
    /*  */
    for (int i = 0; i < 2; i++)
    {
        // 只有当实际路程在范围内时才会移动，且期望标志处于能动状态
        if (M.actual[i].c >= 0 && M.actual[i].c <= MAX_TURNS_NUM && 
        (M.expect[i].f == motor_run_H || M.expect[i].f == motor_run_L))
        {
            /* 累加圈数/累计脉冲，绝对坐标，不会清零 */
            M.actual[i].c += M.actual[i].w * TIMER_PERIOD_MS / 1000;
            /* 累计移动距离，相对坐标，会清零 */
            M.actual[i].t += M.actual[i].w * TIMER_PERIOD_MS / 1000;

            /* 计算期望 */
            if (M.expect[i].f == motor_run_H)
                M.actual[i].w += calculate_av(M.expect[i], M.actual[i]);
            if (M.expect[i].f == motor_run_L)
                M.actual[i].w -= calculate_av(M.expect[i], M.actual[i]);

            /* 判断是否到达目标点 */
            if (M.expect[i].f == motor_run_H && 
            (M.expect[i].c <= M.actual[i].c || 
            M.actual[i].w < 0))
            {
                M.actual[i].w = 0;
                M.expect[i].f = motor_stop; //其他内容不重置了，每次都会覆盖
                continue; // 当要正转时大于了目标
            }
            else if (M.expect[i].f == motor_run_L && 
            (M.expect[i].c >= M.actual[i].c ||
            M.actual[i].w > 0))
            {
                M.actual[i].w = 0;
                M.expect[i].f = motor_stop;
                continue;
            }                

            /* 限幅输出 */
            if (M.actual[i].w > MAX_ANGULAR_V)
                M.actual[i].w = MAX_ANGULAR_V;
            else if (M.actual[i].w < -MAX_ANGULAR_V)
                M.actual[i].w = -MAX_ANGULAR_V;
        }
        else
        {
            M.actual[i].w = 0; // 如果不满足要求，就是有问题，设定角速度为0，且停止期望转动
            M.expect[i].f = motor_stop;
        }
    }

    // ESP_LOGI(HIM, " M.expect[%d].f = %d", 0, (int)(M.expect[0].f*1)); // 调试打印

    /* 最终输出pwm */
    mcpwm_set_hz_a((float)M.actual[0].w);
    mcpwm_set_hz_b((float)M.actual[1].w);
}
/* 模式C：保留 */
void mods_c_call(void)
{
    Motor_coordinates();
}
/* 模式D：开机模式 */
void mods_d_call(void)
{
    // 设置频率为0，使能引脚 会 失能
    mcpwm_set_hz_a(0);
    mcpwm_set_hz_b(0);
}

/*==============================================================
 * @brief
 *          void.周期定时器函数
 * @Note
 *          void.
 *==============================================================*/
static void Math_Timer_Handle(TimerHandle_t xTimer)
{
    /* 分别执行不同模式程序 */
    switch(M.mods)
    {
        case mods_a:mods_a_call();break;
        case mods_b:mods_b_call();break;
        case mods_c:mods_c_call();break;
        default:mods_d_call();break;
    }

    M.flag = M.mods;
}
/*==============================================================
 * @brief
 *          void.初始化所有参数 
 * @Note
 *          void.
 *==============================================================*/
void age_init(void)
{
    M.mods = mods_d;            // 开机模式
    M.flag = M.mods;
    M.mods_cf = mods_a;         // 初始化标志

    /* 初始化电机状态 */
    Motor_memset();
}

/*==============================================================
 * @brief
 *          void.初始化所有功能
 * @Note
 *          void.
 *==============================================================*/
void math_init(void)
{
    him_mcpwm_init();  // 初始化电机输出
    key_init(); // 初始化按键
    
    // 创建一个新的软件计时器实例，并返回一个句柄，通过这个句柄可以引用创建的软件计时器。
    math_timer = xTimerCreate("Websocket shutdown timer",                       // 只是一个文本名称，不被内核使用。
                            TIMER_PERIOD_MS / portTICK_PERIOD_MS,               // 计时器周期(单位是tick)。
                            pdTRUE,                                             // 计时器将在到期时自动重新加载。（会）
                            NULL,                                               // 为每个计时器分配一个唯一的id等于它的数组索引。
                            Math_Timer_Handle);                                 // 每个计时器在到期时调用同一个回调。

    xTimerStart(math_timer, portMAX_DELAY); // 启动定时器

    /* 主循环 */
    while(1)
    {
        /* 扫描按键 */
        switch(key_get())
        {
        case 1: 
            key_1_call();
            break;
        case 2: 
            key_2_call();
            break;
        case 3: 
            key_3_call();
            break;
        default:
            break;
        }

        /* 释放CPU */
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}