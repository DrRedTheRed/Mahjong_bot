/*
 * @文件描述:
 * @作者: Q
 * @Date: 2023-02-11 10:19:05
 * @LastEditTime: 2023-04-14 08:51:22
 */
#include "./sensor/y_sensor.h"

/***********************************************
    ultrasonic_sensor_init()
    功能介绍：初始化超声波传感器
    函数参数：无
    返回值：	无
 ***********************************************/
void ultrasonic_sensor_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

    // 初始化超声波IO口 Trig PB0  Echo PA2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/**
 * @函数描述: 初始化静态感应传感器
 * @return {*}
 */
void AI_chumo_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; /* 如果要用触摸传感器请判断为设为下拉GPIO_Mode_IPD */
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void AI_hongwai_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void AI_shengyin_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/**
 * @函数描述: 获取距离
 * @return {*}返回距离 cm
 * 距离=高电平时间*声速（340M/S） /2
 */
float sensor_sr_ultrasonic_read(void)
{
    uint16_t csb_t;
    uint32_t start_time;
    uint32_t timeout = 20; // 设置超时时间为20毫秒
    
    TRIG_SET(1);
    delay_us(50);
    TRIG_SET(0);

    TIM_Cmd(TIM1, ENABLE); // 使能TIM13外设

    // 等待ECHO信号高电平
    start_time = millis();
    while (ECHO_GET() == 0)
    {
        if ((millis() - start_time) > timeout)
        {
            TIM_Cmd(TIM1, DISABLE); // 禁用TIM13外设
            return -1; // 超时返回
        }
    }

    TIM_SetCounter(TIM1, 0); // 清除计数

    // 等待ECHO信号低电平
    start_time = millis();
    while (ECHO_GET())
    {
        if ((millis() - start_time) > timeout)
        {
            TIM_Cmd(TIM1, DISABLE); // 禁用TIM13外设
            return -1; // 超时返回
        }
    }

    csb_t = TIM1->CNT;
    TIM_Cmd(TIM1, DISABLE); // 禁用TIM13外设

    // 340m/s = 0.034cm/us
    if (csb_t < 25000)
    {
        return (float)csb_t * 0.034f;
    }
    return -1;
}
