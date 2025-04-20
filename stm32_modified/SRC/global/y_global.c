#include "./global/y_global.h"
#include "stm32f10x_conf.h"

u8 cmd_return[CMD_RETURN_SIZE];
eeprom_info_t eeprom_info;
u8 AI_mode = 255;
u8 group_do_ok = 1;

int do_start_index;  // 动作组执行 起始序号
int do_time;         // 动作组执行 执行次数
int group_num_start; // 动作组执行 起始序号
int group_num_end;   // 动作组执行 终止序号
int group_num_times; // 动作组执行 起始变量

u8 needSaveFlag = 0;         // 偏差保存标志
u32 bias_systick_ms_bak = 0; // 偏差保存标志时间
u32 action_time = 0;

void zx_uart_send_str(u8 *str)
{
    uart1_get_ok = 1;
    uart3_send_str(str);
    uart1_send_str(str);
    uart1_get_ok = 0;
}

uint16_t str_contain_str(unsigned char *str, unsigned char *str2)
{
    unsigned char *str_temp, *str_temp2;
    str_temp = str;
    str_temp2 = str2;
    while (*str_temp)
    {
        if (*str_temp == *str_temp2)
        {
            while (*str_temp2)
            {
                if (*str_temp++ != *str_temp2++)
                {
                    str_temp = str_temp - (str_temp2 - str2) + 1;
                    str_temp2 = str2;
                    break;
                }
            }
            if (!*str_temp2)
            {
                return (str_temp - str);
            }
        }
        else
        {
            str_temp++;
        }
    }
    return 0;
}

void selection_sort(int *a, int len)
{
    int i, j, mi, t;
    for (i = 0; i < len - 1; i++)
    {
        mi = i;
        for (j = i + 1; j < len; j++)
        {
            if (a[mi] > a[j])
            {
                mi = j;
            }
        }

        if (mi != i)
        {
            t = a[mi];
            a[mi] = a[i];
            a[i] = t;
        }
    }
}

// int型 取绝对值函数
int abs_int(int int1)
{
    if (int1 > 0)
        return int1;
    return (-int1);
}

float abs_float(float value)
{
    if (value > 0)
    {
        return value;
    }
    return (-value);
}

// 字符串中的字符替代函数 把str字符串中所有的ch1换成ch2
void replace_char(u8 *str, u8 ch1, u8 ch2)
{
    while (*str)
    {
        if (*str == ch1)
        {
            *str = ch2;
        }
        str++;
    }
    return;
}

// 两个int变量交换
void int_exchange(int *int1, int *int2)
{
    int int_temp;
    int_temp = *int1;
    *int1 = *int2;
    *int2 = int_temp;
}

// 小车控制函数
// 参数 左轮速度和右轮速度 范围 -1000 到 1000
void car_set(int car_left1, int car_right1, int car_left2, int car_right2)
{
    if (car_left1 > 999)
    {
        car_left1 = 999;
    }
    else if (car_left1 < -999)
    {
        car_left1 = -999;
    }

    if (car_right1 > 999)
    {
        car_right1 = 999;
    }
    else if (car_right1 < -999)
    {
        car_right1 = -999;
    }

    /*************/
    if (car_left2 > 999)
    {
        car_left2 = 999;
    }
    else if (car_left2 < -999)
    {
        car_left2 = -999;
    }

    if (car_right2 > 999)
    {
        car_right2 = 999;
    }
    else if (car_right2 < -999)
    {
        car_right2 = -999;
    }

    // 总线马达设置
    sprintf((char *)cmd_return, "{#006P%04dT0000!#007P%04dT0000!#008P%04dT0000!#009P%04dT0000!}",
            (int)(1500 + car_left1), (int)(1500 - car_right1), (int)(1500 + car_left2), (int)(1500 - car_right2));
    zx_uart_send_str(cmd_return);
    return;
}

void car_set1(int car_left1, int car_right1, int car_left2, int car_right2)
{
    // 总线马达设置
    sprintf((char *)cmd_return, "{#006P%04dT1500!#007P%04dT1500!#008P%04dT1500!#009P%04dT1500!}",
            (int)(1500 + car_left1), (int)(1500 - car_right1), (int)(1500 + car_left2), (int)(1500 - car_right2));
    zx_uart_send_str(cmd_return);
    return;
}

// 动作组保存函数
// 只有用<>包含的字符串才能在此函数中进行解析
void save_action(u8 *str)
{
    s32 action_index = 0;
    // 预存命令处理
    spiFlahsOn(1);
    delay_ms(10);

    if (str[1] == '$' && str[2] == '!')
    {
        eeprom_info.pre_cmd[PRE_CMD_SIZE] = 0;
        rewrite_eeprom();
        uart1_send_str((u8 *)"@CLEAR PRE_CMD OK!");
        return;
    }
    else if (str[1] == '$')
    {
        memset(eeprom_info.pre_cmd, 0, sizeof(eeprom_info.pre_cmd));
        strcpy((char *)eeprom_info.pre_cmd, (char *)str + 1); // 对字符串进行复制
        eeprom_info.pre_cmd[strlen((char *)str) - 2] = '\0';  // 赋值字符0
        eeprom_info.pre_cmd[PRE_CMD_SIZE] = FLAG_VERIFY;
        rewrite_eeprom();
        zx_uart_send_str((u8 *)"@SET PRE_CMD OK!");
        zx_uart_send_str((u8 *)eeprom_info.pre_cmd); // 打印存储进去的指令
        return;
    }

    // 获取动作的组号如果不正确，或是第6个字符不是#则认为字符串错误
    action_index = (str[2] - '0') * 1000 + (str[3] - '0') * 100 + (str[4] - '0') * 10 + (str[5] - '0');

    //<G0000#000P1500T1000!>
    if ((action_index == -1) || str[6] != '#')
    {
        uart1_send_str((u8 *)"E");
        return;
    }

    if ((action_index * ACTION_SIZE % 4096) == 0)
    {
        w25x_erase_sector(action_index * ACTION_SIZE / 4096);
    }
    // 把尖括号替换成大括号直接存储到存储芯片里面去，则在执行动作组的时候直接拿出来解析就可以了
    replace_char(str, '<', '{');
    replace_char(str, '>', '}');

    w25x_write(str, action_index * ACTION_SIZE, strlen((char *)str) + 1);

    // memset(str, 0, sizeof((char *)str));
    // w25x_read(str, action_index * ACTION_SIZE, ACTION_SIZE);
    // uart1_send_str(str);

    // 反馈一个A告诉上位机我已经接收到了
    uart1_send_str((u8 *)"A");
    uart3_send_str((u8 *)"A");

    spiFlahsOn(0);
    return;
}

// 处理 #000P1500T1000! 类似的字符串
void parse_action(u8 *uart_receive_buf)
{
    u16 index, time, i = 0;
    int bias, len;
    float pwm;
    zx_uart_send_str(uart_receive_buf);

    if (uart_receive_buf[0] == '#' && uart_receive_buf[4] == 'P' && uart_receive_buf[5] == 'S' && uart_receive_buf[6] == 'C' && uart_receive_buf[7] == 'K' && uart_receive_buf[12] == '!')
    {
        index = (uart_receive_buf[1] - '0') * 100 + (uart_receive_buf[2] - '0') * 10 + (uart_receive_buf[3] - '0');
        bias = (uart_receive_buf[9] - '0') * 100 + (uart_receive_buf[10] - '0') * 10 + (uart_receive_buf[11] - '0');
        if ((bias >= -500) && (bias <= 500) && (index < DJ_NUM))
        {
            if (uart_receive_buf[8] == '+')
            {
                duoji_doing[index].cur = duoji_doing[index].cur - eeprom_info.dj_bias_pwm[index] + bias;
                eeprom_info.dj_bias_pwm[index] = bias;
            }
            else if (uart_receive_buf[8] == '-')
            {
                duoji_doing[index].cur = duoji_doing[index].cur - eeprom_info.dj_bias_pwm[index] - bias;
                eeprom_info.dj_bias_pwm[index] = -bias;
            }
            duoji_doing[index].aim = duoji_doing[index].cur;
            duoji_doing[index].inc = 0.001;
            rewrite_eeprom();
        }
    }
    else if (uart_receive_buf[0] == '#' && uart_receive_buf[4] == 'P' && uart_receive_buf[5] == 'D' && uart_receive_buf[6] == 'S' && uart_receive_buf[7] == 'T' && uart_receive_buf[8] == '!')
    {
        index = (uart_receive_buf[1] - '0') * 100 + (uart_receive_buf[2] - '0') * 10 + (uart_receive_buf[3] - '0');
        if (index < DJ_NUM)
        {
            duoji_doing[index].inc = 0;
            duoji_doing[index].aim = duoji_doing[index].cur;
        }
        else if (index == 255)
        {
            for (index = 0; index < DJ_NUM; index++)
            {
                duoji_doing[index].inc = 0;
                duoji_doing[index].aim = duoji_doing[index].cur;
            }
        }
        return;
    }

    len = strlen((char *)uart_receive_buf); // 获取串口接收数据的长度
    while (uart_receive_buf[i] && (len >= i))
    {
        if (uart_receive_buf[i] == '#')
        {
            index = 0;
            i++;
            while (uart_receive_buf[i] && uart_receive_buf[i] != 'P')
            {
                index = index * 10 + uart_receive_buf[i] - '0';
                i++;
            }
        }
        else if (uart_receive_buf[i] == 'P')
        {
            pwm = 0;
            i++;
            while (uart_receive_buf[i] && uart_receive_buf[i] != 'T')
            {
                pwm = pwm * 10 + uart_receive_buf[i] - '0';
                i++;
            }
        }
        else if (uart_receive_buf[i] == 'T')
        {
            time = 0;
            i++;
            while (uart_receive_buf[i] && uart_receive_buf[i] != '!')
            {
                time = time * 10 + uart_receive_buf[i] - '0';
                i++;
            }

            pwm += eeprom_info.dj_bias_pwm[index];
            duoji_doing_set(index, pwm, time);
        }
        else
        {
            i++;
        }
    }
}

/*
    所有舵机停止命令：    $DST!
    第x个舵机停止命令：   $DST:x!
    单片机重启命令：$RST!
    检查动作组x到y组命令：$CGP:x-y!
    执行第x个动作：       $DGS:x!
    执行第x到y组动作z次： $DGT:x-y,z!
    小车左x、右轮y速度:   $DCR:x,y!
    所有舵机复位命令：    $DJR!
    获取应答信号：        $GETA!
    获取智能信号：        $SMODE1!
    $KINEMATICS:x,y,z,time! //坐标单位mm，时间单位ms

*/
// 命令解析函数
void parse_cmd(u8 *cmd)
{
    int pos, i, index, int1, int2, int3, int4;

    // uart1_send_str(cmd);

    if (pos = str_contain_str(cmd, (u8 *)"$DRS!"), pos)
    {
        uart1_send_str((u8 *)"hello word!");
    }
    else if (pos = str_contain_str(cmd, (u8 *)"$DST!"), pos)
    {
        group_do_ok = 1;
        for (i = 0; i < DJ_NUM; i++)
        {
            duoji_doing[i].inc = 0;
            duoji_doing[i].aim = duoji_doing[i].cur;
        }
        zx_uart_send_str((u8 *)"#255PDST!"); // 总线停止
        car_set(0, 0, 0, 0);                 // 车停
        AI_mode = 255;
    }
    else if (pos = str_contain_str(cmd, (u8 *)"$DST:"), pos)
    {
        if (sscanf((char *)cmd, "$DST:%d!", &index))
        {
            duoji_doing[index].inc = 0;
            duoji_doing[index].aim = duoji_doing[index].cur;
            sprintf((char *)cmd_return, "#%03dPDST!\r\n", (int)index);
            zx_uart_send_str(cmd_return);
            memset(cmd_return, 0, sizeof(cmd_return));
        }
    }
    else if (pos = str_contain_str(cmd, (u8 *)"$RST!"), pos)
    {
        soft_reset();
    }
    else if (pos = str_contain_str(cmd, (u8 *)"$PTG:"), pos)
    {
        if (sscanf((char *)cmd, "$PTG:%d-%d!", &int1, &int2))
        {
            print_group(int1, int2);
        }
    }
    else if (pos = str_contain_str(cmd, (u8 *)"$DGS:"), pos)
    {
        if (sscanf((char *)cmd, "$DGS:%d!", &int1))
        {
            group_do_ok = 1;
            do_group_once(int1);
        }
    }
    else if (pos = str_contain_str(cmd, (u8 *)"$DGT:"), pos)
    {
        if (sscanf((char *)cmd, "$DGT:%d-%d,%d!", &group_num_start, &group_num_end, &group_num_times))
        {
            group_do_ok = 1;
            if (group_num_start != group_num_end)
            {
                do_start_index = group_num_start;
                do_time = group_num_times;
                group_do_ok = 0;
            }
            else
            {
                do_group_once(group_num_start);
            }
        }
    }
    else if (pos = str_contain_str(cmd, (u8 *)"$DCR:"), pos)
    {
        if (sscanf((char *)cmd, "$DCR:%d,%d,%d,%d!", &int1, &int2, &int3, &int4))
        {
            car_set(int1, int2, int3, int4);
        }
    }
    else if (pos = str_contain_str(cmd, (u8 *)"$DJR!"), pos)
    {
        zx_uart_send_str((u8 *)"#255P1500T2000!");
        car_set(0, 0, 0, 0);
        AI_mode = 255;
        for (i = 0; i < DJ_NUM; i++)
        {
            duoji_doing[i].aim = 1500 + eeprom_info.dj_bias_pwm[i];
            duoji_doing[i].time = 2000;
            duoji_doing[i].inc = (duoji_doing[i].aim - duoji_doing[i].cur) / (duoji_doing[i].time / 20.000);
        }
    }
    else if (pos = str_contain_str(cmd, (u8 *)"$TZ!"), pos)
    {
        // 停止
				beep_on_times(1, 100);
        AI_mode = 255;
    }
		 else if (pos = str_contain_str(cmd, (u8 *)"YSSB!"), pos)
    {
        beep_on_times(1, 100);
        // 颜色识别
        AI_mode = 1;
    }
    else if (pos = str_contain_str(cmd, (u8 *)"DJJQ!"), pos)
    {
        beep_on_times(1, 100);
        // 定距抓取
        AI_mode = 2;
    }
		else if (pos = str_contain_str(cmd, (u8 *)"RGBD!"), pos)
    {
			static uint8_t color_num = 0;
        beep_on_times(1, 100);
        // RGB彩灯
        ws2812b_test(color_num);
        color_num++;
        if (color_num > 7)
            color_num = 0;
    }
    else if (pos = str_contain_str(cmd, (u8 *)"SKJQ!"), pos)
    {
        beep_on_times(1, 100);
        // 声控抓取
        AI_mode = 4;
    }
    else if (pos = str_contain_str(cmd, (u8 *)"HWCF!"), pos)
    {
        beep_on_times(1, 100);
        // 红外触发
        AI_mode = 5;
    }
    else if (pos = str_contain_str(cmd, (u8 *)"CMJQ!"), pos)
    {
        beep_on_times(1, 100);
        // 触摸夹取
        AI_mode = 6;
    }
    else if (pos = str_contain_str(cmd, (u8 *)"$GETA!"), pos)
    {
        uart1_send_str((u8 *)"AAA");
    }
    else if (pos = str_contain_str(cmd, (u8 *)"$SMODE"), pos)
    {
        if (sscanf((char *)uart_receive_buf, "$SMODE%d!", &int1))
        {
            AI_mode = int1;
            if (int1 == 0)
                zx_uart_send_str((u8 *)"#010P1512T0000!");
            if (int1 == 2)
                zx_uart_send_str((u8 *)"#010P1511T0000!");

            // uart1_send_str((u8 *)"@OK!");
            beep_on_times(1, 100);
        }
    }
    else if (pos = str_contain_str(cmd, (u8 *)"$SMART_STOP!"), pos)
    {
        AI_mode = 255;
        mdelay(10);
        parse_action((u8 *)"#255PDST!");
        mdelay(10);
        uart1_send_str((u8 *)"#006P1500T0000!#007P1500T0000!");
        mdelay(10);
        uart1_send_str((u8 *)"@OK!");
        mdelay(10);
        beep_on_times(1, 100);
    }
    else if (pos = str_contain_str(cmd, (u8 *)"$KMS:"), pos)
    {
        if (sscanf((char *)cmd, "$KMS:%d,%d,%d,%d!", &int1, &int2, &int3, &int4))
        {
            uart1_send_str((u8 *)"Try to find best pos:\r\n");
            if (kinematics_move(int1, int2, int3, int4))
            {
                //beep_on();
                //mdelay(50);
                //beep_off();
                //mdelay(50);
            }
            else
            {
                //beep_on();
                //mdelay(50);
                //beep_off();
                //mdelay(50);
                //beep_on();
                //mdelay(50);
                //beep_off();
                //mdelay(50);
                uart1_send_str((u8 *)"Can't find best pos!!!");
            }
        }
    }
}

// 执行动作组1次
// 参数是动作组序号
void do_group_once(int group_num)
{
    spiFlahsOn(1);
    delay_ms(10);
    // 将uart_receive_buf清零
    memset(uart_receive_buf, 0, sizeof(uart_receive_buf));
    // 从存储芯片中读取第group_num个动作组
    w25x_read(uart_receive_buf, group_num * ACTION_SIZE, ACTION_SIZE);
    // 获取最大的组时间
    action_time = getMaxTime(uart_receive_buf);

    // 把读取出来的动作组传递到parse_action执行
    parse_action(uart_receive_buf);
    spiFlahsOn(0);
}

// 动作组批量执行
void loop_action(void)
{
    // 通过判断舵机是否全部执行完毕 并且是执行动作组group_do_ok尚未结束的情况下进入处理
    static long long systick_ms_bak = 0;
    if (group_do_ok == 0)
    {
        if (millis() - systick_ms_bak > action_time)
        {
            systick_ms_bak = millis();
            if (group_num_times != 0 && do_time == 0)
            {
                group_do_ok = 1;
                uart1_send_str((u8 *)"@GroupDone!");
                return;
            }
            // 调用do_start_index个动作
            do_group_once(do_start_index);

            if (group_num_start < group_num_end)
            {
                if (do_start_index == group_num_end)
                {
                    do_start_index = group_num_start;
                    if (group_num_times != 0)
                    {
                        do_time--;
                    }
                    return;
                }
                do_start_index++;
            }
            else
            {
                if (do_start_index == group_num_end)
                {
                    do_start_index = group_num_start;
                    if (group_num_times != 0)
                    {
                        do_time--;
                    }
                    return;
                }
                do_start_index--;
            }
        }
    }
    else
    {
        action_time = 10;
    }
}

// 获取最大时间
int getMaxTime(u8 *str)
{
    int i = 0, max_time = 0, tmp_time = 0;
    while (str[i])
    {
        if (str[i] == 'T')
        {
            tmp_time = (str[i + 1] - '0') * 1000 + (str[i + 2] - '0') * 100 + (str[i + 3] - '0') * 10 + (str[i + 4] - '0');
            if (tmp_time > max_time)
                max_time = tmp_time;
            i = i + 4;
            continue;
        }
        i++;
    }
    return max_time;
}

// 获取动作组的组号，字符串中有组号返回组号，否则返回-1
int get_action_index(u8 *str)
{
    u16 index = 0;
    // uart_send_str(str);
    while (*str)
    {
        if (*str == 'G')
        {
            str++;
            while ((*str != '#') && (*str != '$'))
            {
                index = index * 10 + *str - '0';
                str++;
            }
            return index;
        }
        else
        {
            str++;
        }
    }
    return -1;
}

int kinematics_move(float x, float y, float z, int time)
{
    int i, j, min = 0, flag = 0;

    if (y < 0)
        return 0;

    // 寻找最佳角度
    flag = 0;
    for (i = 0; i >= -135; i--)
    {
        if (0 == kinematics_analysis(x, y, z, i, &kinematics))
        {
            if (i < min)
                min = i;
            flag = 1;
        }
    }

    // 用3号舵机与水平最大的夹角作为最佳值
    if (flag)
    {
        kinematics_analysis(x, y, z, min, &kinematics);
        for (j = 0; j < 4; j++)
        {
            set_servo(j, kinematics.servo_pwm[j], time);
        }
        return 1;
    }

    return 0;
}

// 打印存储在芯片里的动作组，从串口1中发送出来 $CGP:x-y!这个命令调用
void print_group(int start, int end)
{
    spiFlahsOn(1);
    delay_ms(10);

    if (start > end)
    {
        int_exchange(&start, &end);
    }

    for (; start <= end; start++)
    {
        memset(uart_receive_buf, 0, sizeof(uart_receive_buf));
        w25x_read(uart_receive_buf, start * ACTION_SIZE, ACTION_SIZE);
        uart1_send_str(uart_receive_buf);
        uart1_send_str((u8 *)"\r\n");
    }

    spiFlahsOn(0);
}

// 把eeprom_info写入到W25Q64_INFO_ADDR_SAVE_STR位置
void rewrite_eeprom(void)
{
    spiFlahsOn(1);
    delay_ms(10);

    w25x_erase_sector(W25Q64_INFO_ADDR_SAVE_STR / 4096);
    w25x_writeS((u8 *)(&eeprom_info), W25Q64_INFO_ADDR_SAVE_STR, sizeof(eeprom_info_t));
    spiFlahsOn(0);
}

void set_servo(int index, int pwm, int time)
{
    duoji_doing[index].aim = pwm;
    duoji_doing[index].time = time;
    duoji_doing[index].inc = (duoji_doing[index].aim - duoji_doing[index].cur) / (duoji_doing[index].time / 20.000);
    sprintf((char *)cmd_return, "#%03dP%04dT%04d!", index, pwm, time);
    parse_action(cmd_return);
}

// 定时保存一些变量
void loop_monitor(void)
{
    static u32 saveTime = 3000;
    if ((needSaveFlag == 1) || (millis() - bias_systick_ms_bak > saveTime))
    {
        needSaveFlag = 0;
        bias_systick_ms_bak = millis();
        rewrite_eeprom();

        // 执行预存命令 {G0000#000P1500T1000!#000P1500T1000!}
        if (eeprom_info.pre_cmd[PRE_CMD_SIZE] == FLAG_VERIFY)
        {
        }
        else
        {
            beep_on();
            while (1)
            {
                /* code */
            }
        }
    }
    return;
}
