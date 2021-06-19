
#include "esp32_spi.h"

// 实测引脚并不是固定的，全都是随便设置

//-----------------引脚定义------------------------------
#define IPS114_SPIN_PIN         HSPI_HOST           //定义使用的SPI号(esp32 只有hspi或是vspi？！)
#define IPS114_SCL_PIN          34    //定义SPI_SCK引脚
#define IPS114_SDA_PIN          35   //定义SPI_MOSI引脚
#define IPS114_SDA_IN_PIN       -1   //定义SPI_MISO引脚  IPS没有MISO引脚，但是这里任然需要定义，在spi的初始化时需要使用（实测如果接到这个引脚的话屏幕是不能运作的）用不到就填-1
#define IPS114_CS_PIN           33    //定义SPI_CS引脚
#define IPS114_DC_PIN 	        32	            //液晶命令位引脚定义


spi_device_handle_t ips114_spi;

uint32_t lcd_get_id(void)
{
    //get_id cmd
    lcd_cmd(0x04);

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length=8*3;
    t.flags = SPI_TRANS_USE_RXDATA;
    t.user = (void*)1;
    esp_err_t ret = spi_device_polling_transmit(ips114_spi, &t);
    assert( ret == ESP_OK );

    return *(uint32_t*)t.rx_data;
}

/* Send a command to the LCD. Uses spi_device_polling_transmit, which waits
 * until the transfer is complete.
 *
 * Since command transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void lcd_cmd(const uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.flags=SPI_TRANS_USE_TXDATA;
    t.length=8;                     //Command is 8 bits
    t.tx_data[0]=cmd;               //The data is the cmd itself
    t.user=(void*)0;                //D/C needs to be set to 0
    ret=spi_device_polling_transmit(ips114_spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

/* Send data to the LCD. Uses spi_device_polling_transmit, which waits until the
 * transfer is complete.
 *
 * Since data transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void lcd_data(const uint8_t data[2], int len)
{
    esp_err_t ret;
    spi_transaction_t t;
    if (len==0) return;             //no need to send anything
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.user=(void*)1;                //D/C needs to be set to 1
    t.flags=SPI_TRANS_USE_TXDATA;
    if(len == 1)
    {
        t.length=8;
        t.tx_data[0]=data[0];        //Data
    }
    else if(len == 2)
    {
        t.length=2*8;
        t.tx_data[0]=data[0];
        t.tx_data[1]=data[1];
    }
    else
    {
        assert(1);
    }
    ret=spi_device_polling_transmit(ips114_spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

//This function is called (in irq context!) just before a transmission starts. It will
//set the D/C line to the value indicated in the user field.
// 用来触发回调函数，然后在回调函数中拉高电平
void lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc=(int)t->user;
    gpio_set_level(IPS114_DC_PIN, dc);
    // printf("dc\n");
}

void spi_init(void)
{
    gpio_set_direction(IPS114_DC_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(IPS114_DC_PIN, 0);

    esp_err_t ret;
    spi_bus_config_t buscfg={
        .miso_io_num=IPS114_SDA_IN_PIN,         // 根据例程里的写法，不算使用 位掩码 ，直接使用序号就可以了
        .mosi_io_num=IPS114_SDA_PIN,
        .sclk_io_num=IPS114_SCL_PIN,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
    };
    spi_device_interface_config_t devcfg={
        // .command_bits = 8,
        // .address_bits = 24,
        .clock_speed_hz=10*1000*1000,           //Clock out at 26 MHz (最大26) 10Mhz 频率下可以用逐飞的屏幕
        .mode=0,                                //SPI mode 0 SPI模式 0：CPOL=0 CPHA=0    1：CPOL=0 CPHA=1   2：CPOL=1 CPHA=0   3：CPOL=1 CPHA=1 //具体细节可自行百度
        .spics_io_num=IPS114_CS_PIN,            //CS pin
        .queue_size=7,                          //We want to be able to queue 7 transactions at a time
        .pre_cb=lcd_spi_pre_transfer_callback,  //Specify pre-transfer callback to handle D/C line
    };
    //Initialize the SPI bus 初始化SPI总线
    ret=spi_bus_initialize(IPS114_SPIN_PIN, &buscfg, 0);
    ESP_ERROR_CHECK(ret);
    //Attach the LCD to the SPI bus 将LCD连接到SPI总线
    ret=spi_bus_add_device(IPS114_SPIN_PIN, &devcfg, &ips114_spi);
    ESP_ERROR_CHECK(ret);
}