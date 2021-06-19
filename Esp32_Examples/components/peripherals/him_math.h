#ifndef  _HIM_MATH_H_
#define  _HIM_MATH_H_

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include <math.h>

#include "him_all.h"

typedef struct _Motor_s
{
	int f; 			// 电机状态：运行，停止 / 期望改变，没有改变
	float w; 		// 角速度：即每秒圈数
	float c; 		// 距离/坐标：即累计圈数/累计脉冲
	float t;		// 时间：即期望位置 / 设定期望后的移动位置
} Motor_s;

typedef struct _Math_s
{
	int flag; 			//  标志位
	int test_num[5];	// 测试位
	int mods;			// 模式

	int mods_cf;    	// 模式c的标志位

	Motor_s expect[2]; 	// 期望电机状态
	Motor_s actual[2]; 	// 实际电机状态
} Math_s;               // 所有变量结构体
extern Math_s M;

extern void math_init(void);
extern void age_init(void);

#endif /*  _HIM_MATH_H_ */