#ifndef __MFRC_TASK_H_
#define __MFRC_TASK_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
//////////////////////////////////
//端口定义

#define     MF522_SDA  	 22       //SDA
#define     MF522_SCK    32       //SCK
#define     MF522_MOSI   33       //MOSI
#define     MF522_MISO   26       //MISO
#define     MF522_RST    21       //RST
//指示灯
#define     LED_GREEN   25
//蜂鸣器引脚定义
#define     MF522_IRQ    27

#define     MF522_SDA_1 gpio_set_level(MF522_SDA, 1)
#define     MF522_SDA_0 gpio_set_level(MF522_SDA, 0)

#define     MF522_SCK_1 gpio_set_level(MF522_SCK, 1)
#define     MF522_SCK_0 gpio_set_level(MF522_SCK, 0)

#define     MF522_MOSI_1 gpio_set_level(MF522_MOSI, 1)
#define     MF522_MOSI_0 gpio_set_level(MF522_MOSI, 0)

#define     _MF522_MISO gpio_get_level(MF522_MISO)

#define     MF522_RST_1 gpio_set_level(MF522_RST, 1)
#define     MF522_RST_0 gpio_set_level(MF522_RST, 0)

#define     LED_GREEN_1 gpio_set_level(LED_GREEN, 1)
#define     LED_GREEN_0 gpio_set_level(LED_GREEN, 0)


/////////////////////////////////////////////////////////////////////
//函数原型
/////////////////////////////////////////////////////////////////////
void InitializeSystem();    
void MFRC_main();                               
                                    
#endif
