/*********************************************************************************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2018,逐飞科技
 * All rights reserved.
 * 技术讨论QQ群：一群：179029047(已满)  二群：244861897
 *
 * 以下所有内容版权均属逐飞科技所有，未经允许不得用于商业用途，
 * 欢迎各位使用并传播本程序，修改内容时必须保留逐飞科技的版权声明。
 *
 * @file       		IPS液晶
 * @company	   		成都逐飞科技有限公司
 * @author     		逐飞科技(QQ3184284598)
 * @version    		查看doc内version文件 版本说明
 * @Software 		IAR 8.3 or MDK 5.28
 * @Taobao   		https://seekfree.taobao.com/
 * @date       		2019-03-06
 * @note		
					接线定义：
					------------------------------------ 
						模块管脚            单片机管脚
                        SCL                 查看IPS114_SCL_PIN宏定义的引脚     硬件SPI引脚不可随意切换
						SDA                 查看IPS114_SDA_PIN宏定义的引脚     硬件SPI引脚不可随意切换
						RES                 查看IPS114_REST_PIN宏定义的引脚    
						DC                  查看IPS114_DC_PIN宏定义的引脚  
						CS                  查看IPS114_CS_PIN宏定义的引脚      硬件SPI引脚不可随意切换
						
						电源引脚
						BL  3.3V电源（背光控制引脚，也可以接PWM来控制亮度）
						VCC 3.3V电源
						GND 电源地
						最大分辨率135*240
					------------------------------------ 

 ********************************************************************************************************************/


#ifndef _SEEKFREE_IPS114_H
#define _SEEKFREE_IPS114_H


#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "SEEKFREE_FONT.h"
#include "esp32_spi.h"
// #include "TFT_SPI.h"




//-------常用颜色在SEEKFREE_FONT.h文件中定义----------
//#define RED          	    0xF800	//红色
//#define BLUE         	    0x001F  //蓝色
//#define YELLOW       	    0xFFE0	//黄色
//#define GREEN        	    0x07E0	//绿色
//#define WHITE        	    0xFFFF	//白色
//#define BLACK        	    0x0000	//黑色 
//#define GRAY  			0X8430 	//灰色
//#define BROWN 			0XBC40 	//棕色
//#define PURPLE    		0XF81F	//紫色
//#define PINK    		    0XFE19	//粉色


//定义写字笔的颜色
#define IPS114_PENCOLOR    RED

//定义背景颜色
#define IPS114_BGCOLOR     BROWN



#define IPS114_W   135
#define IPS114_H   240

//定义显示方向
//0 竖屏模式
//1 竖屏模式  旋转180
//2 横屏模式
//3 横屏模式  旋转180
#define IPS114_DISPLAY_DIR 2

#if (0==IPS114_DISPLAY_DIR || 1==IPS114_DISPLAY_DIR)
#define	IPS114_X_MAX	IPS114_W	//液晶X方宽度
#define IPS114_Y_MAX	IPS114_H    //液晶Y方宽度
     
#elif (2==IPS114_DISPLAY_DIR || 3==IPS114_DISPLAY_DIR)
#define	IPS114_X_MAX	IPS114_H	//液晶X方宽度
#define IPS114_Y_MAX	IPS114_W    //液晶Y方宽度
     
#else
#error "IPS114_DISPLAY_DIR 定义错误"
     
#endif


void ips114_init(void);
void ips114_clear(uint16_t color);
void ips114_drawpoint(uint16_t x,uint16_t y,uint16_t color);
void ips114_showchar(uint16_t x,uint16_t y,const int8_t dat);
void ips114_showstr(uint16_t x,uint16_t y,const int8_t dat[]);
void ips114_showint8(uint16_t x,uint16_t y,int8_t dat);
void ips114_showuint8(uint16_t x,uint16_t y,uint8_t dat);
void ips114_showint16(uint16_t x,uint16_t y,int16_t dat);
void ips114_showuint16(uint16_t x,uint16_t y,uint16_t dat);
#endif
