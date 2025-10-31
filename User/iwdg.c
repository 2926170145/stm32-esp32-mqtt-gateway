#include "stm32f10x.h"                  // Device header
#include "iwdg.h"

static volatile uint32_t s_timeoutCounter = 0;

// 初始化独立看门狗
void IWDG_Init(void)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_64);    // 4秒超时
    IWDG_SetReload(2499);
    IWDG_ReloadCounter();
    IWDG_Enable();
}

// 喂狗
void IWDG_Feed(void)
{
    IWDG_ReloadCounter();
}

// 重置通信超时（Modbus收到数据时调用）
void IWDG_ResetTimeout(void)
{
    s_timeoutCounter = 0;
}

// 检查通信超时（在1ms定时器中断中调用）
void IWDG_CheckTimeout(void)
{
    if(s_timeoutCounter++ > 10000) {  // 10秒超时
        NVIC_SystemReset();
    }
}