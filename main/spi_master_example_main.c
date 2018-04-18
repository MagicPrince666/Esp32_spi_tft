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
#include "touch.h" 
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#define ECHO_TEST_TXD1  (13)
#define ECHO_TEST_RXD1  (34)

#define ECHO_TEST_TXD2  (12)
#define ECHO_TEST_RXD2  (35)


#define BUF_SIZE (1024)

//touch
//#define PIN_NUM_TCS 0
//#define PIN_NUM_TIRQ 4
/* The examples use simple WiFi configuration that you can set via
   'make menuconfig'.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_MODE_AP   0 //TRUE:AP FALSE:STA
#define EXAMPLE_ESP_WIFI_SSID      "XAircraft"
#define EXAMPLE_ESP_WIFI_PASS      "5HK72E83CH"
#define EXAMPLE_MAX_STA_CONN       5

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int WIFI_CONNECTED_BIT = BIT0;

static const char *TAG = "simple wifi";

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "got ip:%s",
                 ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_AP_STACONNECTED:
        ESP_LOGI(TAG, "station:"MACSTR" join, AID=%d",
                 MAC2STR(event->event_info.sta_connected.mac),
                 event->event_info.sta_connected.aid);
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        ESP_LOGI(TAG, "station:"MACSTR"leave, AID=%d",
                 MAC2STR(event->event_info.sta_disconnected.mac),
                 event->event_info.sta_disconnected.aid);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

void wifi_init_softap()
{
    wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished.SSID:%s password:%s",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
}

void wifi_init_sta()
{
    wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL) );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
}

static void display_task()
{
    //int times = 0;

    int i = 0;

    while(1)
    {
        switch(i)
        {
            case 0:LCD_Clear(RED);BACK_COLOR = WHITE;POINT_COLOR = BLACK;
            LCD_ShowString(1,0,lcddev.width,16,16," red ");
            break;

            case 1:LCD_Clear(GREEN);BACK_COLOR = WHITE;POINT_COLOR = BLACK;
            LCD_ShowString(1,0,lcddev.width,16,16,"green");
            break;

            case 2:LCD_Clear(BLUE);BACK_COLOR = WHITE;POINT_COLOR = BLACK;
            LCD_ShowString(1,0,lcddev.width,16,16," blue");
            break;

            case 3:LCD_Clear(WHITE);BACK_COLOR = WHITE;POINT_COLOR = BLACK;
            LCD_ShowString(1,0,lcddev.width,16,16,"white");
            break;
        }
        i++;
        if(i >= 3) i = 0;
        //brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, i*30.0);
        vTaskDelay(1000 / portTICK_RATE_MS);
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
static void wifi_task()
{
   //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
#if EXAMPLE_ESP_WIFI_MODE_AP
    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();
#else
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
#endif /*EXAMPLE_ESP_WIFI_MODE_AP*/
}

void app_main(void)
{   
    Lcd_Init();   //tft初始化
    printf("lcd init OK!\n");
    
    //LCD_Display_Dir(L2R_U2D);
    tp_dev.init();//触摸初始化
    //LCD_Display_Dir(U2D_L2R);
    
    xTaskCreate(display_task, "lcd_display_task", 1500, NULL, 10, NULL);
    //xTaskCreate(wifi_task, "sta_wifi_task", 1500, NULL, 11, NULL);
    //xTaskCreate(MFRC_main, "NFC_rc522_task", 1500, NULL, 10, NULL);
}
