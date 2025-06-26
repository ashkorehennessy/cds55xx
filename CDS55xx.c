/*
 * CDS55XX Servo Control Library
 *
 * Copyright (c) 2025 ashkorehennessy
 *
 * This software is licensed under the MIT License.
 * Permission is hereby granted, free of charge, to use, modify, and distribute this software.
 * See the LICENSE file for details.
 */

#include "CDS55xx.h"
#include <stm32f1xx_hal.h>
#include <usart.h>
#include "usbd_cdc_if.h"

// 计算校验和，校验范围从 ID 开始到最后一个参数
static uint8_t calc_checksum(const uint8_t *data, uint8_t len) {
    int16_t sum = 0;
    for (uint8_t i = 2; i < len - 1; ++i) {
        sum += data[i];
    }
    return (~sum) & 0xFF;  // 取反后再取低 8 位
}

// 构建完整数据包并发送（加上包头和校验）
static void build_and_send_packet(const uint8_t *payload, uint8_t len) {
    uint8_t buf[32] = {0xFF, 0xFF};  // 包头
    for (uint8_t i = 0; i < len; ++i) buf[i + 2] = payload[i];
    buf[len + 2] = calc_checksum(buf, len + 3);  // 添加校验和
    CDS55XX_SendPacket(buf, len + 3);  // 调用自己实现的串口发送函数
}

// 用户需实现此函数用于串口发送数据包
void CDS55XX_SendPacket(const uint8_t *data, uint8_t len) {
    HAL_UART_Transmit(&huart1, (uint8_t *)data, len, HAL_MAX_DELAY);
    // CDC_Transmit_FS((uint8_t *)data, len); // 使用USB CDC接口发送数据
}

// 设置单个舵机的工作模式（0=舵机模式，1=电机模式）
void CDS55XX_SetMode(uint8_t id, uint8_t mode) {
    uint8_t pkt[] = {
        id,                   // 舵机 ID
        0x04,                 // 长度（指令 + 参数 + 校验）
        CDS55XX_INST_WRITE,   // 指令码：写指令
        CDS55XX_REG_MODE,     // 模式寄存器地址
        0x00                  // 模式值（0=舵机，1=电机），但是1不起作用
    };
    build_and_send_packet(pkt, sizeof(pkt));
}

// 设置单个舵机的目标位置和速度（用于舵机模式）
void CDS55XX_SetPos(uint8_t id, int16_t position, int16_t speed) {
    if (position > 1023) position = 1023;
    if (speed > 1023) speed = 1023;
    CDS55XX_SyncWritePosSpeed(&id, &position, &speed, 1);
}

// 同步写多个舵机的目标位置和速度（用于舵机模式）
void CDS55XX_SyncWritePosSpeed(const uint8_t *id_list, const int16_t *pos_list, const int16_t *speed_list, uint8_t count) {
    uint8_t buf[128] = {
        CDS55XX_BROADCAST_ID,    // 广播 ID
        0,                       // 数据长度占位
        CDS55XX_INST_SYNC_WRITE, // 同步写指令
        CDS55XX_REG_GOAL_POS,    // 起始地址：目标位置
        CDS55XX_REG_DATA_LEN     // 每个舵机数据长度
    };
    uint8_t offset = 5;
    for (uint8_t i = 0; i < count; ++i) {
        int16_t pos = pos_list[i] > 1023 ? 1023 : pos_list[i];
        int16_t spd = speed_list[i] > 1023 ? 1023 : speed_list[i];
        buf[offset++] = id_list[i];         // 舵机 ID
        buf[offset++] = pos & 0xFF;         // 位置低八位
        buf[offset++] = (pos >> 8) & 0xFF;  // 位置高八位
        buf[offset++] = spd & 0xFF;         // 速度低八位
        buf[offset++] = (spd >> 8) & 0xFF;  // 速度高八位
    }
    buf[1] = offset - 2 + 1;  // 更新有效长度
    build_and_send_packet(buf, offset);
}

// 设置单个舵机速度（用于电机模式）
void CDS55XX_SetSpeed(uint8_t id, int16_t speed) {
    if (speed < -1023) speed = -1023;
    if (speed > 1023) speed = 1023;
    CDS55XX_SyncWriteSpeed(&id, &speed, 1);
}

// 同步写多个舵机的速度（用于电机模式）
void CDS55XX_SyncWriteSpeed(const uint8_t *id_list, const int16_t *speed_list, uint8_t count) {
    uint8_t buf[128] = {
        CDS55XX_BROADCAST_ID,    // 广播 ID
        0,                       // 数据长度占位
        CDS55XX_INST_SYNC_WRITE, // 同步写指令
        CDS55XX_REG_GOAL_POS,    // 起始地址：目标位置（此处忽略）
        CDS55XX_REG_DATA_LEN     // 每个舵机数据长度
    };
    uint8_t offset = 5;
    for (uint8_t i = 0; i < count; ++i) {
        int16_t spd = speed_list[i];
        int16_t enc_spd = 0;
        // 限制范围在 -1023 ~ 1023
        if (spd < -1023) spd = -1023;
        if (spd > 1023) spd = 1023;
        // 如果是负速度，编码为 abs(speed) + 1024
        enc_spd = (spd >= 0) ? spd : (int16_t)(abs(spd) + 1024);
        buf[offset++] = id_list[i];             // 舵机 ID
        buf[offset++] = 0x00;                   // 位置低八位
        buf[offset++] = 0x02;                   // 位置高八位
        buf[offset++] = enc_spd & 0xFF;         // 速度低八位
        buf[offset++] = (enc_spd >> 8) & 0xFF;  // 速度高八位
    }
    buf[1] = offset - 2 + 1;   // 更新有效长度
    build_and_send_packet(buf, offset);
}

