#ifndef __IWDG_H
#define __IWDG_H

#include "stm32f10x.h"

void IWDG_Init(void);          // 初始化看门狗
void IWDG_Feed(void);          // 喂狗
void IWDG_ResetTimeout(void);  // 重置通信超时（给Modbus调用）
void IWDG_CheckTimeout(void);  // 检查超时（给定时器中断调用）

#endif