#ifndef _MAIN_H_
#define _MAIN_H_
#include <stdio.h>  //标准库文件
#include <string.h> //标准库文件
#include <math.h>   //标准库文件

#include "stm32f10x_conf.h"
#include "stm32f10x.h"

/* 驱动 */
#include "./rcc/y_rcc.h"     /* 包含时钟配置文件 */
#include "./delay/y_delay.h" /* 包含延时函数 */
#include "./timer/y_timer.h" /* 包含定时相关函数 */
#include "./led/y_led.h"     /* 包含led相关函数 */
#include "./beep/y_beep.h"   /* 包含蜂鸣器beep相关函数 */
#include "./key/y_key.h"     /* 包含按键key相关函数 */
#include "./usart/y_usart.h" /* 串口相关函数 */
#include "./flash/y_flash.h" /* W25Q64 FLASH相关函数 */
#include "./ps2/y_ps2.h"     /* 包含手柄ps2相关文件 */
#include "./servo/y_servo.h" /* 舵机相关函数 */

#include "./kinematics/y_kinematics.h" /* 逆运动学算法 */
#include "./global/y_global.h"

/* 应用 */
#include "app_gpio.h" /* gpio 任务相关函数 */
#include "app_uart.h" /* 串口任务相关函数 */
#include "app_ps2.h"  /* PS2手柄任务相关 */
#include "app_sensor.h"

typedef u8 bool;



void soft_reset(void); /* 单片机软件复位 */
#endif
