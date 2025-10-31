#include "DHT11.h"
#include "delay.h"
#include "stm32f10x.h"                  // Device header



/**
	* @brief  初始化DHT11
	* @param  无
	* @retval 无
	*/
void DH11_GPIO_Init(void)
{
	// 开启APB2中的GPIOB时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	// 配置GPIO PB13
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD; // 开漏输出
	GPIO_InitStructure.GPIO_Pin   = DHT11_GPIO_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(DHT11_GPIO_Port, &GPIO_InitStructure);
}



/**
	* @brief  复位和检测响应
	* @param  无
	* @retval 1:响应成功, 0:响应失败
	*/
uint8_t DHT11RstAndCheck(void)
{
	uint8_t timer = 0;		// 计数器

	__set_PRIMASK(1);     // 关总中断
	dht11_low; 	          // 输出低电平
	Delay_ms(20);    	    // 拉低至少18ms
	dht11_high; 	        // 输出高电平
	Delay_us(30);     	  // 拉高20~40us
	
	// 检测是否存在第一个低电平
	while (!DHT11_IN)     // 等待总线拉低，DHT11会拉低40~80us作为响应信号
	{
		timer++;            // 总线拉低时计数
		Delay_us(1);
	}
	if (timer>100 || timer<20) // 判断响应时间
	{
		__set_PRIMASK(0);      // 开总中断
		return 0;
	}

	// 检测是否存在第二个高电平
	timer = 0;	// 重置计数器
	while (DHT11_IN)          // 等待DHT11释放总线，持续时间40~80us
	{
		timer++;               	// 总线拉高时计数
		Delay_us(1);
	}
	__set_PRIMASK(0);          // 开总中断
	if (timer>100 || timer<20) // 检测响应信号之后的高电平
	{
		return 0;
	}

	// 均存在, 则返回1, 响应正常
	return 1;
}



/**
  * @brief  读取一字节数据
	* @param  无
	* @retval 读到的数据
	*/
uint8_t DHT11ReadByte(void)
{
	uint8_t i;
	uint8_t byt = 0;

	__set_PRIMASK(1);      // 关总中断
	for (i=0; i<8; i++)
	{
		while (DHT11_IN);  // 等待低电平，数据位前都有50us低电平时隙

		while (!DHT11_IN); // 等待高电平，开始传输数据位

		Delay_us(40);
		byt <<= 1;         // 因高位在前，所以左移byt，最低位补0
		if (DHT11_IN)      // 将总线电平值读取到byt最低位中
		{
			byt |= 0x01;
		}
	}
	__set_PRIMASK(0);      // 开总中断

	return byt;
}



/**
  * @brief  读取数据
	* @param  Temp_H 温度整数部分
	* @param  Temp_L 温度小数部分
  * @param  Humi_H 湿度整数部分
  * @param  Humi_L 湿度小数部分
	* @retval 0-成功，1-失败
	*/
uint8_t DHT11ReadData(uint8_t *Temp_H,uint8_t *Temp_L,uint8_t *Humi_H,uint8_t *Humi_L)
{
	uint8_t sta = 0;
	uint8_t i;
	uint8_t buf[5];

	if (DHT11RstAndCheck())       // 检测响应信号
	{
		for(i=0;i<5;i++)            // 读取40位数据
		{
			buf[i]=DHT11ReadByte(); 	// 读取1字节数据
		}
		if(buf[0]+buf[1]+buf[2]+buf[3] == buf[4]) // 校验成功
		{
			*Humi_H = buf[0]; // 湿度 整数部分数据
			*Humi_L = buf[1]; // 湿度 小数部分数据
			*Temp_H = buf[2]; // 温度 整数部分数据
			*Temp_L = buf[3]; // 温度 小数部分数据
		}
		sta = 0;
	}
	else // 响应失败返回-1
	{
		*Temp_H = 0;
		*Temp_L = 0;
		*Humi_H = 0;
		*Humi_L = 0;
		sta = 1;
	}

	return sta;    
}