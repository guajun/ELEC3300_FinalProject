#pragma once
#include "stdint.h"

#define HT4310_NUM 2

enum HT4310_State
{
    HT4310_INIT = 0,
    HT4310_RUNNING
};

typedef struct 
{
    // volatile float rpm;
    // volatile float position; 
    // volatile float torque;
    // volatile uint8_t rotorTemperature;
    // volatile uint8_t mosTemperature;

} HT4310_Feedback;

typedef struct 
{
    volatile int32_t position;
}HT4310_Control;

typedef struct 
{
    HT4310_Control control;
    HT4310_Feedback feedback;
    uint8_t rxData[8];
    enum HT4310_State state;
    uint8_t motorID; // Not CAN ID
}HT4310;

extern HT4310 HT4310_insts[HT4310_NUM];

void HT4310_init();
void HT4310_update();
