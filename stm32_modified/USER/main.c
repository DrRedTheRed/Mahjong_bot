/*
 * @文件描述:
 * @作者: Q
 * @Date: 2023-02-22 13:48:59
 * @LastEditTime: 2023-04-12 19:39:12
 * 实现的功能：
    1、手柄按钮控制0-5号舵机，摇杆控制6-7号电机；
    2、zide图形化控制舵机
    3、可脱机存储控制

    传感器引脚:
        触摸（S1-PA1）
				红外(S2-PA0)
        超声波(S3-PB0 PA2)
        声音(S4-PB1)
				RGB灯（S5-PA6）
				颜色识别(S6-PA5 PA7)
				
    舵机引脚：
        DJ0-PB3
        DJ1-PB8
        DJ2-PB9
        DJ3-PB6
        DJ4-PB7
        DJ5-PB4
    蜂鸣器引脚：
        BEEP-PB5
    LED引脚：
        NLED-PB13
  PS2手柄引脚：
      PS1-DAT-PA15
      PS2-CMD-PA14
      PS6-ATT-PA13
      PS7-CLK-PA12
    按键引脚：
      KEY1-PA8 KEY2-PA11

    统一总线口： TX3 RX3

    主频：72M
    单片机型号：STM32F103C8T6
 */
#include "main.h" /* 包含各类驱动文件 */
#include "tcs34725/y_tcs34725.h"
#include "ws2812b/y_ws2812b.h"

// 初始化其他
void others_init(void);
void loop_action(void);
void SWJ_gpio_init(void); /* SWJ引脚配置 */

int main(void)
{
    rcc_init();      /* 时钟初始化 */
    SysTick_Init();  /* 初始化系统嘀答定时器，1ms定时一次 */
    SWJ_gpio_init(); /* 禁用(JTAG-DP + SW-DP) */

    servo_init();             /* 舵机初始化 */
    TIM2_init(20000, 72 - 1); /* 初始化定时器2，用于pwm控制舵机 */

    spi_flash_init(); /* 初始化SPI FLASH的IO口 */

    app_gpio_init(); /* 初始化gpio相关引脚 */
    app_uart_init(); /*  初始化相关串口 */
    app_ps2_init();  /* 初始化PS2手柄 */

    app_sensor_init(); /* 初始化传感器功能 */

    others_init(); /* 初始化其他 */

    interrupt_open();  /* 初始化总中断 */
    app_setup_start(); /* 应用程序开始 */

    // kinematics 100mm 105mm 88mm 155mm
    setup_kinematics(100, 105, 88, 155, &kinematics);/*逆运动学初始化*/
		kinematics_move(100,100,30, 1000);//防止运动解算不出来
		
    while (1)
    {
        app_led_run();  /* 循环执行工作指示灯 */
        app_uart_run(); /* 串口应用循环运行 */
        app_ps2_run();  /* 循环处理PS2手柄上数据 */
        loop_action();  /* 动作组批量执行 */
        // loop_monitor(); // 定时保存一些变量

        app_sensor_run(); // 微信小程序功能

  
        /* 逆运动学测试 */
        // kinematics_move(0, 200, 100, 1000);
        // mdelay(2000);
        // kinematics_move(0, 200, 250, 1000);
        // mdelay(2000);
    }
}

// 初始化其他
void others_init(void)
{
    uint8_t i = 0;

    spiFlahsOn(1);
    delay_ms(10);
    w25x_read((u8 *)(&eeprom_info), W25Q64_INFO_ADDR_SAVE_STR, sizeof(eeprom_info)); // 读取全局变量

    if (eeprom_info.version != VERSION) // 判断版本是否是当前版本
    {
        eeprom_info.version = VERSION; // 复制当前版本
        eeprom_info.dj_record_num = 0; // 学习动作组变量赋值0
    }

    if (eeprom_info.dj_bias_pwm[DJ_NUM] != FLAG_VERIFY)
    {
        for (i = 0; i < DJ_NUM; i++)
        {
            eeprom_info.dj_bias_pwm[i] = 0;
        }
        eeprom_info.dj_bias_pwm[DJ_NUM] = FLAG_VERIFY;
    }

    // 将偏差带入初始值
    for (i = 0; i < DJ_NUM; i++)
    {
        set_servo((int)i, 1500, 0);
    }
    spiFlahsOn(0);

    //printf("\r\npre_cmd = %u  FLAG_VERIFY=37\r\n", eeprom_info.pre_cmd[PRE_CMD_SIZE]);

    // 执行预存命令 {G0000#000P1500T1000!#000P1500T1000!}
    if (eeprom_info.pre_cmd[PRE_CMD_SIZE] == FLAG_VERIFY)
    {
        if (eeprom_info.pre_cmd[0] == '$')
        {
            parse_cmd(eeprom_info.pre_cmd);
        }
    }
}

/* 单片机软件复位 */
void soft_reset(void)
{
    printf("stm32 reset\r\n");
    // 关闭所有中断
    __set_FAULTMASK(1);
    // 复位
    NVIC_SystemReset();
}

/* SWJ引脚配置 */
void SWJ_gpio_init(void)
{
    /**********************
    1.执行端口重映射时,复用功能时钟得使能:RCC_APB2Periph_AFIO

    2.  &1.GPIO_Remap_SWJ_Disable: !< Full SWJ Disabled (JTAG-DP + SW-DP)
         此时PA13|PA14|PA15|PB3|PB4都可作为普通IO用了
       为了保存某些调试端口,GPIO_Remap_SWJ_Disable也可选择为下面两种模式：

        &2.GPIO_Remap_SWJ_JTAGDisable: !< JTAG-DP Disabled and SW-DP Enabled
        此时PA15|PB3|PB4可作为普通IO用了

        &3.GPIO_Remap_SWJ_NoJTRST: !< Full SWJ Enabled (JTAG-DP + SW-DP) but without JTRST
        此时只有PB4可作为普通IO用了
    **********************/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE); // 使能 PA 端口时钟
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);                                               // 使能禁止JTAG和SW-DP
    // GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); // 使能禁止JTAG
}
