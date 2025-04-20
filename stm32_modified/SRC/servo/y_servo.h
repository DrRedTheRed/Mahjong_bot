/****************************************************************************
 *	@笔者	：	Q
 *	@日期	：	2023年2月8日
 *	@所属	：	杭州友辉科技
 *	@功能	：	存放舵机相关的函数
 ****************************************************************************/
#ifndef _Y_SERVO_H_
#define _Y_SERVO_H_
#include "main.h"

#define SERVO0_PIN GPIO_Pin_3
#define SERVO0_GPIO_PORT GPIOB               /* GPIO端口 */
#define SERVO0_GPIO_CLK RCC_APB2Periph_GPIOB /* GPIO端口时钟 */

#define SERVO1_PIN GPIO_Pin_8
#define SERVO1_GPIO_PORT GPIOB               /* GPIO端口 */
#define SERVO1_GPIO_CLK RCC_APB2Periph_GPIOB /* GPIO端口时钟 */

#define SERVO2_PIN GPIO_Pin_9
#define SERVO2_GPIO_PORT GPIOB               /* GPIO端口 */
#define SERVO2_GPIO_CLK RCC_APB2Periph_GPIOB /* GPIO端口时钟 */

#define SERVO3_PIN GPIO_Pin_6
#define SERVO3_GPIO_PORT GPIOB               /* GPIO端口 */
#define SERVO3_GPIO_CLK RCC_APB2Periph_GPIOB /* GPIO端口时钟 */

#define SERVO4_PIN GPIO_Pin_7
#define SERVO4_GPIO_PORT GPIOB               /* GPIO端口 */
#define SERVO4_GPIO_CLK RCC_APB2Periph_GPIOB /* GPIO端口时钟 */

#define SERVO5_PIN GPIO_Pin_4
#define SERVO5_GPIO_PORT GPIOB               /* GPIO端口 */
#define SERVO5_GPIO_CLK RCC_APB2Periph_GPIOB /* GPIO端口时钟 */

/* 控制舵机引脚输出的宏 */
#define SERVO0_PIN_SET(level) GPIO_WriteBit(SERVO0_GPIO_PORT, SERVO0_PIN, level)
#define SERVO1_PIN_SET(level) GPIO_WriteBit(SERVO1_GPIO_PORT, SERVO1_PIN, level)
#define SERVO2_PIN_SET(level) GPIO_WriteBit(SERVO2_GPIO_PORT, SERVO2_PIN, level)
#define SERVO3_PIN_SET(level) GPIO_WriteBit(SERVO3_GPIO_PORT, SERVO3_PIN, level)
#define SERVO4_PIN_SET(level) GPIO_WriteBit(SERVO4_GPIO_PORT, SERVO4_PIN, level)
#define SERVO5_PIN_SET(level) GPIO_WriteBit(SERVO5_GPIO_PORT, SERVO5_PIN, level)

#define DJ_NUM 8 /* 舵机数量，为8是因为定时器中断计算pwm周期需要 */

typedef struct
{
    // uint8_t valid; // 有效 TODO
    uint16_t aim;  // 执行目标
    uint16_t time; // 执行时间
    float cur;     // 当前值
    float inc;     // 增量
} servo_t;

extern servo_t duoji_doing[DJ_NUM];

/*******LED相关函数声明*******/
void servo_init(void);                            /* 舵机引脚初始化 */
void servo_pin_set(u8 index, BitAction level);    /* 设置舵机引脚电平 */
void duoji_doing_set(u8 index, int aim, int time); /* 设置舵机参数 */
void servo_inc_offset(u8 index);                  /* 设置舵机每次增加的偏移量 */
#endif
