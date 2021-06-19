/* ESP HTTP Client Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


#include <stdio.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
// #include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_websocket_client.h"
#include "esp_event.h"

#include "him_all.h"

#define NO_DATA_TIMEOUT_SEC 10

static const char *TAG = "WEBSOCKET";
static const char *HIM = "Lovely_him";

static TimerHandle_t shutdown_signal_timer;
esp_websocket_client_handle_t client;

#define HIM_DEBUG_KEY  0

/* 关闭连接 */
void websocket_app_stop(void)
{
    /*
    停止WebSocket连接而没有WebSocket关闭握手。
    此API停止ws客户端并直接关闭TCP连接，而不发送关闭帧。
    使用 esp_websocket_client_close() 以一种干净的方式关闭连接是一个很好的实践。
    */
    esp_websocket_client_stop(client);
    ESP_LOGI(TAG, "Websocket Stopped");
    ESP_LOGI(HIM, "esp_websocket_client_stop --------------");
    /*
    销毁 WebSocket 连接并释放所有资源。
    这个函数必须是会话调用的最后一个函数。
    它与 esp_websocket_client_init 函数相反，
    调用时必须使用与 esp_websocket_client_init 调用返回的输入相同的句柄。
    这可能会关闭该句柄使用过的所有连接。
    */
    esp_websocket_client_destroy(client);
}

/* 终端/串口获取IP */
#if CONFIG_WEBSOCKET_URI_FROM_STDIN
static void get_string(char *line, size_t size)
{
    int count = 0;
    while (count < size) {
        int c = fgetc(stdin);
        if (c == '\n') {
            line[count] = '\0';
            break;
        } else if (c > 0 && c < 127) {
            line[count] = c;
            ++count;
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
#endif /* CONFIG_WEBSOCKET_URI_FROM_STDIN */

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
        break;
    case WEBSOCKET_EVENT_DATA:
#if HIM_DEBUG_KEY
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DATA");
        ESP_LOGI(TAG, "Received opcode=%d", data->op_code);
        ESP_LOGW(TAG, "Received=%.*s", data->data_len, (char *)data->data_ptr);
        ESP_LOGW(TAG, "Total payload length=%d, data_len=%d, current payload offset=%d\r\n", data->payload_len, data->data_len, data->payload_offset);
#else
        ESP_LOGW(TAG, "Received=%.*s", data->data_len, (char *)data->data_ptr);
#endif
        break;
    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
        break;
    }
}

/* 定时器中断任务 */
int error_i = 0;
static void Timer_Handle(TimerHandle_t xTimer)
{
    char data[64];
    // 检查WebSocket客户端连接状态。
    if (esp_websocket_client_is_connected(client)) 
    {
        int len = sprintf(data, "time = %d s, M.flag = %04d e", xTaskGetTickCount()/100, M.flag);
        // ESP_LOGI(TAG, "%s ", data); // 调试打印
        // 将文本数据写入WebSocket连接
        esp_websocket_client_send_text(client, data, len, portMAX_DELAY);
        led_off(); // 关灯
    }
    else
    {
        error_i++;
        sprintf(data, "hello %04d", error_i++);
        ESP_LOGI(HIM, "websocket error %s -----------------", data);
        led_on(); // 开灯
    }
}

void websocket_app_start(void)
{
    // 创建一个新的软件计时器实例，并返回一个句柄，通过这个句柄可以引用创建的软件计时器。
    shutdown_signal_timer = xTimerCreate("Websocket shutdown timer",                        // 只是一个文本名称，不被内核使用。
                                         1 * 1000 / portTICK_PERIOD_MS,                     // 计时器周期(单位是tick)。
                                         pdTRUE,                                            // 计时器将在到期时自动重新加载。（会）
                                         NULL,                                              // 为每个计时器分配一个唯一的id等于它的数组索引。
                                         Timer_Handle);                                     // 每个计时器在到期时调用同一个回调。


    esp_websocket_client_config_t websocket_cfg = {};
    // 是否需要手动输入uri地址，若配置中不存在则需要
#if CONFIG_WEBSOCKET_URI_FROM_STDIN
    char line[128];

    ESP_LOGI(TAG, "Please enter uri of websocket endpoint");
    get_string(line, sizeof(line));

    websocket_cfg.uri = line;
    ESP_LOGI(TAG, "Endpoint uri: %s\n", line);
#else
    // 直接获取uri地址
    websocket_cfg.uri = CONFIG_WEBSOCKET_URI;
#endif /* CONFIG_WEBSOCKET_URI_FROM_STDIN */

    ESP_LOGI(TAG, "Connecting to %s...", websocket_cfg.uri);

    /*
    这个函数必须是第一个调用的函数，它返回一个 esp_websocket_client_handle_t ，
    你必须把它作为接口中其他函数的输入。
    当操作完成时，这个调用必须有一个对应的 esp_websocket_client_destroy 调用。
    */
    client = esp_websocket_client_init(&websocket_cfg);
    // 注册Websocket事件。
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);
    // 打开WebSocket连接。
    esp_websocket_client_start(client);
    ESP_LOGI(HIM, "esp_websocket_client_start --------------");
    
    // 启动定时器
    xTimerStart(shutdown_signal_timer, portMAX_DELAY);

    /* 关闭服务 websocket */
    // websocket_app_stop();
}

