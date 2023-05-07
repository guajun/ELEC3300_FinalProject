#include "OS.h"

#include "tim.h"

#include "stdint.h"

#include "DM4310.h"
#include "HT4310.h"
#include "GO_M8010_6.h"
#include "SpaceMouse.h"
#include "RotaryEncoder.h"
#include "IMU.h"
// #inlcude 

float OS_tickFreq = 0;

uint32_t encoder = 0;

static void thread(TIM_HandleTypeDef *htim)
{
    static uint32_t lastTick = 0;
    OS_tickFreq = (HAL_GetTick() - lastTick) / 0.001;
    lastTick = HAL_GetTick();

    encoder = RotaryEncoder_read();

    DM4310_update();
    HT4310_update();
    GO_M8010_6_update();

}

void OS_init()
{
    HAL_TIM_RegisterCallback(&htim6, HAL_TIM_PERIOD_ELAPSED_CB_ID, thread);
    HAL_TIM_Base_Start_IT(&htim6);
}