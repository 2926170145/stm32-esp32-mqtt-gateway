#include "stm32f10x.h"                  // Device header
#include "iwdg.h"

static volatile uint32_t s_timeoutCounter = 0;

// ��ʼ���������Ź�
void IWDG_Init(void)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_64);    // 4�볬ʱ
    IWDG_SetReload(2499);
    IWDG_ReloadCounter();
    IWDG_Enable();
}

// ι��
void IWDG_Feed(void)
{
    IWDG_ReloadCounter();
}

// ����ͨ�ų�ʱ��Modbus�յ�����ʱ���ã�
void IWDG_ResetTimeout(void)
{
    s_timeoutCounter = 0;
}

// ���ͨ�ų�ʱ����1ms��ʱ���ж��е��ã�
void IWDG_CheckTimeout(void)
{
    if(s_timeoutCounter++ > 10000) {  // 10�볬ʱ
        NVIC_SystemReset();
    }
}