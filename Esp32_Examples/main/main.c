
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

#include "SEEKFREE_all.h"
#include "him_all.h"

extern int station_call(void);
extern void ethernet_call(void);
extern void websocket_app_start(void);

void app_main(void)
{
    /* 初始化参数 */
    age_init();

    /* 初始化外设 */
    led_init();         // 初始化提示灯，websocket服务中用到
    led_on();           // 开灯，灭灯时表示网络通讯正常

    /* 返回0表示成功，返回1表示不成功 */
    if (station_call())
    {
        /* 以太网创建一定是成功了，不然不会退出 */
        ethernet_call();      
    }

    /* 初始化web服务 */
    websocket_app_start();

    /* 初始化/启动 运动控制卡 功能 */
    math_init();
}
