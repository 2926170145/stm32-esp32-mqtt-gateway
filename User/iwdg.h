#ifndef __IWDG_H
#define __IWDG_H

#include "stm32f10x.h"

void IWDG_Init(void);          // ��ʼ�����Ź�
void IWDG_Feed(void);          // ι��
void IWDG_ResetTimeout(void);  // ����ͨ�ų�ʱ����Modbus���ã�
void IWDG_CheckTimeout(void);  // ��鳬ʱ������ʱ���жϵ��ã�

#endif