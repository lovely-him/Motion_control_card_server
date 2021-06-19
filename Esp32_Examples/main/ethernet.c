/* Ethernet Basic Example
   以太网基本示例
*/

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

static const char *TAG = "eth_example";
static const char *HIM = "Lovely_him";

/* 创建信号量 */
static SemaphoreHandle_t xSemaphore = NULL;

/**以太网事件处理程序*/
static void eth_event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{
    uint8_t mac_addr[6] = {0};

    /*我们可以从事件数据中获得以太网驱动程序句柄*/
    esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

    switch (event_id) {
    case ETHERNET_EVENT_CONNECTED:
        /* Etherent驱动的Misc IO功能 */
        esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
        ESP_LOGI(TAG, "Ethernet Link Up"); // 以太网连接
        ESP_LOGI(TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x",
                 mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Ethernet Link Down"); // 以太网链接下
        break;
    case ETHERNET_EVENT_START:
        ESP_LOGI(TAG, "Ethernet Started"); // 以太网开始
        break;
    case ETHERNET_EVENT_STOP:
        ESP_LOGI(TAG, "Ethernet Stopped"); // 以太网停止
        break;
    default:
        break;
    }
}

/** IP_EVENT_ETH_GOT_IP的事件处理程序*/
static void got_ip_event_handler(void *arg, esp_event_base_t event_base,
                                 int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    const esp_netif_ip_info_t *ip_info = &event->ip_info;

    ESP_LOGI(TAG, "Ethernet Got IP Address"); // 以太网获取IP地址
    ESP_LOGI(TAG, "~~~~~~~~~~~");
    ESP_LOGI(TAG, "ETHIP:" IPSTR, IP2STR(&ip_info->ip));
    ESP_LOGI(TAG, "ETHMASK:" IPSTR, IP2STR(&ip_info->netmask));
    ESP_LOGI(TAG, "ETHGW:" IPSTR, IP2STR(&ip_info->gw));
    ESP_LOGI(TAG, "~~~~~~~~~~~");

    /* 释放信号量 */
    xSemaphoreGive( xSemaphore );
    ESP_LOGI(HIM, "Ethernet xSemaphoreGive --------------"); // 释放信号量
}

void ethernet_call(void)
{
   /* 创建信号量来保护共享资源。 */
    xSemaphore = xSemaphoreCreateBinary();
    ESP_LOGI(HIM, "Ethernet vSemaphoreCreateBinary --------------"); // 创建信号量

    if( xSemaphore == NULL )
    {
        // 没有成功创建了信号量。
        // 现在不可以使用信号量了。
        ESP_LOGI(HIM, "Ethernet vSemaphoreCreateBinary !!!!!!!! --------------");
    }

    /* 初始化TCP/IP网络接口(在应用程序中只能调用一次) */
    // ESP_ERROR_CHECK(esp_netif_init()); // wifi 中已经创建

    /* 创建在后台运行的默认事件循环 */
    // ESP_ERROR_CHECK(esp_event_loop_create_default()); // wifi 中已经创建

    /* 设置默认处理程序来处理TCP/IP内容 */ 
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
    esp_netif_t *eth_netif = esp_netif_new(&cfg);
    ESP_ERROR_CHECK(esp_eth_set_default_handlers(eth_netif)); // wifi 中没有创建，不是这个

    /* 注册用户定义的事件处理程序 */
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event_handler, NULL));

    /* 创建一个PHY实例LAN8720 */
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = CONFIG_EXAMPLE_ETH_PHY_ADDR;
    phy_config.reset_gpio_num = CONFIG_EXAMPLE_ETH_PHY_RST_GPIO;
    esp_eth_phy_t *phy = esp_eth_phy_new_lan8720(&phy_config);

    /* 创建ESP32以太网MAC实例 */
    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    mac_config.smi_mdc_gpio_num = CONFIG_EXAMPLE_ETH_MDC_GPIO;
    mac_config.smi_mdio_gpio_num = CONFIG_EXAMPLE_ETH_MDIO_GPIO;
    esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&mac_config);

    /* 以太网驱动程序安装 */
    esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
    esp_eth_handle_t eth_handle = NULL;
    ESP_ERROR_CHECK(esp_eth_driver_install(&config, &eth_handle));

    /* 连接TCP/IP协议栈 */
    ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));
    
    /*启动以太网驱动程序状态机*/
    ESP_ERROR_CHECK(esp_eth_start(eth_handle));
    
    /* 开始阻塞获取信号量 */
    ESP_LOGI(HIM, "Ethernet xSemaphoreTake start --------------"); // 创建信号量
    if( xSemaphoreTake( xSemaphore, portMAX_DELAY ) )
    {
        ESP_LOGI(HIM, "Ethernet xSemaphoreTake end --------------"); // 创建信号量
        /* 删除信号量 */
        vSemaphoreDelete(xSemaphore);
    }
}
