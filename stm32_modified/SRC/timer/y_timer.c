/****************************************************************************
 *	@笔者	：	Q
 *	@日期	：	2023年2月8日
 *	@所属	：	杭州友辉科技
 *	@功能	：	存放定时器相关的函数
 *	@函数列表:
 *	1.	void systick_init(void) -- 初始化systick
 *	2.	void SysTick_Handler(void) interrupt 19 -- systick中断函数
 *	3.	u32 millis(void) -- 滴答时钟查询
 *	4.	void TIM2_init(void) -- 初始化TIM2
 *	5.	void TIM2_IRQHandler(void) -- 输出舵机控制波形
 ****************************************************************************/
#include "./timer/y_timer.h"

static u32 systick_ms = 0; /* 记录时间 */

/* 初始化systick */
void SysTick_Init(void) // 1毫秒@72MHz
{
	SysTick_Config(SystemCoreClock / 1000);
}

/* SysTick中断 */
void SysTick_Handler(void)
{
	// static uint8_t key_scan_times = 0;
	systick_ms++;

	// /* 10ms检查一次按键状态 */
	// key_scan_times++;
	// if (key_scan_times >= 10)
	// {
	// 	key_scan_times = 0;
	// 	key_scan();
	// }
}

/* 获取滴答时钟数值 */
u32 millis(void)
{
	return systick_ms;
}

/**
 * @函数描述: 初始化TIM2，用于生成pwm控制舵机
 * @param {u16} arr 计数器自动重装值
 * @param {u16} psc 预分频器
 * @return {*}
 */
void TIM2_init(u16 arr, u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); // 时钟 TIM2 使能
	// 定时器 TIM2 初始化
	TIM_TimeBaseStructure.TIM_Period = arr;						// 设置自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler = psc;					// 设置时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		// 输入捕获分频：不分频
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // TIM 向上计数
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);				// ②初始化 TIM2
	TIM_ARRPreloadConfig(TIM2, DISABLE);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); // ③允许更新中断

	// 中断优先级 NVIC 设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;			  // TIM2 中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // 先占优先级 0 级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  // 从优先级 2 级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ 通道被使能
	NVIC_Init(&NVIC_InitStructure);							  // ④初始化 NVIC 寄存器
	TIM_Cmd(TIM2, ENABLE);									  // ⑤使能 TIM2
}

/**
 * @函数描述: 定时器1初始化
 * @param {u16} arr 计数器自动重装值
 * @param {u16} psc 预分频器
 * @return {*}
 */
void timer1_init(u16 arr, u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE); // 时钟 TIM1 使能

	TIM_TimeBaseInitStructure.TIM_Period = arr;						/* 设定计数器自动重装值 */
	TIM_TimeBaseInitStructure.TIM_Prescaler = psc;					/* 预分频器 */
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;		/* 设置时钟分割:TDTS = Tck_tim */
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; /* TIM向上计数模式 */
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);
}

/* 定时器2中断函数，输出舵机控制波形 */
void TIM2_IRQHandler(void)
{
    static u8 flag = 0;
    static u8 duoji_index1 = 0;
    int temp;

    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) // 检查 TIM2 更新中断发生与否
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update); // 清除 TIM2 更新中断标志

        /* 通过改变重装载值和舵机下标索引，每个舵机定时2500（2.5ms），执行8个舵机后完成一个周期20000（20ms） */
        if (duoji_index1 == 8)
        {
            duoji_index1 = 0;
        }

        // 如果是3号或4号舵机，将它们的cur值取反
        if (duoji_index1 == 3 || duoji_index1 == 4)
        {
            temp = 3000 - (unsigned int)(duoji_doing[duoji_index1].cur);  // 反向处理
        }
        else
        {
            temp = (unsigned int)(duoji_doing[duoji_index1].cur);  // 正常处理
        }

        if (flag == 0)
        {
            TIM2->ARR = temp;  // 设置定时器重装载值
            servo_pin_set(duoji_index1, Bit_SET);  // 使能舵机
            servo_inc_offset(duoji_index1);  // 调整舵机位置
        }
        else
        {
            TIM2->ARR = 2500 - temp;  // 设置剩余的周期时间
            servo_pin_set(duoji_index1, Bit_RESET);  // 关闭舵机
            duoji_index1++;  // 下一个舵机
        }

        flag = !flag;  // 切换状态
    }
}

