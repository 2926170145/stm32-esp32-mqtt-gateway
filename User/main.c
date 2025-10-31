#include "timer.h"
#include "modbus-uart.h"
#include "modbus.h"
#include "DHT11.h"
#include "iwdg.h"

u16 Reg[]={0x0000,   //本设备寄存器中的值
           0x0001,
           0x0002,
           0x0003,
           0x0004,
           0x0005,
           0x0006,
           0x0007,
           0x0008,
           0x0009,
           0x000A,	
          };	


void delay(u32 x)
{

 while(x--);
}
void Isr_Init()
{
	NVIC_InitTypeDef  isr;
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1); //  a bbb 
	
	isr.NVIC_IRQChannel=TIM2_IRQn;
	isr.NVIC_IRQChannelCmd=ENABLE;
	isr.NVIC_IRQChannelPreemptionPriority=1;
	isr.NVIC_IRQChannelSubPriority=2;	
	NVIC_Init(&isr);   //	
	
	isr.NVIC_IRQChannel=USART2_IRQn;
	isr.NVIC_IRQChannelCmd=ENABLE;
	isr.NVIC_IRQChannelPreemptionPriority=1;
	isr.NVIC_IRQChannelSubPriority=0;	
	NVIC_Init(&isr);   //
	
		
}



void TIM2_IRQHandler()//定时器2的中断服务子函数  1ms一次中断
{
  u8 st;
  st= TIM_GetFlagStatus(TIM2, TIM_FLAG_Update);	
	if(st==SET)
	{
	  TIM_ClearFlag(TIM2, TIM_FLAG_Update);
		if(modbus.timrun!=0)
		{
		  modbus.timout++; 
		  if(modbus.timout>=90)  //间隔时间达到了时间
			{
				modbus.timrun=0;//关闭定时器--停止定时
				modbus.reflag=1;  //收到一帧数据
			}
		}  
			IWDG_CheckTimeout();
	}	
}



int main()
{
  Timer2_Init();  
  Mosbus_Init();
  DH11_GPIO_Init(); // DHT11初始化
  IWDG_Init();  // 初始化看门狗
  Isr_Init();
	
  while(1)
	{	
		Mosbus_Event();  // Modbus正常通信
		IWDG_Feed();  // 主循环喂狗
        
        // 每2秒更新一次DHT11数据
        static u32 count = 0;
      // 尝试更大的数值
if(count++ >= 2000000) {  // 增加到200万，约20秒
    DHT11ReadData((uint8_t*)&Reg[0], (uint8_t*)&Reg[1], 
                 (uint8_t*)&Reg[2], (uint8_t*)&Reg[3]);
    count = 0;
}
}
}