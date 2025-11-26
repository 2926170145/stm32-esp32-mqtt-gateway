#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "stm32f10x.h"

// 显示模式枚举
typedef enum {
    DISPLAY_MODE_SENSOR = 0,    // 传感器数据显示模式
    DISPLAY_MODE_MPU6050 = 1,   // MPU6050数据显示模式
    DISPLAY_MODE_MAX            // 显示模式总数
} DisplayMode_t;

// 函数声明
void Display_Init(void);
void Display_UpdateSensorData(uint16_t *Reg);
void Display_UpdateMPU6050Data(uint16_t *Reg);
void Display_UpdateLEDMatrix(uint8_t direction);
void Display_SwitchMode(void);
DisplayMode_t Display_GetCurrentMode(void);
void Display_UpdateMPU6050Direction(uint16_t *Reg);  // ：MPU6050方向检测

#endif

