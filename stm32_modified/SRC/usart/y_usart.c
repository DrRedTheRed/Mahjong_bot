/****************************************************************************
 *	@笔者	：	Q
 *	@日期	：	2023年2月8日
 *	@所属	：	杭州友辉科技
 *	@功能	：	存放usart串口相关的函数
 *	@函数列表:
 *	1.	void uart1_init(u32 baud) -- 初始化串口1
 *	2.	void uart3_init(u32 baud) -- 初始化串口3
 *	3.	void uart1_send_byte(u8 dat) -- 串口1发送字节
 *	4.	void uart3_send_byte(u8 dat) -- 串口3发送字节
 *	5.	void uart1_send_str(char *s) -- 串口1发送字符串
 *	6.	void uart3_send_str(char *s) -- 串口3发送字符串
 ****************************************************************************/

#include "./usart/y_usart.h"

u8 uart_receive_buf[UART_BUF_SIZE];
uint16_t uart1_get_ok;
u8 uart1_mode;

/* 初始化串口1 */
void uart1_init(uint32_t BaudRate)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* 使能端口时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

	USART_DeInit(USART1);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;		/* PA.9 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; /* 复用推挽输出 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; /* 浮空输入 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = BaudRate;									/* 串口波特率 */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						/* 字长为8位数据格式 */
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							/* 字长为8位数据格式 */
	USART_InitStructure.USART_Parity = USART_Parity_No;								/* 无奇偶校验位 */
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					/* 收发模式 */
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; /* 无硬件数据流控制 */
	USART_Init(USART1, &USART_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; /* 抢占优先级 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  /* 子优先级 */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  /* IRQ通道使能 */
	NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); /* 开启串口接受中断 */
	USART_ITConfig(USART1, USART_IT_TXE, DISABLE); /* 禁止串口发送中断 */

	USART_Cmd(USART1, ENABLE); /* 使能串口1  */
}

/***********************************************
	函数名称:	uart3_init()
	功能介绍:	初始化串口3
	函数参数:	baud 波特率
	返回值:		无
 ***********************************************/
void uart3_init(u32 baud)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	USART_ClockInitTypeDef USART_ClockInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;
	USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;
	USART_ClockInitStructure.USART_CPHA = USART_CPHA_2Edge;
	USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable;
	USART_ClockInit(USART3, &USART_ClockInitStructure);

	USART_InitStructure.USART_BaudRate = baud;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART3, USART_IT_TXE, DISABLE); /* 禁止串口发送中断 */

	USART_Cmd(USART3, ENABLE);
}

/***********************************************
	功能介绍：	串口1发送字节
	函数参数：	dat 发送的字节
	返回值：		无
 ***********************************************/
void uart1_send_byte(u8 dat)
{
	USART_SendData(USART1, dat);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
		;
}

/***********************************************
	功能介绍：	串口1发送字符串
	函数参数：	*s 发送的字符串
	返回值：		无
 ***********************************************/
void uart1_send_str(u8 *s)
{
	while (*s)
	{
		uart1_send_byte(*s++);
	}
}

/***********************************************
	功能介绍：	串口1发送数字
	函数参数：	tmp 发送的数字
	返回值：		无
 ***********************************************/
void uart1_send_int(int tmp)
{
	static u8 str[20];
	sprintf((char *)str, "%d", tmp);
	uart1_send_str(str);
}

/* 重定义fputc函数,写这个函数可以使用printf,记得开启Use MicroLIB */
int fputc(int ch, FILE *f)
{
	while ((USART1->SR & 0X40) == 0)
		; // 循环发送,直到发送完毕
	USART1->DR = (u8)ch;
	return ch;
}

/***********************************************
	函数名称:	uart3_send_byte()
	功能介绍:	串口3发送字节
	函数参数:	dat 发送的字节
	返回值:		无
 ***********************************************/
void uart3_send_byte(u8 dat)
{
	USART_SendData(USART3, dat);
	while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)
		;
	return;
}

/***********************************************
	函数名称:	uart3_send_str()
	功能介绍:	串口3发送字符串
	函数参数:	*s 发送的字符串
	返回值:		无
 ***********************************************/
void uart3_send_str(u8 *s)
{
	while (*s)
	{
		uart3_send_byte(*s++);
	}
}

/* 串口1中断服务程序 */
void USART1_IRQHandler(void) /* 最后数据发送\r\n结束 */
{
	u8 sbuf_bak;
	static u16 buf_index = 0;

	if (USART_GetFlagStatus(USART1, USART_IT_RXNE) == SET)
	{
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
		sbuf_bak = USART_ReceiveData(USART1);
		// USART_SendData(USART1, sbuf_bak);
		if (uart1_get_ok)
			return;
		if (sbuf_bak == '<')
		{
			uart1_mode = 4;
			buf_index = 0;
		}
		else if (uart1_mode == 0)
		{
			if (sbuf_bak == '$')
			{
				// 命令模式 $XXX!
				uart1_mode = 1;
			}
			else if (sbuf_bak == '#')
			{
				// 单舵机模式	#000P1500T1000! 类似这种命令
				uart1_mode = 2;
			}
			else if (sbuf_bak == '{')
			{
				// 多舵机模式	{#000P1500T1000!#001P1500T1000!} 多个单舵机命令用大括号括起来
				uart1_mode = 3;
			}
			else if (sbuf_bak == '<')
			{
				// 保存动作组模式	<G0000#000P1500T1000!#001P1500T1000!B000!> 用尖括号括起来 带有组序号
				uart1_mode = 4;
			}
			buf_index = 0;
		}

		uart_receive_buf[buf_index++] = sbuf_bak;

		if ((uart1_mode == 4) && (sbuf_bak == '>'))
		{
			uart_receive_buf[buf_index] = '\0';
			uart1_get_ok = 1;
		}
		else if ((uart1_mode == 1) && (sbuf_bak == '!'))
		{
			uart_receive_buf[buf_index] = '\0';
			uart1_get_ok = 1;
		}
		else if ((uart1_mode == 2) && (sbuf_bak == '!'))
		{
			uart_receive_buf[buf_index] = '\0';
			uart1_get_ok = 1;
		}
		else if ((uart1_mode == 3) && (sbuf_bak == '}'))
		{
			uart_receive_buf[buf_index] = '\0';
			uart1_get_ok = 1;
		}

		if (buf_index >= UART_BUF_SIZE)
			buf_index = 0;
	}
}

/***********************************************
	函数名称:	void USART3_IRQHandler(void)
	功能介绍:	串口3中断函数
	函数参数:	无
	返回值:		无
 ***********************************************/
void USART3_IRQHandler(void)
{
	u8 sbuf_bak;
	static u16 buf_index = 0;
	if (USART_GetFlagStatus(USART3, USART_IT_RXNE) == SET)
	{
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
		sbuf_bak = USART_ReceiveData(USART3);
		USART_SendData(USART1, sbuf_bak);
		if (uart1_get_ok)
			return;
		if (sbuf_bak == '<')
		{
			uart1_mode = 4;
			buf_index = 0;
		}
		else if (uart1_mode == 0)
		{
			if (sbuf_bak == '$')
			{
				uart1_mode = 1;
			}
			else if (sbuf_bak == '#')
			{
				uart1_mode = 2;
			}
			else if (sbuf_bak == '{')
			{
				uart1_mode = 3;
			}
			else if (sbuf_bak == '<')
			{
				uart1_mode = 4;
			}
			buf_index = 0;
		}

		uart_receive_buf[buf_index++] = sbuf_bak;

		if ((uart1_mode == 4) && (sbuf_bak == '>'))
		{
			uart_receive_buf[buf_index] = '\0';
			uart1_get_ok = 1;
		}
		else if ((uart1_mode == 1) && (sbuf_bak == '!'))
		{
			uart_receive_buf[buf_index] = '\0';
			uart1_get_ok = 1;
		}
		else if ((uart1_mode == 2) && (sbuf_bak == '!'))
		{
			uart_receive_buf[buf_index] = '\0';
			uart1_get_ok = 1;
		}
		else if ((uart1_mode == 3) && (sbuf_bak == '}'))
		{
			uart_receive_buf[buf_index] = '\0';
			uart1_get_ok = 1;
		}

		if (buf_index >= UART_BUF_SIZE)
			buf_index = 0;
	}
}
