#include "display.h"
#include "oled.h"
#include "LEDMatrix8x8.h"
#include "MPU6050.h"  
#include "MySPI.h"  
#include "stm32f10x.h"
#include <stdlib.h>

// 点阵屏图案
static const uint8_t arrow_up[8] = {0x18, 0x3C, 0x7E, 0xFF, 0x18, 0x18, 0x18, 0x18};
static const uint8_t arrow_down[8] = {0x18, 0x18, 0x18, 0x18, 0xFF, 0x7E, 0x3C, 0x18};
static const uint8_t arrow_left[8] = {0x08, 0x0C, 0x0E, 0xFF, 0xFF, 0x0E, 0x0C, 0x08};
static const uint8_t arrow_right[8] = {0x10, 0x30, 0x70, 0xFF, 0xFF, 0x70, 0x30, 0x10};
static const uint8_t icon_normal[8] = {0x3C, 0x42, 0x81, 0x81, 0x81, 0x81, 0x42, 0x3C};

// 当前显示模式
static DisplayMode_t currentDisplayMode = DISPLAY_MODE_SENSOR;

// MPU6050数据变量（只在display.c中使用）
static int16_t AX, AY, AZ, GX, GY, GZ;

/**
  * 函    数：显示初始化
  * 参    数：无
  * 返 回 值：无
  */
void Display_Init(void)
{	
    LEDMatrix_Init();          // 初始化点阵屏
    OLED_Clear();
    LEDMatrix_DisplayPattern(icon_normal);
}

/**
  * 函    数：MPU6050方向检测和更新
  * 参    数：Reg 寄存器数组指针
  * 返 回 值：无
  */
void Display_UpdateMPU6050Direction(uint16_t *Reg)
{
    // 读取MPU6050数据
    MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);
    
    // 更新加速度数据到寄存器
    Reg[6] = (AX >> 8) & 0xFF;
    Reg[7] = AX & 0xFF;
    Reg[8] = (AY >> 8) & 0xFF;
    Reg[9] = AY & 0xFF;
    Reg[10] = (AZ >> 8) & 0xFF;
    Reg[11] = AZ & 0xFF;
    
    // 修正方向检测逻辑（参考您的正确代码）
    if (AX > 1500) {           // 向右倾斜
        Display_UpdateLEDMatrix(4);  // 右箭头
        Reg[5] = 4;
        // OLED显示"RIGHT"
    }
    else if (AX < -1500) {     // 向左倾斜
        Display_UpdateLEDMatrix(3);  // 左箭头
        Reg[5] = 3;
        // OLED显示"LEFT"
    }
    else if (AY > 1500) {      // 向下倾斜
        Display_UpdateLEDMatrix(2);  // 下箭头
        Reg[5] = 2;
        // OLED显示"DOWN"
    }
    else if (AY < -1500) {     // 向上倾斜
        Display_UpdateLEDMatrix(1);  // 上箭头
        Reg[5] = 1;
        // OLED显示"UP"
    }
    else if (AX > 500) {       // 轻微向右
        Display_UpdateLEDMatrix(4);  // 右箭头
        Reg[5] = 4;
        // OLED显示"right"
    }
    else if (AX < -500) {      // 轻微向左
        Display_UpdateLEDMatrix(3);  // 左箭头
        Reg[5] = 3;
        // OLED显示"left"
    }
    else if (AY > 500) {       // 轻微向下
        Display_UpdateLEDMatrix(2);  // 下箭头
        Reg[5] = 2;
        // OLED显示"down"
    }
    else if (AY < -500) {      // 轻微向上
        Display_UpdateLEDMatrix(1);  // 上箭头
        Reg[5] = 1;
        // OLED显示"up"
    }
    else {                     // 接近水平
        Display_UpdateLEDMatrix(0);  // 正常图标
        Reg[5] = 0;
        // OLED显示"NORMAL"
    }
}

/**
  * 函    数：更新传感器数据显示
  * 参    数：Reg 寄存器数组指针
  * 返 回 值：无
  */
void Display_UpdateSensorData(uint16_t *Reg)
{
    if (currentDisplayMode != DISPLAY_MODE_SENSOR) return;
    
    OLED_ShowString(1, 1, "T:");
    OLED_ShowString(1, 5, ".");
    OLED_ShowString(1, 8, "C");
    OLED_ShowNum(1, 3, Reg[0] % 256, 2);  // 温度
    OLED_ShowNum(1, 6, Reg[1] % 256, 2);
    
    OLED_ShowString(2, 1, "H:");
    OLED_ShowString(2, 5, ".");
    OLED_ShowNum(2, 3, Reg[2] % 256, 2);  // 湿度  
    OLED_ShowNum(2, 6, Reg[3] % 256, 2);
    OLED_ShowString(2, 8, "%"); 
    
    OLED_ShowString(3, 1, "Light:");
    OLED_ShowNum(3, 7, Reg[4], 1);
    
    OLED_ShowString(4, 1, "Mode:Sensor");
}

/**
  * 函    数：更新MPU6050数据显示
  * 参    数：Reg 寄存器数组指针
  * 返 回 值：无
  */
void Display_UpdateMPU6050Data(uint16_t *Reg)
{
    if (currentDisplayMode != DISPLAY_MODE_MPU6050) return;
      
    // 第一行：方向状态（区分大小写）
    OLED_ShowString(1, 1, "Dir:");
    
    // 根据阈值判断是大动作还是小动作
    int16_t accel_x = (Reg[6] << 8) | Reg[7];
    int16_t accel_y = (Reg[8] << 8) | Reg[9];
    
    // 判断是否是大动作（绝对值>1500）
    uint8_t isBigAction = (abs(accel_x) > 1500) || (abs(accel_y) > 1500);
    
    switch (Reg[5]) {
        case 0: OLED_ShowString(1, 6, "Normal "); break;
        case 1: 
            if (isBigAction) OLED_ShowString(1, 6, "RIGHT  ");
            else OLED_ShowString(1, 6, "right  ");
            break;
        case 2: 
			if (isBigAction) OLED_ShowString(1, 6, "LEFT   ");
            else OLED_ShowString(1, 6, "left   ");
		break;
        case 3: 
            if (isBigAction) OLED_ShowString(1, 6, "DOWN   ");
            else OLED_ShowString(1, 6, "down   ");
            break;
        case 4: 
            if (isBigAction) OLED_ShowString(1, 6, "UP     ");
            else OLED_ShowString(1, 6, "up     ");
            break;
        default: OLED_ShowString(1, 6, "Unknown"); break;
    }
    
    // 第二行：X轴加速度
    OLED_ShowString(2, 1, "X:");
    OLED_ShowSignedNum(2, 4, accel_x, 5);
    
    // 第三行：Y轴加速度
    OLED_ShowString(3, 1, "Y:");
    OLED_ShowSignedNum(3, 4, accel_y, 5);
    
    // 第四行：Z轴加速度
    OLED_ShowString(4, 1, "Z:");
    int16_t accel_z = (Reg[10] << 8) | Reg[11];
    OLED_ShowSignedNum(4, 4, accel_z, 5);
}

/**
  * 函    数：更新点阵屏显示
  * 参    数：direction 方向状态
  * 返 回 值：无
  */
void Display_UpdateLEDMatrix(uint8_t direction)
{
	    // 每次更新显示时都设置亮度，防止进入休眠
    LEDMatrix_SetBrightness(0x08);  // 设置亮度
	
    switch (direction) 
		{
        case 0: LEDMatrix_DisplayPattern(icon_normal); break;  // 正常
        case 1: LEDMatrix_DisplayPattern(arrow_up); break;     // 上
        case 2: LEDMatrix_DisplayPattern(arrow_down); break;   // 下
        case 3: LEDMatrix_DisplayPattern(arrow_left); break;   // 左
        case 4: LEDMatrix_DisplayPattern(arrow_right); break;  // 右
        default: LEDMatrix_DisplayPattern(icon_normal); break;
    }
}

/**
  * 函    数：切换显示模式
  * 参    数：无
  * 返 回 值：无
  */
void Display_SwitchMode(void)
{
    currentDisplayMode = (currentDisplayMode + 1) % DISPLAY_MODE_MAX;
    OLED_Clear();  // 切换模式时清屏
}

/**
  * 函    数：获取当前显示模式
  * 参    数：无
  * 返 回 值：当前显示模式
  */
DisplayMode_t Display_GetCurrentMode(void)
{
    return currentDisplayMode;
}