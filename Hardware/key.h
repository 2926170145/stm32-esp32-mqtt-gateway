#ifndef __KEY_H
#define __KEY_H
#include "stm32f10x.h"                  // Device header


// 声明外部变量
extern volatile uint32_t key_check_timer;
// 函数声明
void Key_Init(void);                    // 按键初始化
uint8_t Key_GetState(void);             // 获取按键状态：1-有按键按下，0-无按键

#endif


