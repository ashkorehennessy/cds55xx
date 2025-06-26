/*
 * CDS55XX Servo Control Library
 *
 * Copyright (c) 2025 ashkorehennessy
 *
 * This software is licensed under the MIT License.
 * Permission is hereby granted, free of charge, to use, modify, and distribute this software.
 * See the LICENSE file for details.
 */

#ifndef __CDS55XX_H__
#define __CDS55XX_H__

#include <stdint.h>

#define CDS55XX_BROADCAST_ID  0xFE

// 舵机模式
#define CDS55XX_MODE_SERVO    0x00
#define CDS55XX_MODE_MOTOR    0x01

// 舵机寄存器地址（推测）
#define CDS55XX_REG_MODE      0x18
#define CDS55XX_REG_GOAL_POS  0x1E
#define CDS55XX_REG_DATA_LEN  0x04

// 舵机指令码
#define CDS55XX_INST_READ     0x02
#define CDS55XX_INST_WRITE    0x03
#define CDS55XX_INST_SYNC_WRITE 0x83

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 发送数据包到CDS55xx舵机
 * @param data 指向要发送的数据的指针
 * @param len 数据长度
 */
void CDS55XX_SendPacket(const uint8_t *data, uint8_t len);

/**
 * @brief 设置单个舵机的工作模式
 * @param id 舵机ID
 * @param mode 工作模式（0=舵机模式，1=电机模式）
 */
void CDS55XX_SetMode(uint8_t id, uint8_t mode);

/**
 * @brief 设置单个舵机的目标位置和速度（用于舵机模式）
 * @param id 舵机ID
 * @param position 目标位置（0-1023）
 * @param speed 运动速度（0-1023）
 */
void CDS55XX_SetPos(uint8_t id, int16_t position, int16_t speed);

/**
 * @brief 设置单个舵机的速度（用于电机模式）
 * @param id 舵机ID
 * @param speed 运动速度（-1023到1023）
 */
void CDS55XX_SetSpeed(uint8_t id, int16_t speed);

/**
 * @brief 同步写多个舵机的目标位置和速度（用于舵机模式）
 * @param id_list 舵机ID列表
 * @param pos_list 目标位置列表
 * @param speed_list 运动速度列表
 * @param count 舵机数量
 */
void CDS55XX_SyncWritePosSpeed(const uint8_t *id_list, const int16_t *pos_list, const int16_t *speed_list, uint8_t count);

/**
 * @brief 同步写多个舵机的速度（用于电机模式）
 * @param id_list 舵机ID列表
 * @param speed_list 运动速度列表
 * @param count 舵机数量
 */
void CDS55XX_SyncWriteSpeed(const uint8_t *id_list, const int16_t *speed_list, uint8_t count);

#ifdef __cplusplus
}
#endif
#endif