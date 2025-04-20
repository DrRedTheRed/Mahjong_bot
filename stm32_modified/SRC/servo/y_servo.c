/****************************************************************************
 *	@笔者	：	Q
 *	@日期	：	2023年2月8日
 *	@所属	：	杭州友辉科技
 *	@功能	：	存放舵机相关的函数
 *	@函数列表:
 *	1.	void servo_init(void) -- 舵机gpio初始化
 *	2.	void servo_pin_set(u8 index, BitAction level) -- 设置舵机引脚电平函数
 *	3.	void duoji_doing_set(u8 index, int aim, int time) -- 设置舵机控制参数函数
 ****************************************************************************/
#include "./servo/y_servo.h"

servo_t duoji_doing[DJ_NUM];

/* 舵机gpio初始化 */
void servo_init(void)
{
    u8 i;
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(SERVO0_GPIO_CLK | SERVO1_GPIO_CLK | SERVO2_GPIO_CLK | SERVO3_GPIO_CLK | SERVO4_GPIO_CLK | SERVO5_GPIO_CLK, ENABLE); /* 使能 舵机 端口时钟 */

    GPIO_InitStructure.GPIO_Pin = SERVO0_PIN;         /* 配置引脚 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; /* IO翻转50MHZ */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  /* 推挽输出 */
    GPIO_Init(SERVO0_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = SERVO1_PIN;
    GPIO_Init(SERVO1_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = SERVO2_PIN;
    GPIO_Init(SERVO2_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = SERVO3_PIN;
    GPIO_Init(SERVO3_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = SERVO4_PIN;
    GPIO_Init(SERVO4_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = SERVO5_PIN;
    GPIO_Init(SERVO5_GPIO_PORT, &GPIO_InitStructure);

    for (i = 0; i < DJ_NUM; i++)
    {
        duoji_doing[i].aim = 1500;
        duoji_doing[i].cur = 1500;
        duoji_doing[i].inc = 0;
        duoji_doing[i].time = 5000;
    }
}

/***********************************************
    功能介绍：	设置舵机引脚电平
    函数参数1：	index 要设置的舵机引脚索引
    函数参数2：	level 要设置的舵机引脚电平，1为高，0为低
    返回值：无
 ***********************************************/
void servo_pin_set(u8 index, BitAction level)
{
    switch (index)
    {
    case 0:
        SERVO0_PIN_SET(level);
        break;
    case 1:
        SERVO1_PIN_SET(level);
        break;
    case 2:
        SERVO2_PIN_SET(level);
        break;
    case 3:
        SERVO3_PIN_SET(level);
        break;
    case 4:
        SERVO4_PIN_SET(level);
        break;
    case 5:
        SERVO5_PIN_SET(level);
        break;
    default:
        break;
    }
}

/***********************************************
    功能介绍：	设置舵机控制参数函数
    函数参数：	index 舵机编号 aim 执行目标 time 执行时间(如果aim 执行目标==0，视为舵机停止)
    返回值：		无
 ***********************************************/
void duoji_doing_set(u8 index, int aim, int time)
{
    /* 限制输入值大小 */
    if (index >= DJ_NUM)
        return;

    if (aim == 0)
    {
        duoji_doing[index].inc = 0;
        duoji_doing[index].aim = duoji_doing[index].cur;
        return;
    }

    if (aim > 2490)
        aim = 2490;
    else if (aim < 510)
        aim = 510;

    if (time > 10000)
        time = 10000;

    if (duoji_doing[index].cur == aim)
    {
        aim = aim + 0.0077;
    }

    if (time < 20) /* 执行时间太短，舵机直接以最快速度运动 */
    {
        duoji_doing[index].aim = aim;
        duoji_doing[index].cur = aim;
        duoji_doing[index].inc = 0;
    }
    else
    {
        duoji_doing[index].aim = aim;
        duoji_doing[index].time = time;
        duoji_doing[index].inc = (duoji_doing[index].aim - duoji_doing[index].cur) / (duoji_doing[index].time / 20.000);
    }
}

/* 设置舵机每次增加的偏移量 */
void servo_inc_offset(u8 index)
{
    int aim_temp;

    if (duoji_doing[index].inc != 0)
    {

        aim_temp = duoji_doing[index].aim;

        if (aim_temp > 2490)
        {
            aim_temp = 2490;
        }
        else if (aim_temp < 500)
        {
            aim_temp = 500;
        }

        if (abs_float(aim_temp - duoji_doing[index].cur) <= abs_float(duoji_doing[index].inc + duoji_doing[index].inc))
        {
            duoji_doing[index].cur = aim_temp;
            duoji_doing[index].inc = 0;
        }
        else
        {
            duoji_doing[index].cur += duoji_doing[index].inc;
        }
    }
}
