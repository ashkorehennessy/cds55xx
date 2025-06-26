# CDS55XX Servo Controller

一个用于控制 CDS55xx 系列串口舵机的轻量级库，支持位置控制、速度控制、同步写操作等基本功能。

## 特性

- 支持舵机和电机模式
- 兼容 STM32 平台
- 可以快速移植到其他平台
- 仅支持同步写舵机位置或速度
- 不支持任何读取操作

## 协议说明

本项目基于对 CDS55xx 串口舵机通信行为的逆向分析，**未使用任何厂商私有源代码或协议文档**，仅实现公开串口协议兼容。

## 移植说明

仅需要修改相应头文件，再实现`CDS55XX_SendPacket`函数即可完成移植

## 使用方式

### 单个舵机控制

1. 调用`CDS55XX_SetMode`初始化为舵机或电机模式
2. 使用`CDS55XX_SetPos`或`CDS55XX_SetSpeed`设置舵机位置或速度

### 多个舵机控制

1. 调用`CDS55XX_SetMode`初始化所有舵机为舵机或电机模式
2. 为不同模式的舵机分别创建数组用于存储舵机ID
3. 对于舵机模式的舵机，创建用于存储位置和用于存储速度的数组
4. 对于电机模式的舵机，创建用于存储速度的数组
5. 调用`CDS55XX_SyncWritePosSpeed`来控制处于舵机模式的舵机
6. 调用`CDS55XX_SyncWriteSpeed`来控制处于电机模式的舵机
7. 要修改速度或位置，直接修改对应数组中的值，然后再次调用同步写函数

### 示例代码

```c
uint8_t servo_list[4] = {1, 2, 3, 4};
uint8_t motor_list[2] = {5, 6};
uint16_t servo_pos[4] = {512, 512, 512, 512};
uint16_t servo_speed[4] = {200, 200, 200, 200};
uint16_t motor_speed[2] = {100, -100};

CDS55XX_SetMode(1, CDS55XX_MODE_SERVO);
CDS55XX_SetMode(2, CDS55XX_MODE_SERVO);
CDS55XX_SetMode(3, CDS55XX_MODE_SERVO);
CDS55XX_SetMode(4, CDS55XX_MODE_SERVO);
CDS55XX_SetMode(5, CDS55XX_MODE_MOTOR);
CDS55XX_SetMode(6, CDS55XX_MODE_MOTOR);

CDS55XX_SyncWritePosSpeed(servo_list, servo_pos, servo_speed, 4);
CDS55XX_SyncWriteSpeed(motor_list, motor_speed, 2);
```

