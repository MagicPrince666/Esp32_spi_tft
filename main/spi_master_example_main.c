/* SPI Master example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "soc/gpio_struct.h"
#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "lcd.h"
#include "font.h"
#include "MFRC_Task.h"

#define ECHO_TEST_TXD1  (21)
#define ECHO_TEST_RXD1  (34)

#define ECHO_TEST_TXD2  (12)
#define ECHO_TEST_RXD2  (35)


#define BUF_SIZE (1024)

//touch
//#define PIN_NUM_TCS 0
//#define PIN_NUM_TIRQ 4


static void display_task()
{
    //int times = 0;
    Lcd_Init();   //tft初始化
    printf("lcd init OK!\n");

    BACK_COLOR = WHITE;
    POINT_COLOR = BLACK;
    while(1)
    {
        LCD_Clear(RED);
        LCD_ShowString(10,300,200,16,16,"  red color\n");
	 	LCD_Clear(GREEN);
        LCD_ShowString(10,300,200,16,16,"green color");
		LCD_Clear(BLUE);
        LCD_ShowString(10,300,200,16,16," blue color");
	 	LCD_Clear(WHITE);
        LCD_ShowString(10,300,200,16,16,"white color");
        vTaskDelay(500 / portTICK_RATE_MS);
        //printf("display continue\n");
    } 
}
//an example of echo test with hardware flow control on UART1
static void echo_task1()
{
    const int uart_num1 = UART_NUM_1;
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
    };
    //Configure UART1 parameters
    uart_param_config(uart_num1, &uart_config);
    //Set UART1 pins(TX: IO4, RX: I05, RTS: IO18, CTS: IO19)
    uart_set_pin(uart_num1, ECHO_TEST_TXD1, ECHO_TEST_RXD1,-1,-1);
    //Install UART driver (we don't need an event queue here)
    //In this example we don't even use a buffer for sending data.
    uart_driver_install(uart_num1, BUF_SIZE * 2, 0, 0, NULL, 0);

    uint8_t* data1 = (uint8_t*) malloc(BUF_SIZE);
    while(1) {
        //Read data from UART
        int len1 = uart_read_bytes(uart_num1, data1, BUF_SIZE, 20 / portTICK_RATE_MS);
        //Write data back to UART
        uart_write_bytes(uart_num1, (const char*) data1, len1);
    }
}

static void echo_task2()
{
    const int uart_num2 = UART_NUM_2;
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
    };
    //Configure UART2 parameters
    uart_param_config(uart_num2, &uart_config);
    //Set UART2 pins(TX: IO4, RX: I05, RTS: IO18, CTS: IO19)
    uart_set_pin(uart_num2, ECHO_TEST_TXD2, ECHO_TEST_RXD2,-1,-1);
    //Install UART driver (we don't need an event queue here)
    //In this example we don't even use a buffer for sending data.
    uart_driver_install(uart_num2, BUF_SIZE * 2, 0, 0, NULL, 0);

    uint8_t* data2 = (uint8_t*) malloc(BUF_SIZE);
    while(1) {
        //Read data from UART
        int len2 = uart_read_bytes(uart_num2, data2, BUF_SIZE, 20 / portTICK_RATE_MS);
        //Write data back to UART
        uart_write_bytes(uart_num2, (const char*) data2, len2);
    }
}
void app_main(void)
{   
    xTaskCreate(display_task, "lcd_display_task", 2048, NULL, 10, NULL);
    //xTaskCreate(echo_task1, "uart_echo_task1", 1024, NULL, 10, NULL);
    xTaskCreate(MFRC_main, "NFC_rc522_task", 1024, NULL, 10, NULL);
}
