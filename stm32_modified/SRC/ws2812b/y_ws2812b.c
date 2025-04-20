/*
 * @文件描述:
 * @作者: Q
 * @Date: 2023-02-25 16:14:54
 * @LastEditTime: 2023-04-14 08:42:24
 */
#include "ws2812b/y_ws2812b.h"
#include "./delay/y_delay.h"

/* 幻彩颜色测试缓存 */
const uint32_t color_grb_buffer[8] = {COLOR_RED, COLOR_ORANGE, COLOR_YELLOW, COLOR_GREEN, COLOR_CYAN, COLOR_BLUE, COLOR_PURPLE, 0x00000000};

/* 幻彩颜色缓存 */
uint16_t ws2812b_led_buffer[COLOR_DATA_SIZE * WS2812B_NUM];

static uint8_t send_busy_flag = 0; /* 用来检测是否发送完数据 */

/* 延时函数,100us即可 */
void ws2812b_delay(void)
{
    delay_us(1000);
}

/* 修改引脚输出还是复用 */
void ws2812b_config_gpio_AF(void)
{
    /* 配置GPIO变量 */
    GPIO_InitTypeDef GPIO_InitStructure;
    // 初始化GPIO口
    GPIO_InitStructure.GPIO_Pin = WS2812B_PIN;        // GPIO口
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 速度50MHz
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   // 复用推挽输出
    GPIO_Init(WS2812B_PORT, &GPIO_InitStructure);     // 根据指定的参数初始化GPIO口
}
void ws2812b_config_gpio_OUT(void)
{
    /* 配置GPIO变量 */
    GPIO_InitTypeDef GPIO_InitStructure;
    // 初始化GPIO口
    GPIO_InitStructure.GPIO_Pin = WS2812B_PIN;        // GPIO口
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 速度50MHz
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // 复用推挽输出
    GPIO_Init(WS2812B_PORT, &GPIO_InitStructure);     // 根据指定的参数初始化GPIO口
}

/**
 * @函数描述:   配置幻彩灯相关引脚和需要的外设
 * @param {uint16_t} arr 设置计数溢出大小
 * @return {*}
 */
void ws2812b_init(uint16_t arr)
{
    // 结构体变量
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    DMA_InitTypeDef DMA_InitStructure;

    // 使能RCC时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE); // 使能PORTA时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);                        // 使能TIM3时钟
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);                          // 使能DMA1时钟

    ws2812b_config_gpio_AF();

    NVIC_EnableIRQ(TIM3_IRQn);
    // 初始化TIM3
    TIM_TimeBaseStructure.TIM_Prescaler = 0;                    // 定时器分频：(0+1)=1,不分频
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数模式
    TIM_TimeBaseStructure.TIM_Period = arr;                     // 自动重装载值
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     // 时钟分割
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);             // 根据指定的参数初始化TIM3

    // 初始化TIM3 Channel1 PWM模式
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;             // 选择定时器模式:TIM脉冲宽度调制模式1
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // 比较输出使能
    // TIM_OCInitStructure.TIM_Pulse = 50;													//待装入捕获比较寄存器的脉冲值(此程序不用加初值)
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; // 输出极性:TIM输出比较极性高
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);                  // 根据指定的参数初始化外设TIM3 Channel1
    /* 此处为知识点所描述处的对应代码，一定要有。移植时注意0C1代表定时器通道1*/
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable); // 使能TIM3在CCR1上的预装载寄存器

    TIM_Cmd(TIM3, DISABLE); // 失能TIM3，防止第一个脉冲异常

    TIM_DMACmd(TIM3, TIM_DMA_CC1, ENABLE); // 使能TIM3_CH1的DMA功能(CC1对应通道1)

    NVIC_EnableIRQ(DMA1_Channel6_IRQn);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) & (TIM3->CCR1);       // 设置DMA目的地址
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ws2812b_led_buffer;        // 设置DMA源地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;                          // 方向：从内存SendBuff到内存ReceiveBuff
    DMA_InitStructure.DMA_BufferSize = COLOR_DATA_SIZE;                         // 一次传输大小DMA_BufferSize=SendBuffSize
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;            // ReceiveBuff地址不增
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                     // SendBuff地址自增
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; // ReceiveBuff数据单位,16bit
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;         // SENDBUFF_SIZE数据单位,16bit
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                               // DMA模式：正常模式(传输一次)
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                       // 优先级：中
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                                // 内存到内存的传输
    DMA_Init(DMA1_Channel6, &DMA_InitStructure);                                // 配置DMA1的7通道(不同定时器的通道不一样)

    DMA_Cmd(DMA1_Channel6, DISABLE); // 失能DMA1的7通道，因为一旦使能就开始传输
}

/* 用于检测是否还有数据发送的函数
    返回 0==忙  1==空闲 */
uint8_t ws2812b_busy_check(void)
{
    uint8_t cnt = 0;
    while (send_busy_flag)
    {
        cnt++;
        if (cnt > 250)
            return 1;
        ws2812b_delay();
    }
    return 0;
}

/* 复位所有灯 */
void ws2812b_reset(void)
{
    ws2812b_config_gpio_OUT();
    WS2812B_LOW();
    ws2812b_delay(); /* 此处延时时间最小值大于50us即可 */
    ws2812b_config_gpio_AF();
    send_busy_flag = 0; /* 标记空闲 */
}

/**
 * @函数描述: 把要显示的颜色写到缓存
 * @param {uint16_t} num 要修改的灯珠号
 * @param {uint32_t} RGB_Color 24位颜色
 * @return {*}
 */
void ws2812b_write_24bit(uint16_t num, uint32_t RGB_Color)
{
    uint8_t i;
    if (num > WS2812B_NUM - 1)
        num = WS2812B_NUM - 1;

    for (i = 0; i < COLOR_DATA_SIZE; i++)
    {
        /*因为数据发送的顺序高位先发，所以从高位开始判断，判断后比较值先放入缓存数组*/
        ws2812b_led_buffer[num * COLOR_DATA_SIZE + i] = ((RGB_Color << i) & 0x800000) ? T1H : T0H;
    }
}

/**
 * @函数描述: 填充幻彩灯颜色，
 * @param {uint32_t} RGB_Color 颜色
 * @return {*}
 */
void ws2812b_fill(uint32_t RGB_Color)
{
    uint16_t num;
    memset(ws2812b_led_buffer, 0, sizeof(ws2812b_led_buffer)); // 清空数组,
    for (num = 0; num < WS2812B_NUM; num++)
    {
        ws2812b_write_24bit(num, RGB_Color);
    }
}

/**
 * @函数描述: 修改显示幻彩灯的RGB
 * @param {uint16_t} num 要修改的的灯珠数目，幻彩灯只能按灯珠的顺序修改
 * @return {*}
 */
void ws2812b_show(uint16_t num)
{
    if (ws2812b_busy_check() == 1) /* 检测忙 */
        return;
    send_busy_flag = 1; /* 标记忙 */

    /* 移植时此处对应的通道和中断标志都需要更改 */
    DMA_SetCurrDataCounter(DMA1_Channel6, num * COLOR_DATA_SIZE);
    DMA_Cmd(DMA1_Channel6, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
    while (DMA_GetFlagStatus(DMA1_FLAG_TC6) != SET)
        ;
    DMA_Cmd(DMA1_Channel6, DISABLE);
    DMA_ClearFlag(DMA1_FLAG_TC6);

    /* 最后步骤在定时器中断里完成 */
    TIM_ClearFlag(TIM3, TIM_FLAG_Update);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE); /* 开启定时器中断 */

    ws2812b_reset(); /* 完成发送数据后复位彩灯，让其显示更换的颜色 */
}

/**
 * @函数描述: 中断方式修改显示幻彩灯的RGB,节约cpu资源，但会让中断函数处理时间更多，不建议使用
 * @param {uint16_t} num 要修改的的灯珠数目，幻彩灯只能按灯珠的顺序修改
 * @return {*}
 */
void ws2812b_show_dma_isr(uint16_t num)
{
    if (ws2812b_busy_check() == 1) /* 检测忙 */
        return;
    send_busy_flag = 1; /* 标记忙 */

    DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, ENABLE); // 配置DMA发送完成后产生中断
    /* 移植时此处对应的通道和中断标志都需要更改 */
    DMA_SetCurrDataCounter(DMA1_Channel6, num * COLOR_DATA_SIZE);
    DMA_ClearFlag(DMA1_FLAG_TC6);
    DMA_Cmd(DMA1_Channel6, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}

void DMA1_Channel6_IRQHandler(void)
{
    if (DMA_GetFlagStatus(DMA1_FLAG_TC6) == SET)
    {
        DMA_ClearFlag(DMA1_FLAG_TC6);

        TIM_ClearFlag(TIM3, TIM_FLAG_Update);
        TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE); /* 开启定时器中断 */

        DMA_Cmd(DMA1_Channel6, DISABLE);
        DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, DISABLE); // 配置DMA发送完成后产生中断
    }
}

// 定时器3中断服务程序
void TIM3_IRQHandler(void) // TIM3中断
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) // 检查TIM3更新中断发生与否
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update); // 清除TIMx更新中断标志 (单片机要靠查询中断标志来判断是否要进入中断，如果你不清除中断标志，本次中断退出，单片机又会检测到中断标志，因此重复进入中断)
        TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
        TIM_Cmd(TIM3, DISABLE);

        // ws2812b_reset();
    }
}

/**
 * @函数描述: RGB彩灯测试程序
 * @return {*}
 */
void ws2812b_test(uint8_t color_num)
{
    uint8_t i;

    if (color_num == 255)
    {
        for (i = 0; i < 3; i++)
        {
            ws2812b_fill(color_grb_buffer[0]); /* 给所有led灯缓存写颜色 */
            ws2812b_show(WS2812B_NUM);         /* 发送数据给WS2812B */
            delay_ms(200);
            ws2812b_fill(color_grb_buffer[3]); /* 给所有led灯缓存写颜色 */
            ws2812b_show(WS2812B_NUM);         /* 发送数据给WS2812B */
            delay_ms(200);
            ws2812b_fill(color_grb_buffer[5]); /* 给所有led灯缓存写颜色 */
            ws2812b_show(WS2812B_NUM);         /* 发送数据给WS2812B */
            delay_ms(200);
        }

        ws2812b_fill(color_grb_buffer[0]); /* 给所有led灯缓存写颜色 */
        ws2812b_show(WS2812B_NUM);         /* 发送数据给WS2812B */
        delay_ms(500);
        ws2812b_fill(color_grb_buffer[5]); /* 给所有led灯缓存写颜色 */
        ws2812b_show(WS2812B_NUM);         /* 发送数据给WS2812B */
        delay_ms(500);
        ws2812b_fill(color_grb_buffer[3]); /* 给所有led灯缓存写颜色 */
        ws2812b_show(WS2812B_NUM);         /* 发送数据给WS2812B */

        // ws2812b_fill(color_grb_buffer[3]); /* 给所有led灯缓存写颜色 */
        // ws2812b_show(WS2812B_NUM);         /* 发送数据给WS2812B */
        delay_ms(500);

        ws2812b_fill(color_grb_buffer[7]); /* 给所有led灯缓存写颜色 */
        ws2812b_show(WS2812B_NUM);         /* 发送数据给WS2812B */
    }
    else
    {
        if (color_num > 7)
            color_num = 7;

        ws2812b_fill(color_grb_buffer[color_num]); /* 给所有led灯缓存写颜色 */
        ws2812b_show(WS2812B_NUM);                 /* 发送数据给WS2812B */
    }
}
