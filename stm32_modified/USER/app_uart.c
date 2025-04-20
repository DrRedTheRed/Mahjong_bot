/*
 * @文件描述:
 * @作者: Q
 * @Date: 2023-02-13 14:01:12
 * @LastEditTime: 2023-03-15 16:17:46
 */
#include "app_uart.h"

/**
 * @函数描述: uart串口相关设备控制初始化
 * @return {*}
 */
void app_uart_init(void)
{
    uart1_init(115200); /* uart1串口初始化 */
    uart3_init(115200); /* 连接总线设备串口 */

    printf("uart1_init succeed\r\n");
}

/**
 * @函数描述: 循环检测串口接收到的指令
 * @return {*}
 */
void app_uart_run(void)
{
    if (uart1_get_ok)
    {
        // printf("\r\n  app_uart_run = %s \r\n", uart_receive_buf);
        if (uart1_mode == 1)
        {
            // 命令模式
            parse_cmd(uart_receive_buf);
        }
        else if (uart1_mode == 2)
        {
            // 单个舵机调试
            parse_action(uart_receive_buf);
        }
        else if (uart1_mode == 3)
        {
            // 多路舵机调试
            parse_action(uart_receive_buf);
        }
        else if (uart1_mode == 4)
        {
            // 存储模式
            save_action(uart_receive_buf);
        }
        uart1_mode = 0;
        uart1_get_ok = 0;
    }
}
