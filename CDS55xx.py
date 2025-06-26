import time
import serial

# CDS55XX 参数定义
CDS55XX_BROADCAST_ID = 0xFE

# 舵机模式
CDS55XX_MODE_SERVO = 0x00
CDS55XX_MODE_MOTOR = 0x01

# 舵机寄存器地址（推测）
CDS55XX_REG_MODE = 0x18
CDS55XX_REG_GOAL_POS = 0x1E
CDS55XX_REG_DATA_LEN = 0x04

# 舵机指令码
CDS55XX_INST_READ = 0x02
CDS55XX_INST_WRITE = 0x03
CDS55XX_INST_SYNC_WRITE = 0x83

# 初始化串口
ser = serial.Serial('/dev/ttyS0', baudrate=1000000, timeout=0.1)

# 计算校验和，校验范围从 ID 开始到最后一个参数
def calc_checksum(data):
    sum_val = sum(data[2:])
    return (~sum_val) & 0xFF  # 取反后再取低 8 位

# 构建完整数据包并发送（加上包头和校验）
def build_and_send_packet(payload):
    buf = bytearray([0xFF, 0xFF]) + payload
    buf.append(calc_checksum(buf))
    CDS55XX_SendPacket(buf)

# 用户需实现此函数用于串口发送数据包
def CDS55XX_SendPacket(data):
    ser.write(data)

# 设置单个舵机的工作模式（0=舵机模式，1=电机模式）
def CDS55XX_SetMode(id, mode):
    pkt = bytearray([
        id,                   # 舵机 ID
        0x04,                 # 长度（指令 + 参数 + 校验）
        CDS55XX_INST_WRITE,   # 指令码：写指令
        CDS55XX_REG_MODE,     # 模式寄存器地址
        0x00                  # 模式值（0=舵机，1=电机），但是1不起作用
    ])
    build_and_send_packet(pkt)

# 设置单个舵机的目标位置和速度（用于舵机模式）
def CDS55XX_SetPos(id, position, speed):
    if position > 1023:
        position = 1023
    if speed > 1023:
        speed = 1023
    CDS55XX_SyncWritePosSpeed([id], [position], [speed], 1)

# 设置单个舵机的速度（用于电机模式）
def CDS55XX_SetSpeed(id, speed):
    if speed < -1023:
        speed = -1023
    if speed > 1023:
        speed = 1023
    CDS55XX_SyncWriteSpeed([id], [speed], 1)

# 同步写多个舵机的目标位置和速度（用于舵机模式）
def CDS55XX_SyncWritePosSpeed(id_list, pos_list, speed_list, count):
    buf = bytearray([
        CDS55XX_BROADCAST_ID,    # 广播 ID
        0,                       # 数据长度占位
        CDS55XX_INST_SYNC_WRITE, # 同步写指令
        CDS55XX_REG_GOAL_POS,    # 起始地址：目标位置
        CDS55XX_REG_DATA_LEN     # 每个舵机数据长度
    ])
    for i in range(count):
        pos = min(pos_list[i], 1023)
        spd = min(speed_list[i], 1023)
        buf += bytearray([
            id_list[i],             # 舵机 ID
            pos & 0xFF,             # 位置低八位
            (pos >> 8) & 0xFF,      # 位置高八位
            spd & 0xFF,             # 速度低八位
            (spd >> 8) & 0xFF       # 速度高八位
        ])
    buf[1] = len(buf) - 2 + 1  # 更新有效长度
    build_and_send_packet(buf)

# 同步写多个舵机的速度（用于电机模式）
def CDS55XX_SyncWriteSpeed(id_list, speed_list, count):
    buf = bytearray([
        CDS55XX_BROADCAST_ID,    # 广播 ID
        0,                       # 数据长度占位
        CDS55XX_INST_SYNC_WRITE, # 同步写指令
        CDS55XX_REG_GOAL_POS,    # 起始地址：目标位置（此处忽略）
        CDS55XX_REG_DATA_LEN     # 每个舵机数据长度
    ])
    for i in range(count):
        spd = speed_list[i]
        if spd < -1023:
            spd = -1023
        if spd > 1023:
            spd = 1023
        # 如果是负速度，编码为 abs(speed) + 1024
        enc_spd = spd if spd >= 0 else abs(spd) + 1024
        buf += bytearray([
            id_list[i],             # 舵机 ID
            0x00, 0x02,             # 位置字段占位（忽略）
            enc_spd & 0xFF,         # 速度低八位
            (enc_spd >> 8) & 0xFF   # 速度高八位
        ])
    buf[1] = len(buf) - 2 + 1   # 更新有效长度
    build_and_send_packet(buf)


# 示例：设置舵机模式并移动到指定位置
if __name__ == '__main__':
    time.sleep(2)  # 等待舵机初始化
    CDS55XX_SetMode(3, CDS55XX_MODE_SERVO)
    while True:
        CDS55XX_SetPos(3, 384, 512)
        time.sleep(1)
        CDS55XX_SetPos(3, 768, 512)
        time.sleep(1)