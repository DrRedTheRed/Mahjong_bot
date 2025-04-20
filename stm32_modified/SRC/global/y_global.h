#ifndef _Y_GLOBAL_H_
#define _Y_GLOBAL_H_

#include "main.h"

#include "ws2812b/y_ws2812b.h"

/* 电机的三种速度，用不同电机时要修改 */
#define CAR_SPEED_RANK1 350
#define CAR_SPEED_RANK2 500
#define CAR_SPEED_RANK3 1000

/*
	宏定义数据
*/
#define VERSION 20180705  // 版本定义
#define ACTION_USE_ROM 0  // 1:动作组使用内部数组动作组	0:动作组使用上位机下载动作组
#define CYCLE 1000		  // PWM模块周期
#define PS2_LED_RED 0x73  // PS2手柄红灯模式
#define PS2_LED_GRN 0x41  // PS2手柄绿灯模式
#define PSX_BUTTON_NUM 16 // 手柄按键数目
#define PS2_MAX_LEN 64	  // 手柄命令最大字节数
#define FLAG_VERIFY 0x25  // 校验标志
#define ACTION_SIZE 256	  // 一个动作的存储大小

#define W25Q64_INFO_ADDR_SAVE_STR (((8 << 10) - 4) << 10) //(8*1024-4)*1024		//eeprom_info结构体存储的位置

#define MODULE "YH-KSTM32"

extern u8 AI_mode;
extern u8 group_do_ok;
extern int do_start_index;	// 动作组执行 起始序号
extern int do_time;			// 动作组执行 执行次数
extern int group_num_start; // 动作组执行 起始序号
extern int group_num_end;	// 动作组执行 终止序号
extern int group_num_times; // 动作组执行 起始变量

extern u8 needSaveFlag;			// 偏差保存标志
extern u32 bias_systick_ms_bak; // 偏差保存标志时间
extern u32 action_time;

#define DJ_NUM 8
#define car_mode 1 // 1是四驱  0是2驱

#define PRE_CMD_SIZE 128
typedef struct
{
	u32 version;
	u32 dj_record_num;
	u8 pre_cmd[PRE_CMD_SIZE + 1];
	int dj_bias_pwm[DJ_NUM + 1];
	u8 color_base_flag;
	int color_red_base;
	int color_grn_base;
	int color_blu_base;
} eeprom_info_t;

extern uint8_t duoji_index1;
extern uint8_t tim2_stop;

#define CMD_RETURN_SIZE 1024
extern u8 cmd_return[CMD_RETURN_SIZE];
extern eeprom_info_t eeprom_info;

uint16_t str_contain_str(unsigned char *str, unsigned char *str2);
int abs_int(int int1);
void selection_sort(int *a, int len);
void replace_char(u8 *str, u8 ch1, u8 ch2);
void int_exchange(int *int1, int *int2);
float abs_float(float value);

void parse_action(u8 *uart_receive_buf);
void parse_cmd(u8 *cmd);
void save_action(u8 *str);

void do_group_once(int group_num);
void loop_action(void);
int getMaxTime(u8 *str);
int get_action_index(u8 *str);
int kinematics_move(float x, float y, float z, int time);
void print_group(int start, int end);
void rewrite_eeprom(void);
void set_servo(int index, int pwm, int time);
void loop_monitor(void);
void zx_uart_send_str(u8 *str);

void car_set(int car_left1, int car_right1, int car_left2, int car_right2);
void car_set1(int car_left1, int car_right1, int car_left2, int car_right2);
#endif
