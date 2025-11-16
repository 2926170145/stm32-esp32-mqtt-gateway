#include "LEDMatrix8x8.h"

// MAX7219寄存器地址定义
#define MAX7219_REG_NOOP        0x00
#define MAX7219_REG_DIGIT0      0x01
#define MAX7219_REG_DIGIT1      0x02
#define MAX7219_REG_DIGIT2      0x03
#define MAX7219_REG_DIGIT3      0x04
#define MAX7219_REG_DIGIT4      0x05
#define MAX7219_REG_DIGIT5      0x06
#define MAX7219_REG_DIGIT6      0x07
#define MAX7219_REG_DIGIT7      0x08
#define MAX7219_REG_DECODEMODE  0x09
#define MAX7219_REG_INTENSITY   0x0A
#define MAX7219_REG_SCANLIMIT   0x0B
#define MAX7219_REG_SHUTDOWN    0x0C
#define MAX7219_REG_DISPLAYTEST 0x0F

/**
  * 函    数：向MAX7219写入数据
  * 参    数：address 寄存器地址
  * 参    数：data 要写入的数据
  * 返 回 值：无
  */
void LEDMatrix_WriteByte(uint8_t address, uint8_t data)
{
    MySPI_Start();              // 开始SPI通信
    MySPI_SwapByte(address);    // 发送寄存器地址
    MySPI_SwapByte(data);       // 发送数据
    MySPI_Stop();               // 结束SPI通信
}

/**
  * 函    数：点阵屏初始化
  * 参    数：无
  * 返 回 值：无
  */
void LEDMatrix_Init(void)
{
    
    // 初始化MAX7219寄存器
    LEDMatrix_WriteByte(MAX7219_REG_SCANLIMIT, 0x07);   // 扫描8行
    LEDMatrix_WriteByte(MAX7219_REG_DECODEMODE, 0x00);  // 不使用BCD解码
    LEDMatrix_WriteByte(MAX7219_REG_SHUTDOWN, 0x01);    // 正常模式（非关机）
    LEDMatrix_WriteByte(MAX7219_REG_DISPLAYTEST, 0x00); // 正常模式（非测试）
    
    LEDMatrix_SetBrightness(0x08);          // 设置中等亮度
    LEDMatrix_Clear();                      // 清屏
}

/**
  * 函    数：设置点阵屏亮度
  * 参    数：brightness 亮度值，范围0x00~0x0F
  * 返 回 值：无
  */
void LEDMatrix_SetBrightness(uint8_t brightness)
{
    if (brightness > 0x0F) brightness = 0x0F;  // 限制亮度范围
    LEDMatrix_WriteByte(MAX7219_REG_INTENSITY, brightness);
}

/**
  * 函    数：清空点阵屏显示
  * 参    数：无
  * 返 回 值：无
  */
void LEDMatrix_Clear(void)
{
    for (uint8_t i = 1; i <= 8; i++) {
        LEDMatrix_WriteByte(i, 0x00);       // 将所有行数据清零
    }
}

/**
  * 函    数：设置指定行的显示数据
  * 参    数：row 行号，范围1~8
  * 参    数：data 该行的显示数据，每个bit对应一个LED
  * 返 回 值：无
  */
void LEDMatrix_DisplayRow(uint8_t row, uint8_t data)
{
    if (row < 1 || row > 8) return;         // 检查行号范围
    LEDMatrix_WriteByte(row, data);
}

/**
  * 函    数：显示完整图案
  * 参    数：pattern 指向8字节数组的指针，每字节对应一行的显示数据
  * 返 回 值：无
  */
void LEDMatrix_DisplayPattern(const uint8_t *pattern)
{
    for (uint8_t i = 0; i < 8; i++) {
        LEDMatrix_WriteByte(i + 1, pattern[i]);  // 写入8行数据
    }
}