/* WiFi station Example
   .无线电台的例子
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

static const char *TAG = "wifi station";
static const char *HIM = "Lovely_him";

/* .示例使用WiFi配置，您可以通过项目配置菜单设置
   .如果您不愿意，只需用您想要的配置将下面的条目更改为字符串
   . - 比如 #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY

/* FreeRTOS事件组在连接时发出信号*/
static EventGroupHandle_t s_wifi_event_group;

/* .事件组允许每个事件有多个位，但我们只关心两个事件:
 * - 我们用IP连接到AP
 * - 在达到最大重试次数后，我们连接失败 */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static int s_retry_num = 0;

/* 事件处理函数 */
static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        esp_wifi_connect(); // 将ESP32 WiFi站连接到AP。
        ESP_LOGI(HIM, "WIFI_EVENT_STA_START --------------");
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) 
        {
            esp_wifi_connect();
            s_retry_num++; // 每次连接不成功就累加
            ESP_LOGI(TAG, "retry to connect to the AP"); // 重试连接到AP
        } 
        else 
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT); // 如果累加错过次数就设置标志位
        }
        ESP_LOGI(TAG,"connect to the AP fail");         // 连接AP失败
        ESP_LOGI(HIM,"WIFI_EVENT_STA_DISCONNECTED --------------");
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        s_retry_num = 0; // 如果成功连接过一次就清零
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip)); // 得到联网IP
        ESP_LOGI(HIM,"IP_EVENT_STA_GOT_IP --------------");
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

int wifi_init_sta(void)
{
    /* 创建默认的WIFI STA。在任何初始化错误的情况下，这个API将中止。 */
    esp_netif_create_default_wifi_sta();

    /* Init WiFi为WiFi驱动分配资源，如WiFi控制结构、RX/TX缓冲区、WiFi NVS结构等，该WiFi也启动WiFi任务。 */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* 向默认循环注册一个事件处理程序实例 */
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id)); // 在达到最大重试次数后，我们连接失败
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip)); // 我们用IP连接到AP

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            /* 设置密码意味着工作站将连接到所有安全模式，包括WEP/WPA。
             * 然而，这些模式是不赞成的，不建议使用。如果您的访问点不支持WPA2，这些模式可以通过下面的注释启用 */
	        .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    /* 设置WiFi工作模式 */
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    /* 配置ESP32 STA / AP的配置信息 */
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    /* 根据当前配置启动WiFi */
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished."); // wifi 初始化 sta 模式 结束

    /* 块等待一个或多个位在先前创建的事件组中被设置。 */
    /* 等待连接建立 (WIFI_CONNECTED_BIT) 或连接失败重试次数 (WIFI_FAIL_BIT) 。位由 event_handler() 设置(见上面) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() 返回调用返回前的位，因此我们可以测试实际发生了哪个事件。 */
    if (bits & WIFI_CONNECTED_BIT) 
    {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
        
        /* 返回0，表示wifi连接成功 */
        return 0;
    } 
    else if (bits & WIFI_FAIL_BIT) 
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } 
    else 
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
    
    /* 取消注册后事件将不会被处理 */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));

    /* 删除以前通过调用xEventGroupCreate()创建的事件组。 */
    vEventGroupDelete(s_wifi_event_group);

    /* 返回1，表示wifi连接不成功 */
    ESP_LOGI(HIM, "Wifi STA ON! --------------");  
    return 1;
}

/* 外部调用函数 */
int station_call(void)
{
    /* 初始化默认NVS分区 */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || 
    ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    /* 创建一个新的事件组 */
    s_wifi_event_group = xEventGroupCreate();

    /* 初始化底层TCP/IP堆栈 */
    ESP_ERROR_CHECK(esp_netif_init());

    /* 创建默认事件循环 */
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");

    /* 返回0表示成功，返回1表示不成功 */
    return wifi_init_sta();
}


