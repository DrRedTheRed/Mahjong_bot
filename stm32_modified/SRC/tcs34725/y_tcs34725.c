/*
 * @文件描述:
 * @作者: Q
 * @Date: 2023-03-01 11:13:49
 * @LastEditTime: 2023-03-02 16:34:27
 */
#include "tcs34725/y_tcs34725.h"

COLOR_RGBC rgb;
COLOR_HSL hsl;

/**
 * @函数描述: 将数据写入从设备
 * @param {uint8_t} slaveAddress 从设备地址
 * @param {uint8_t} *dataBuffer 指向存储传输数据的缓冲区的指针
 * @param {uint8_t} bytesNumber 要写入的字节数
 * @param {uint8_t} stopBit 是否发送停止位
 * @return {*}
 */
void TCS34725_I2C_Write(uint8_t slaveAddress, uint8_t *dataBuffer, uint8_t bytesNumber, uint8_t stopBit)
{
	uint8_t i = 0;
	i2c_start();
	i2c_write_byte((slaveAddress << 1) | 0x00); // 发送从机地址写命令
	i2c_wait_ack();
	for (i = 0; i < bytesNumber; i++)
	{
		i2c_write_byte(*(dataBuffer + i));
		i2c_wait_ack();
	}
	if (stopBit == 1)
		i2c_stop();
}

/**
 * @函数描述: 从设备读取数据
 * @param {uint8_t} slaveAddress 从设备地址
 * @param {uint8_t} *dataBuffer 指向存储接收数据的缓冲区的指针
 * @param {uint8_t} bytesNumber 要读取的字节数
 * @param {uint8_t} stopBit 停止状态控制
 * @return {*}
 */
void TCS34725_I2C_Read(uint8_t slaveAddress, uint8_t *dataBuffer, uint8_t bytesNumber, uint8_t stopBit)
{
	uint8_t i = 0;

	i2c_start();
	i2c_write_byte((slaveAddress << 1) | 0x01); // 发送从机地址读命令
	i2c_wait_ack();
	for (i = 0; i < bytesNumber; i++)
	{
		if (i == bytesNumber - 1)
		{
			*(dataBuffer + i) = i2c_read_byte(0); // 读取的最后一个字节发送NACK
		}
		else
		{
			*(dataBuffer + i) = i2c_read_byte(1);
		}
	}
	if (stopBit == 1)
		i2c_stop();
}

/**
 * @函数描述: 从所选寄存器开始，将数据写入TCS34725寄存器寄存器地址指针
 * @param {uint8_t} subAddr 所选的寄存器地址指针
 * @param {uint8_t} *dataBuffer 指向存储传输数据的缓冲区的指针
 * @param {uint8_t} bytesNumber 将要发送的字节数
 * @return {*}
 */
void TCS34725_Write(uint8_t subAddr, uint8_t *dataBuffer, uint8_t bytesNumber)
{
	uint8_t sendBuffer[10] = {0};
	uint8_t byte = 0;

	sendBuffer[0] = subAddr | TCS34725_COMMAND_BIT;
	for (byte = 1; byte <= bytesNumber; byte++)
	{
		sendBuffer[byte] = dataBuffer[byte - 1];
	}
	TCS34725_I2C_Write(TCS34725_ADDRESS, sendBuffer, bytesNumber + 1, 1);
}

void TCS34725_Read(uint8_t subAddr, uint8_t *dataBuffer, uint8_t bytesNumber)
{
	subAddr |= TCS34725_COMMAND_BIT;
	TCS34725_I2C_Write(TCS34725_ADDRESS, (uint8_t *)&subAddr, 1, 0);
	TCS34725_I2C_Read(TCS34725_ADDRESS, dataBuffer, bytesNumber, 1);
}

/*******************************************************************************
 * @brief TCS34725设置积分时间
 *
 * @return None
 *******************************************************************************/
void TCS34725_SetIntegrationTime(uint8_t time)
{
	TCS34725_Write(TCS34725_ATIME, &time, 1);
}

/*******************************************************************************
 * @brief TCS34725设置增益
 *
 * @return None
 *******************************************************************************/
void TCS34725_SetGain(uint8_t gain)
{
	TCS34725_Write(TCS34725_CONTROL, &gain, 1);
}

/*******************************************************************************
 * @brief TCS34725使能
 * @return None
 *******************************************************************************/
void TCS34725_Enable(void)
{
	uint8_t cmd = TCS34725_ENABLE_PON;

	TCS34725_Write(TCS34725_ENABLE, &cmd, 1);
	cmd = TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN;
	TCS34725_Write(TCS34725_ENABLE, &cmd, 1);
	// delay_s(600000);//delay_ms(3);//延时应该放在设置AEN之后
}

/*******************************************************************************
 * @brief TCS34725失能
 * @return None
 *******************************************************************************/
void TCS34725_Disable(void)
{
	uint8_t cmd = 0;

	TCS34725_Read(TCS34725_ENABLE, &cmd, 1);
	cmd = cmd & ~(TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN);
	TCS34725_Write(TCS34725_ENABLE, &cmd, 1);
}

/*******************************************************************************
 * @brief TCS34725初始化
 * @return ID - ID寄存器中的值
 *******************************************************************************/
uint8_t TCS34725_Init(void)
{
	uint8_t id = 0;

	soft_i2c_gpio_init();
	TCS34725_Read(TCS34725_ID, &id, 1); // TCS34725 的 ID 是 0x44 可以根据这个来判断是否成功连接
	if (id == 0x44)
	{
		TCS34725_SetIntegrationTime(TCS34725_INTEGRATIONTIME_50MS);
		TCS34725_SetGain(TCS34725_GAIN_1X);
		TCS34725_Enable();
		return 1;
	}
	return 0;
}
/*******************************************************************************
 * @brief TCS34725获取单个通道数据
 * @return data - 该通道的转换值
 *******************************************************************************/
uint16_t TCS34725_GetChannelData(uint8_t reg)
{
	uint8_t tmp[2] = {0, 0};
	uint16_t data;

	TCS34725_Read(reg, tmp, 2);
	data = (tmp[1] << 8) | tmp[0];

	return data;
}
/*******************************************************************************
 * @brief TCS34725获取各个通道数据
 * @return 1 - 转换完成，数据可用
 *   	   0 - 转换未完成，数据不可用
 *******************************************************************************/
uint8_t TCS34725_GetRawData(COLOR_RGBC *rgbc)
{
	uint8_t status = TCS34725_STATUS_AVALID;

	TCS34725_Read(TCS34725_STATUS, &status, 1);

	if (status & TCS34725_STATUS_AVALID)
	{
		rgbc->c = TCS34725_GetChannelData(TCS34725_CDATAL);
		rgbc->r = TCS34725_GetChannelData(TCS34725_RDATAL);
		rgbc->g = TCS34725_GetChannelData(TCS34725_GDATAL);
		rgbc->b = TCS34725_GetChannelData(TCS34725_BDATAL);
		return 1;
	}
	return 0;
}

void TCS34725_LedON(uint8_t enable)
{
	uint8_t cmd = 0;
	TCS34725_Read(TCS34725_ENABLE, &cmd, 1);
	if (enable)
	{
		cmd |= TCS34725_ENABLE_AIEN;
	}
	else
	{
		cmd &= ~TCS34725_ENABLE_AIEN;
	}
	TCS34725_Write(TCS34725_ENABLE, &cmd, 1);
}

// RGB转HSL
void RGBtoHSL(COLOR_RGBC *Rgb, COLOR_HSL *Hsl)
{
	uint8_t maxVal, minVal, difVal;
	uint8_t r = Rgb->r * 100 / Rgb->c; //[0-100]
	uint8_t g = Rgb->g * 100 / Rgb->c;
	uint8_t b = Rgb->b * 100 / Rgb->c;

	maxVal = max3v(r, g, b);
	minVal = min3v(r, g, b);
	difVal = maxVal - minVal;

	// 计算亮度
	Hsl->l = (maxVal + minVal) / 2; //[0-100]

	if (maxVal == minVal) // 若r=g=b,灰度
	{
		Hsl->h = 0;
		Hsl->s = 0;
	}
	else
	{
		// 计算色调
		if (maxVal == r)
		{
			if (g >= b)
				Hsl->h = 60 * (g - b) / difVal;
			else
				Hsl->h = 60 * (g - b) / difVal + 360;
		}
		else
		{
			if (maxVal == g)
				Hsl->h = 60 * (b - r) / difVal + 120;
			else if (maxVal == b)
				Hsl->h = 60 * (r - g) / difVal + 240;
		}

		// 计算饱和度
		if (Hsl->l <= 50)
			Hsl->s = difVal * 100 / (maxVal + minVal); //[0-100]
		else
			Hsl->s = difVal * 100 / (200 - (maxVal + minVal));
	}
}
/******************************************************************************/
