#include "timer.h"
#include "modbus-uart.h"
#include "modbus.h"
#include "DHT11.h"
#include "iwdg.h"
#include "oled.h"
#include "display.h"
#include "Buzzer.h"
#include "LightSensor.h"
#include "key.h"
#include "MPU6050.h"
#include "MySPI.h"  
#include "Servo.h"

/* 静态函数：只能在本文件调用  防止编译器优化，确保每次都从内存读取 */
static volatile uint32_t dht11_update_timer = 0;//刷新DHT11读取标志位
static volatile uint32_t light_check_timer = 0;		//光敏检测计时器
static volatile uint32_t mpu6050_check_timer = 0;   // MPU6050检测计时器
//在
volatile uint32_t key_check_timer = 0;              //按键检测计时器,在key中调用

//本设备寄存器中的值
u16 Reg[]={0x0000,	//DHT11温度高八位
           0x0001,	//DHT11温度低八位
           0x0002,	//DHT11湿度高八位
           0x0003,	//DHT11湿度低八位
           0x0004,	//光敏传感器状态寄存器
           0x0005,  // MPU6050方向状态
           0x0006,  // MPU6050加速度X轴高8位
           0x0007,  // MPU6050加速度X轴低8位
           0x0008,  // MPU6050加速度Y轴高8位  
           0x0009,  // MPU6050加速度Y轴低8位
           0x000A,  // MPU6050加速度Z轴高8位
           0x000B,  // MPU6050加速度Z轴低8位
		   0x000C,  // 工作模式 0:自动 1:手动
		   0x000D,  // 手动舵机角度 (0-180) ← ESP32通过06功能码写这个
		   0x000E,  // 手动风扇速度 (0-255) ← 预留
		   0x000F,  // 实际舵机角度 (反馈)
		   0x0010,  // 实际风扇速度 (反馈) ← 预留
          };	

void Isr_Init()	//初始化中断
{
	NVIC_InitTypeDef  isr;	//声明一个名为isr的NVIC初始化结构体变量
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);	//将中断优先级寄存器分为抢占优先级1比特(0-1),子优先级3比特(0-7)
													
	isr.NVIC_IRQChannel=TIM2_IRQn;				//配置TIM2中断
	isr.NVIC_IRQChannelCmd=ENABLE;				//使能TIM2中断
	isr.NVIC_IRQChannelPreemptionPriority=1;	//抢占优先级为1(越小越优先)
	isr.NVIC_IRQChannelSubPriority=2;			//子优先级为2(抢占优先级相同时,子优先级越小越优先)
	NVIC_Init(&isr);   							//每次NVIC_Init()调用只配置一个中断通道,所以要两次
	
	isr.NVIC_IRQChannel=USART2_IRQn;			//配置USART2中断
	isr.NVIC_IRQChannelCmd=ENABLE;				//使能USART2中断
	isr.NVIC_IRQChannelPreemptionPriority=1;	//配置抢占优先级为1
	isr.NVIC_IRQChannelSubPriority=0;			//子优先级为0(确保串口数据到来时能被优先及时响应)
	NVIC_Init(&isr);   							//配置中断
}

void TIM2_IRQHandler()//定时器2的中断服务子函数  1ms一次中断
{
  u8 st;
  st = TIM_GetFlagStatus(TIM2, TIM_FLAG_Update);	//获取定时器更新中断标志位状态
	if(st == SET)	//到达1ms
	{
	  TIM_ClearFlag(TIM2, TIM_FLAG_Update);	//清除更新中断标志位
		if(modbus.timrun != 0)				//启动Modbus定时器
		{
		  modbus.timout ++; 				//Modbus超时计数器自增(1ms一次)
		  if(modbus.timout >= 90)  			//超时计数器;累计间隔时间达到了时间(间隔3.5字节以上)
			{
				modbus.timrun = 0;			//关闭Modbus定时器--停止定时
				modbus.reflag = 1;  		//收到一帧数据
			}
		}  
			IWDG_CheckTimeout();			//软件看门狗,检查通信是否超时
			dht11_update_timer ++;			//刷新读取标志位自增
			light_check_timer++;			//光敏检测计时器自增
			mpu6050_check_timer++;			// MPU6050检测计时器自增
			key_check_timer++;				// 按键检测计时器自增
	}	
}



int main()
{	
  MySPI_Init();		//高内聚,低耦合
  Timer2_Init();  	//定时器初始化
  Mosbus_Init();	//Modbus初始化
  IWDG_Init();  	//看门狗初始化
  IWDG_Feed();
  DH11_GPIO_Init(); //DHT11初始化
  Isr_Init();		//中断初始化                
  OLED_Init();		//OLED初始化
  Buzzer_Init();
  Servo_Init();
  Servo_SetAngle(90);  // 明确设置90度
  Key_Init();
  MPU6050_Init(); 
  Display_Init();
 
//  IWDG_Init();  	//看门狗初始化

	  // STM32复位后主动设置旋转编码器到手动模式，并同步角度
    Reg[0x000C] = 1;  // 强制设置为手动模式
    Servo_SetAngle(90); // 复位后统一到90度
    Reg[0x000D] = 90;  // 更新目标角度
    Reg[0x000F] = 90;  // 更新实际角度
	
  while(1)	//主循环
  {
		Mosbus_Event();				// Modbus正常通信
		IWDG_Feed();  				// 喂硬件看门狗，防程序卡死
	  
	   // 按键检测和显示模式切换
        if (Key_GetState() == 1) {
            Display_SwitchMode();
        }
        
   // 实时更新显示
    if (Display_GetCurrentMode() == DISPLAY_MODE_SENSOR) 
	{
        Display_UpdateSensorData(Reg);
    } 
	else 
	{
        Display_UpdateMPU6050Data(Reg);
    }
	
	if(dht11_update_timer >= 2000) 	// 每2秒更新一次DHT11数据
	{  
    DHT11ReadData((uint8_t*)&Reg[0], (uint8_t*)&Reg[1], 	//将DHT11的8位数据传到存储的16位数组
                 (uint8_t*)&Reg[2], (uint8_t*)&Reg[3]);
	dht11_update_timer = 0;			//清零刷新读取标志位
    
	}
	 // 光敏传感器检测 (50ms一次)
        if(light_check_timer >= 50) {
            if(LightSensor_Get() == 1) {  // 暗
                Buzzer_ON();
                Reg[4] = 1;
            } else {                      // 亮
                Buzzer_OFF();  
                Reg[4] = 0;
            }
            light_check_timer = 0;
		}
		
		 // MPU6050方向检测 (100ms一次)
        if(mpu6050_check_timer >= 100) {
    Display_UpdateMPU6050Direction(Reg);  // 高内聚：显示相关的都在display中
    mpu6050_check_timer = 0;
		}
}
}
