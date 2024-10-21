/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#include <string.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "i2c_eeprom.h"

#define SCL_IO_PIN 19           // GPIO 19
#define SDA_IO_PIN 18           // GPIO 18
#define MASTER_FREQUENCY 400000 // 400K
#define PORT_NUMBER -1
#define LENGTH 16 // 单页写入最大16个字节

static void disp_buf(uint8_t *buf, int len) // 把每一页内容打印出来
{
    int i;
    for (i = 0; i < len; i++)
    {
        printf("0x%02X ", buf[i]);
        if ((i + 1) % 16 == 0)
        {
            printf("\n");
        }
    }
    printf("\n");
}

void app_main(void)
{
    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = PORT_NUMBER,
        .scl_io_num = SCL_IO_PIN,
        .sda_io_num = SDA_IO_PIN,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus_handle;

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &bus_handle));

    uint32_t block_addr = 0x0011; // block_addr:写入到eeprom的首地址,这个地址必需是能被16整除

    i2c_eeprom_config_t eeprom_config = {
        .eeprom_device.scl_speed_hz = MASTER_FREQUENCY, // 400KHz
        .eeprom_device.device_address = 0x50 + (block_addr >> 8),
        .addr_wordlen = 2,
        .write_time_ms = 10,
    };

    i2c_eeprom_handle_t eeprom_handle;

    uint8_t buf[LENGTH];

    // for (int i = 0; i < LENGTH; i++) {   //赋值
    //     buf[i] = i;
    // }
    memset(buf, 0, LENGTH);
    buf[0] = 2;
    buf[1] = 3;
    buf[2] = 5;
    buf[3] = 0x07;
    buf[6] = 0x07;
    buf[7] = 0x0A;
    buf[15] = 0x77;

    uint8_t read_buf[LENGTH];
    ESP_ERROR_CHECK(i2c_eeprom_init(bus_handle, &eeprom_config, &eeprom_handle));

    while (1)
    {
        ESP_ERROR_CHECK(i2c_eeprom_write(eeprom_handle, block_addr, buf, LENGTH));
        // Needs wait for eeprom hardware done, referring from datasheet
        i2c_eeprom_wait_idle(eeprom_handle);
        ESP_ERROR_CHECK(i2c_eeprom_read(eeprom_handle, block_addr, read_buf, LENGTH));
        disp_buf(read_buf, LENGTH);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
