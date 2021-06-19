
#ifndef _zf_spi_h
#define _zf_spi_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

void spi_init(void);
void lcd_cmd(const uint8_t cmd);
void lcd_data(const uint8_t data[2], int len);
uint32_t lcd_get_id(void);

#endif