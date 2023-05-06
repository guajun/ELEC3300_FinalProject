#pragma once

#include "stdint.h"

#include "fdcan.h"

#define DM4310_NUM 2


enum DM4310_Error
{
    NO_ERR = 7,
    OVER_VOLTAGE,
    UNDER_VOLTAGE,
    OVER_CURRENT,
    MOS_OVER_HEAT,
    ROTOR_OVER_HEAT,
    LOSE_CONNECTION,
    OVER_LOAD
};

enum DM4310_State
{
    DM4310_INIT = 0,
    DM4310_RUNNING
};

typedef struct 
{
    enum DM4310_Error error;
    volatile float rpm;
    volatile float position;
    volatile float torque;
    volatile uint8_t rotorTemperature;
    volatile uint8_t mosTemperature;

}DM4310_Feedback;

typedef struct 
{
    volatile float rpm;
    volatile float position;
}DM4310_Control;

typedef struct 
{
    DM4310_Control control;
    DM4310_Feedback feedback;

    uint8_t rxData[8];
    enum DM4310_State state;
    // volatile int32_t rotaryCnt;
    // volatile float positionOffset;

    uint8_t motorID; // Not CAN ID
}DM4310;


extern DM4310 DM4310_insts[DM4310_NUM];

void DM4310_update();
void DM4310_init();