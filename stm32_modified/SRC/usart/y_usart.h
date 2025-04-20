/****************************************************************************
 *	@笔者	：	Q
 *	@日期	：	2023年2月8日
 *	@所属	：	杭州友辉科技
 *	@功能	：	存放usart串口相关的函数
 ****************************************************************************/

#ifndef _Y_USART_H_
#define _Y_USART_H_
#include "main.h"

#define UART_BUF_SIZE 1024 // 定义最大接收字节数 128

extern u8 uart_receive_buf[UART_BUF_SIZE]; // 接收缓冲,最大UART_BUF_SIZE个字节.末字节为换行符
extern uint16_t uart1_get_ok;              // 接收完成标记
extern u8 uart1_mode;                      /* 指令的模式 */

#define interrupt_open() {__enable_irq();}

#define uart1_open() 	{USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);}
#define uart1_close() 	{USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);}

#define uart2_open() 	{USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);}		
#define uart2_close() 	{USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);}		

#define uart3_open() 	{USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);}		
#define uart3_close() 	{USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);}

/*******串口相关函数声明*******/
void uart1_init(uint32_t BaudRate); /* 初始化串口1 */
void uart1_send_str(u8 *s);         /* Usart1发送字符串 */
void uart1_send_int(int tmp);       /*	串口1发送数字 */

void uart3_init(u32 baud);
void uart3_send_str(u8 *s);

#endif
