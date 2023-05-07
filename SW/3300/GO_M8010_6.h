#pragma once

#include "stdint.h"

#define GO_M8010_6_NUM 2


// #pragma pack()

#pragma pack(1)
typedef struct      // __attribute__((packed)) <- c++
{
    uint8_t id     :4;      // 电机ID: 0,1...,14 15表示向所有电机广播数据(此时无返回)
    uint8_t status :3;      // 工作模式: 0.锁定 1.FOC闭环 2.编码器校准 3.保留
    uint8_t none   :1;      // 保留位
} GO_M8010_6_Mode /*__attribute__((packed))*/;   // 控制模式 1Byte

typedef struct
{
    int16_t tor_des;        // 期望关节输出扭矩 unit: N.m     (q8)
    int16_t spd_des;        // 期望关节输出速度 unit: rad/s   (q7)
    int32_t pos_des;        // 期望关节输出位置 unit: rad     (q15)
    uint16_t  k_pos;        // 期望关节刚度系数 unit: 0.0-1.0 (q15)
    uint16_t  k_spd;        // 期望关节阻尼系数 unit: 0.0-1.0 (q15)
    
} GO_M8010_6_Command;   // 控制参数 12Byte

enum GO_M8010_6_State
{
    GO_M8010_6_INIT = 0,
    GO_M8010_6_RUNNING
};

/**
 * @brief 控制数据包格式
 * 
 */
typedef struct
{
    uint8_t head[2];    // 包头         2Byte
    GO_M8010_6_Mode mode;    // 电机控制模式  1Byte
    GO_M8010_6_Command command;    // 电机期望数据 12Byte
    uint16_t   CRC16;   // CRC          2Byte

} GO_M8010_6_Control;    // 主机控制命令     17Byte

typedef struct
{          
    uint8_t head[2]; 
    GO_M8010_6_Mode mode;    // 电机控制模式  1Byte
    int16_t  torque;        // 实际关节输出扭矩 unit: N.m     (q8)
    int16_t  speed;         // 实际关节输出速度 unit: rad/s   (q7)
    int32_t  pos;           // 实际关节输出位置 unit: W       (q15)
    int8_t   temp;          // 电机温度: -128~127°C 90°C时触发温度保护
    uint8_t  MError :3;     // 电机错误标识: 0.正常 1.过热 2.过流 3.过压 4.编码器故障 5-7.保留
    uint16_t force  :12;    // 足端气压传感器数据 12bit (0-4095)
    uint8_t  none   :1;     // 保留位
    uint16_t   CRC16;   // CRC          2Byte

} GO_M8010_6_Feedback;

#pragma pack()


// typedef struct 
// {
//     // volatile float rpm;
//     // volatile float position; 
//     // volatile float torque;
//     // volatile uint8_t rotorTemperature;
//     // volatile uint8_t mosTemperature;

// } GO_M8010_6_Feedback;

// typedef struct 
// {
//     volatile int32_t position;
// }GO_M8010_6_Control;

typedef struct 
{
    float tarTor;
    float tarPos;                  // target position 
    float tarW;                    // target speed

    float kPos;
    float kSpeed;

    float torque;
    float w;
    float pos;

    int8_t temperature;

    GO_M8010_6_Control control;
    GO_M8010_6_Feedback feedback;
    // uint8_t txData[17];
    // enum GO_M8010_6_State state;
    uint8_t motorID; // Not CAN ID
} GO_M8010_6;

__attribute__((section(".D1"))) extern GO_M8010_6 GO_M8010_6_insts[GO_M8010_6_NUM];

void GO_M8010_6_init();
void GO_M8010_6_update();