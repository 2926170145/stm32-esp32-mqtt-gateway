#ifndef __MPU6050_REG_H
#define __MPU6050_REG_H

/************************ 采样和配置寄存器 ************************/
#define	MPU6050_SMPLRT_DIV		0x19	// 采样率分频器 - 设置传感器数据输出速率
#define	MPU6050_CONFIG			0x1A	// 配置寄存器 - 设置数字低通滤波器参数
#define	MPU6050_GYRO_CONFIG		0x1B	// 陀螺仪配置 - 设置陀螺仪量程和自检
#define	MPU6050_ACCEL_CONFIG	0x1C	// 加速度计配置 - 设置加速度计量程和自检

/************************ 传感器数据输出寄存器 ************************/
/* 加速度计数据 (16位, 需要组合高8位和低8位) */
#define	MPU6050_ACCEL_XOUT_H	0x3B	// 加速度计X轴数据高8位
#define	MPU6050_ACCEL_XOUT_L	0x3C	// 加速度计X轴数据低8位
#define	MPU6050_ACCEL_YOUT_H	0x3D	// 加速度计Y轴数据高8位
#define	MPU6050_ACCEL_YOUT_L	0x3E	// 加速度计Y轴数据低8位
#define	MPU6050_ACCEL_ZOUT_H	0x3F	// 加速度计Z轴数据高8位
#define	MPU6050_ACCEL_ZOUT_L	0x40	// 加速度计Z轴数据低8位

/* 温度传感器数据 */
#define	MPU6050_TEMP_OUT_H		0x41	// 温度数据高8位
#define	MPU6050_TEMP_OUT_L		0x42	// 温度数据低8位

/* 陀螺仪数据 (16位, 需要组合高8位和低8位) */
#define	MPU6050_GYRO_XOUT_H		0x43	// 陀螺仪X轴数据高8位
#define	MPU6050_GYRO_XOUT_L		0x44	// 陀螺仪X轴数据低8位
#define	MPU6050_GYRO_YOUT_H		0x45	// 陀螺仪Y轴数据高8位
#define	MPU6050_GYRO_YOUT_L		0x46	// 陀螺仪Y轴数据低8位
#define	MPU6050_GYRO_ZOUT_H		0x47	// 陀螺仪Z轴数据高8位
#define	MPU6050_GYRO_ZOUT_L		0x48	// 陀螺仪Z轴数据低8位

/************************ 电源管理寄存器 ************************/
#define	MPU6050_PWR_MGMT_1		0x6B	// 电源管理1 - 设备复位、睡眠模式、时钟源选择
#define	MPU6050_PWR_MGMT_2		0x6C	// 电源管理2 - 单个传感器唤醒和休眠控制

/************************ 身份识别寄存器 ************************/
#define	MPU6050_WHO_AM_I		0x75	// 器件ID寄存器 - 读取该寄存器应返回0x68(MPU6050的设备地址)

#endif