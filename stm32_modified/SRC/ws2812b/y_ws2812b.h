#ifndef _Y_WS2812B_H_
#define _Y_WS2812B_H_
#include "stm32f10x.h"
#include "stm32f10x_conf.h"

#define WS2812B_PIN GPIO_Pin_6
#define WS2812B_PORT GPIOA /* GPIO端口 */

/* 幻彩WS2812B控制引脚 */
#define WS2812B_HIGH() GPIO_SetBits(WS2812B_PORT, WS2812B_PIN)
#define WS2812B_LOW() GPIO_ResetBits(WS2812B_PORT, WS2812B_PIN)

/*	根据DataSheet定义0/1编码对应的高电平比较值，因为高电平在前，定义高电平的即可
**	高低电平的比例约为2:1
*/
#define WS2812B_ARR 90 // TIM2的自动重装载值
#define T0H 30         // 0编码高电平时间占1/3
#define T1H 60         // 1编码高电平时间占2/3

/* 灯珠亮的个数 */
#define WS2812B_NUM 6      // 底层驱动未用，为应用方便，先加上
#define COLOR_DATA_SIZE 24 // WS2812B一个编码的bit数，3*8

/* RGB颜色 */
// #define COLOR_RED 0XFF0000    /* 红色 */
// #define COLOR_ORANGE 0XFFA500 /* 橙色 */
// #define COLOR_YELLOW 0XFFFF00 /* 黄色 */
// #define COLOR_GREEN 0X008000  /* 绿色 */
// #define COLOR_CYAN 0X00FFFF   /* 青色 */
// #define COLOR_BLUE 0X0000FF   /* 蓝色 */
// #define COLOR_PURPLE 0XEE82EE    /* 紫色 */
// #define COLOR_ABAISER 0x292421      /* 象牙黑 */
// #define COLOR_ANTIQUEWHITE 0xFAEBD7 /* 古董白 */
// #define COLOR_CORNSILK 0xFFF8DC     /* 米绸色 */
// #define COLOR_CADMIUM_RED 0xE3170D  /* 镉红 */
// #define COLOR_HAZEN 0x3D59AB        /* 钴色 */
// #define COLOR_TOMATO_RED 0xFF6347   /* 蕃茄红 */

/* GRB颜色 */
#define COLOR_RED 0X00FF00    /* 红色 */
#define COLOR_ORANGE 0XA5FF00 /* 橙色 */
#define COLOR_YELLOW 0XFFFF00 /* 黄色 */
#define COLOR_GREEN 0XFF0000  /* 绿色 */
#define COLOR_CYAN 0XFF00FF   /* 青色 */
#define COLOR_BLUE 0X0000FF   /* 蓝色 */
#define COLOR_PURPLE 0X82EEEE /* 紫色 */

void ws2812b_delay(void);
uint8_t ws2812b_busy_check(void);
void ws2812b_reset(void);

void ws2812b_init(uint16_t arr);
void ws2812b_write_24bit(uint16_t num, uint32_t RGB_Color);
void ws2812b_fill(uint32_t RGB_Color);
void ws2812b_show(uint16_t num);
void ws2812b_show_dma_isr(uint16_t num);

void ws2812b_test(uint8_t color_num);
#endif
