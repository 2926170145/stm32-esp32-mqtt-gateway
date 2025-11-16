#ifndef __DHT11_H
#define __DHT11_H
#include "stm32f10x.h"                  // Device header

// 定义引脚
#define DHT11_GPIO_Port GPIOB
#define DHT11_GPIO_Pin GPIO_Pin_15

// 定义函数
#define dht11_high GPIO_SetBits(DHT11_GPIO_Port, DHT11_GPIO_Pin)
#define dht11_low GPIO_ResetBits(DHT11_GPIO_Port, DHT11_GPIO_Pin)
#define DHT11_IN GPIO_ReadInputDataBit(DHT11_GPIO_Port, DHT11_GPIO_Pin)

void DH11_GPIO_Init(void);
uint8_t DHT11RstAndCheck(void);
uint8_t DHT11ReadByte(void);
uint8_t DHT11ReadData(uint8_t *Temp_H,uint8_t *Temp_L,uint8_t *Humi_H,uint8_t *Humi_L);

#endif
