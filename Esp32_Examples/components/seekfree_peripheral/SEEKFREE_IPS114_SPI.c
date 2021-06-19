/*********************************************************************************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2018,逐飞科技
 * All rights reserved.
 * 技术讨论QQ群：一群：179029047(已满)  二群：244861897
 *
 * 以下所有内容版权均属逐飞科技所有，未经允许不得用于商业用途，
 * 欢迎各位使用并传播本程序，修改内容时必须保留逐飞科技的版权声明。
 *
 * @file               IPS液晶
 * @company               成都逐飞科技有限公司
 * @author             逐飞科技(QQ3184284598)
 * @version            查看doc内version文件 版本说明
 * @Software         IAR 8.3 or MDK 5.28
 * @Taobao           https://seekfree.taobao.com/
 * @date               2019-03-06
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

#include "SEEKFREE_IPS114_SPI.h"

     
// #define PIN_NUM_DC   21
// #define PIN_NUM_RST  18
// #define PIN_NUM_BCKL 5

// #define IPS114_BL_PIN           21	            //液晶背光引脚定义  
#define IPS114_REST_PIN         14              //液晶复位引脚定义 由于实际通讯未使用B2因此 这里复用为GPIO
// #define IPS114_DC_PIN 	        21	            //液晶命令位引脚定义

// #define IPS114_DC(x)            gpio_set_level(IPS114_DC_PIN,x)
#define IPS114_REST(x)          gpio_set_level(IPS114_REST_PIN,x)

//-------------------------------------------------------------------------------------------------------------------
//  @brief      内部调用，用户无需关心
//  @param      void             
//  @return                     
//  @since      v1.0
//  Sample usage:               
//-------------------------------------------------------------------------------------------------------------------
void ips114_writeIndex(uint8_t cmd)            //写命令
{    
    // IPS114_DC(0);
    lcd_cmd(cmd);
    // LCD_WR_REG(cmd);
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      内部调用，用户无需关心
//  @param      void             
//  @return                     
//  @since      v1.0
//  Sample usage:               
//-------------------------------------------------------------------------------------------------------------------
void ips114_writeData(uint8_t data)            //写数据
{
    // IPS114_DC(1);
    uint8_t ch[2] = {0,0};
    ch[0] = data;
    lcd_data(ch,1);
    // LCD_WR_DATA(data);
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      内部调用，用户无需关心
//  @param      void             
//  @return                     
//  @since      v1.0
//  Sample usage:               
//-------------------------------------------------------------------------------------------------------------------
void ips114_writedata_16bit(uint16_t data)    //向液晶屏写一个16位数据
{    
    // IPS114_DC(1);
    uint8_t ch[2] = {0,0};
    ch[0] = data>>8;
    ch[1] = (uint8_t)data;
    lcd_data(ch,2);
    // Lcd_WriteData_16Bit(data);
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      1.14寸 IPS液晶初始化
//  @return     void
//  @since      v1.0
//  Sample usage:               
//-------------------------------------------------------------------------------------------------------------------
void ips114_init(void)
{
    spi_init();//硬件SPI初始化
    // LCD_GPIOInit();//软件spi初始化

    // gpio_set_direction(IPS114_BL_PIN, GPIO_MODE_OUTPUT);        // 背光源引脚，初始化拉高后就没管过了，
    // gpio_set_level(IPS114_BL_PIN, 1);                           // 如果不用的话其实直接接电源，然后把这段代码屏蔽。

    gpio_set_direction(IPS114_REST_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(IPS114_REST_PIN, 0);

    IPS114_REST(0);
    vTaskDelay(200 / portTICK_RATE_MS);
    
    IPS114_REST(1);
    vTaskDelay(100 / portTICK_RATE_MS);
    
    ips114_writeIndex(0x36);
    
    if      (IPS114_DISPLAY_DIR==0)    ips114_writeData(0x00);
    else if (IPS114_DISPLAY_DIR==1)    ips114_writeData(0xC0);
    else if (IPS114_DISPLAY_DIR==2)    ips114_writeData(0x70);
    else                            ips114_writeData(0xA0);
    
    ips114_writeIndex(0x3A); 
    ips114_writeData(0x05);

    ips114_writeIndex(0xB2);
    ips114_writeData(0x0C);
    ips114_writeData(0x0C);
    ips114_writeData(0x00);
    ips114_writeData(0x33);
    ips114_writeData(0x33); 

    ips114_writeIndex(0xB7); 
    ips114_writeData(0x35);  

    ips114_writeIndex(0xBB);
    ips114_writeData(0x37);

    ips114_writeIndex(0xC0);
    ips114_writeData(0x2C);

    ips114_writeIndex(0xC2);
    ips114_writeData(0x01);

    ips114_writeIndex(0xC3);
    ips114_writeData(0x12);   

    ips114_writeIndex(0xC4);
    ips114_writeData(0x20);  

    ips114_writeIndex(0xC6); 
    ips114_writeData(0x0F);    

    ips114_writeIndex(0xD0); 
    ips114_writeData(0xA4);
    ips114_writeData(0xA1);

    ips114_writeIndex(0xE0);
    ips114_writeData(0xD0);
    ips114_writeData(0x04);
    ips114_writeData(0x0D);
    ips114_writeData(0x11);
    ips114_writeData(0x13);
    ips114_writeData(0x2B);
    ips114_writeData(0x3F);
    ips114_writeData(0x54);
    ips114_writeData(0x4C);
    ips114_writeData(0x18);
    ips114_writeData(0x0D);
    ips114_writeData(0x0B);
    ips114_writeData(0x1F);
    ips114_writeData(0x23);

    ips114_writeIndex(0xE1);
    ips114_writeData(0xD0);
    ips114_writeData(0x04);
    ips114_writeData(0x0C);
    ips114_writeData(0x11);
    ips114_writeData(0x13);
    ips114_writeData(0x2C);
    ips114_writeData(0x3F);
    ips114_writeData(0x44);
    ips114_writeData(0x51);
    ips114_writeData(0x2F);
    ips114_writeData(0x1F);
    ips114_writeData(0x1F);
    ips114_writeData(0x20);
    ips114_writeData(0x23);

    ips114_writeIndex(0x21); 

    ips114_writeIndex(0x11); 
    vTaskDelay(120 / portTICK_RATE_MS);

    ips114_writeIndex(0x29);
    ips114_clear(IPS114_BGCOLOR);
}



void ips114_set_region(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2)
{
    if(IPS114_DISPLAY_DIR==0)
    {
        ips114_writeIndex(0x2a);//列地址设置
        ips114_writedata_16bit(x1+52);
        ips114_writedata_16bit(x2+52);
        ips114_writeIndex(0x2b);//行地址设置
        ips114_writedata_16bit(y1+40);
        ips114_writedata_16bit(y2+40);
        ips114_writeIndex(0x2c);//储存器写
    }
    else if(IPS114_DISPLAY_DIR==1)
    {
        ips114_writeIndex(0x2a);//列地址设置
        ips114_writedata_16bit(x1+53);
        ips114_writedata_16bit(x2+53);
        ips114_writeIndex(0x2b);//行地址设置
        ips114_writedata_16bit(y1+40);
        ips114_writedata_16bit(y2+40);
        ips114_writeIndex(0x2c);//储存器写
    }
    else if(IPS114_DISPLAY_DIR==2)
    {
        ips114_writeIndex(0x2a);//列地址设置
        ips114_writedata_16bit(x1+40);
        ips114_writedata_16bit(x2+40);
        ips114_writeIndex(0x2b);//行地址设置
        ips114_writedata_16bit(y1+53);
        ips114_writedata_16bit(y2+53);
        ips114_writeIndex(0x2c);//储存器写
    }
    else
    {
        ips114_writeIndex(0x2a);//列地址设置
        ips114_writedata_16bit(x1+40);
        ips114_writedata_16bit(x2+40);
        ips114_writeIndex(0x2b);//行地址设置
        ips114_writedata_16bit(y1+52);
        ips114_writedata_16bit(y2+52);
        ips114_writeIndex(0x2c);//储存器写
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      液晶清屏函数
//  @param      color             颜色设置
//  @return     void
//  @since      v1.0
//  Sample usage:               ips114_clear(YELLOW);// 全屏设置为黄色
//-------------------------------------------------------------------------------------------------------------------
void ips114_clear(uint16_t color)
{
    uint16_t i,j;
    ips114_set_region(0,0,IPS114_X_MAX-1,IPS114_Y_MAX-1);
    for(i=0;i<IPS114_X_MAX;i++)
    {
        for (j=0;j<IPS114_Y_MAX;j++)
           {
            ips114_writedata_16bit(color);                  
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      液晶画点
//  @param      x                 坐标x方向的起点
//  @param      y                 坐标y方向的起点
//  @param      dat               需要显示的颜色
//  @return     void
//  @since      v1.0
//  Sample usage:               ips114_drawpoint(0,0,RED);  //坐标0,0画一个红色的点
//-------------------------------------------------------------------------------------------------------------------
void ips114_drawpoint(uint16_t x,uint16_t y,uint16_t color)
{
    ips114_set_region(x,y,x,y);
    ips114_writedata_16bit(color);
}



//-------------------------------------------------------------------------------------------------------------------
//  @brief      液晶显示字符
//  @param      x                 坐标x方向的起点 参数范围 0 -（IPS114_X_MAX-1）
//  @param      y                 坐标y方向的起点 参数范围 0 -（IPS114_Y_MAX/16-1）
//  @param      dat               需要显示的字符
//  @return     void
//  @since      v1.0
//  Sample usage:               ips114_showchar(0,0,'x');//坐标0,0写一个字符x
//-------------------------------------------------------------------------------------------------------------------
void ips114_showchar(uint16_t x,uint16_t y,const int8_t dat)
{
    uint8_t i,j;
    uint8_t temp;
    
    for(i=0; i<16; i++)
    {
        ips114_set_region(x,y+i,x+7,y+i);
        temp = tft_ascii[dat-32][i];//减32因为是取模是从空格开始取得 空格在ascii中序号是32
        for(j=0; j<8; j++)
        {
            if(temp&0x01)    
            {
                ips114_writedata_16bit(IPS114_PENCOLOR);
            }
            else            ips114_writedata_16bit(IPS114_BGCOLOR);
            temp>>=1;
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      液晶显示字符串
//  @param      x                 坐标x方向的起点  参数范围 0 -（IPS114_X_MAX-1）
//  @param      y                 坐标y方向的起点  参数范围 0 -（IPS114_Y_MAX/16-1）
//  @param      dat               需要显示的字符串
//  @return     void
//  @since      v1.0
//  Sample usage:               ips114_showstr(0,0,"seekfree");
//-------------------------------------------------------------------------------------------------------------------
void ips114_showstr(uint16_t x,uint16_t y,const int8_t dat[])
{
    uint16_t j;
    
    j = 0;
    while(dat[j] != '\0')
    {
        ips114_showchar(x+8*j,y*16,dat[j]);
        j++;
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      液晶显示8位有符号
//  @param      x                 坐标x方向的起点  参数范围 0 -（IPS114_X_MAX-1）
//  @param      y                 坐标y方向的起点  参数范围 0 -（IPS114_Y_MAX/16-1）
//  @param      dat               需要显示的变量，数据类型int8
//  @return     void
//  @since      v1.0
//  Sample usage:               ips114_showint8(0,0,x);//x为int8类型
//-------------------------------------------------------------------------------------------------------------------
void ips114_showint8(uint16_t x,uint16_t y,int8_t dat)
{
    uint8_t a[3];
    uint8_t i;
    if(dat<0)
    {
        ips114_showchar(x,y*16,'-');
        dat = -dat;
    }
    else    ips114_showchar(x,y*16,' ');
    
    a[0] = dat/100;
    a[1] = dat/10%10;
    a[2] = dat%10;
    i = 0;
    while(i<3)
    {
        ips114_showchar(x+(8*(i+1)),y*16,'0' + a[i]);
        i++;
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      液晶显示8位无符号
//  @param      x                 坐标x方向的起点 参数范围 0 -（IPS114_X_MAX-1）
//  @param      y                 坐标y方向的起点 参数范围 0 -（IPS114_Y_MAX/16-1）
//  @param      dat               需要显示的变量，数据类型uint8
//  @return     void
//  @since      v1.0
//  Sample usage:               ips114_showuint8(0,0,x);//x为uint8类型
//-------------------------------------------------------------------------------------------------------------------
void ips114_showuint8(uint16_t x,uint16_t y,uint8_t dat)
{
    uint8_t a[3];
    uint8_t i;
    
    a[0] = dat/100;
    a[1] = dat/10%10;
    a[2] = dat%10;
    i = 0;
    while(i<3)
    {
        ips114_showchar(x+(8*i),y*16,'0' + a[i]);
        i++;
    }
    
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      液晶显示16位有符号
//  @param      x                 坐标x方向的起点 参数范围 0 -（IPS114_X_MAX-1）
//  @param      y                 坐标y方向的起点 参数范围 0 -（IPS114_Y_MAX/16-1）
//  @param      dat               需要显示的变量，数据类型int16
//  @return     void
//  @since      v1.0
//  Sample usage:               ips114_showint16(0,0,x);//x为int16类型
//-------------------------------------------------------------------------------------------------------------------
void ips114_showint16(uint16_t x,uint16_t y,int16_t dat)
{
    uint8_t a[5];
    uint8_t i;
    if(dat<0)
    {
        ips114_showchar(x,y*16,'-');
        dat = -dat;
    }
    else    ips114_showchar(x,y*16,' ');

    a[0] = dat/10000;
    a[1] = dat/1000%10;
    a[2] = dat/100%10;
    a[3] = dat/10%10;
    a[4] = dat%10;
    
    i = 0;
    while(i<5)
    {
        ips114_showchar(x+(8*(i+1)),y*16,'0' + a[i]);
        i++;
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      液晶显示16位无符号
//  @param      x                 坐标x方向的起点 参数范围 0 -（IPS114_X_MAX-1）
//  @param      y                 坐标y方向的起点 参数范围 0 -（IPS114_Y_MAX/16-1）
//  @param      dat               需要显示的变量，数据类型uint16
//  @return     void
//  @since      v1.0
//  Sample usage:               ips114_showuint16(0,0,x);//x为uint16类型
//-------------------------------------------------------------------------------------------------------------------
void ips114_showuint16(uint16_t x,uint16_t y,uint16_t dat)
{
    uint8_t a[5];
    uint8_t i;

    a[0] = dat/10000;
    a[1] = dat/1000%10;
    a[2] = dat/100%10;
    a[3] = dat/10%10;
    a[4] = dat%10;
    
    i = 0;
    while(i<5)
    {
        ips114_showchar(x+(8*i),y*16,'0' + a[i]);
        i++;
    }
}
