#ifndef __LEDMATRIX8X8_H
#define __LEDMATRIX8X8_H

#include "stm32f10x.h"

// º¯ÊýÉùÃ÷
void LEDMatrix_Init(void);
void LEDMatrix_WriteByte(uint8_t address, uint8_t data);
void LEDMatrix_Clear(void);
void LEDMatrix_SetBrightness(uint8_t brightness);
void LEDMatrix_DisplayRow(uint8_t row, uint8_t data);
void LEDMatrix_DisplayPattern(const uint8_t *pattern);

#endif