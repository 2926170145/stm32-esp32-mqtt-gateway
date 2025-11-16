#include "key.h"
#include "stm32f10x.h"

static uint8_t keyStableState = 1;      // 稳定的按键状态：1-弹起(高电平)，0-按下(低电平)
static uint8_t keyPressFlag = 0;        // 按键按下标志：1-有按键按下事件，0-无按键事件
extern volatile uint32_t key_check_timer;

/**
  * 函    数：按键初始化
  * 参    数：无
  * 返 回 值：无
  * 说    明：配置PA0为上拉输入模式，默认高电平，按下为低电平
  */
void Key_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 开启GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    // 配置PA11为上拉输入模式
    // 上拉模式：默认引脚为高电平(3.3V)
    // 当按键按下时，引脚被拉低到GND(0V)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;      // 上拉输入模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  // 输入模式此参数不影响
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/**
  * 函    数：获取按键状态
  * 参    数：无
  * 返 回 值：1-检测到按键按下事件，0-无按键事件
  * 说    明：非阻塞检测，每10ms执行一次按键检测
  */
uint8_t Key_GetState(void)
{
    // 每10ms检测一次按键，避免频繁检测消耗CPU
    if (key_check_timer < 10) {
        return 0;  // 时间未到，直接返回无按键
    }
    
    uint8_t currentState = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);  // 读取PA11当前电平状态
    uint8_t result = 0;  // 返回值，默认为无按键
    
    /******************************************************************
     * 关键逻辑解释：
     * 
     * 物理状态：
     * - 按键弹起：PA0 = 高电平(1)，因为上拉电阻拉到3.3V
     * - 按键按下：PA0 = 低电平(0)，因为按键接通GND
     * 
     * 变量说明：
     * - currentState：当前读取的物理电平状态
     * - keyStableState：上一次检测时的稳定状态
     * 
     * 条件判断逻辑：
     * if (currentState == 0 && keyStableState == 1)
     * 
     * 这个条件的意思是：
     * 1. currentState == 0  ：当前检测到低电平（按键被按下）
     * 2. keyStableState == 1：上一次的状态是高电平（按键之前是弹起的）
     * 
     * 两个条件同时满足，说明：
     * "按键从弹起状态变为按下状态" - 这是一个下降沿事件
     * 
     * 为什么要这样判断？
     * - 如果只判断 currentState == 0，那么只要按键按着就会一直触发
     * - 通过记录前一次状态，我们只在新按下时触发一次
     * - 这叫做"边沿检测"，只检测状态变化的那一瞬间
     ******************************************************************/
    if (currentState == 0 && keyStableState == 1) {
        // 检测到下降沿：从高电平(弹起)变为低电平(按下)
        // 设置按键按下标志，表示有一个新的按键按下事件
        keyPressFlag = 1;
    }
    
    // 更新稳定状态为当前状态，供下一次检测使用
    keyStableState = currentState;
    
    // 检查是否有按键按下事件需要处理
    if (keyPressFlag) {
        result = 1;        // 返回有按键事件
        keyPressFlag = 0;  // 清除标志，确保只返回一次
    }
    
    key_check_timer = 0;  // 重置计时器，开始下一个10ms周期
    return result;
}


