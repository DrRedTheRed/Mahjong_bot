#ifndef _Y_SENSOR_H_
#define _Y_SENSOR_H_
#include "main.h"

/*******传感器IO口映射表*******/
#define Trig_Pin GPIO_Pin_0//超声波引脚
#define Trig_GPIO_Port GPIOB

#define Echo_Pin GPIO_Pin_2
#define Echo_GPIO_Port GPIOA

/*******传感器快捷指令表*******/
#define TRIG_SET(x) GPIO_WriteBit(Trig_GPIO_Port, Trig_Pin, (BitAction)x)
#define ECHO_GET() GPIO_ReadInputDataBit(Echo_GPIO_Port, Echo_Pin)

#define shengyin_Read() GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1)

#define chumo_READ() GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1)

#define hongwai_READ() GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)



void ultrasonic_sensor_init(void);     /* 传感器初始化 */
float sensor_sr_ultrasonic_read(void); /* 读取SR超声波传感器数据 */
void AI_chumo_init(void);              /* 触摸传感器静态识别初始化 */
void AI_hongwai_init(void);            /* 红外传感器静态识别初始化 */
void AI_shengyin_init(void);           /* 声音传感器静态识别初始化 */
#endif
