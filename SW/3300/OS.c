#include "tim.h"
#include "OS.h"
#include "DM4310.h"
#include "stdint.h"

float OS_tickFreq = 0;


static void thread(TIM_HandleTypeDef *htim)
{
    static uint32_t lastTick = 0;
    OS_tickFreq = (HAL_GetTick() - lastTick) / 0.001;
    lastTick = HAL_GetTick();

    DM4310_update();
    HT4310_update();
}

void OS_init()
{
    HAL_TIM_RegisterCallback(&htim6, HAL_TIM_PERIOD_ELAPSED_CB_ID, thread);
    HAL_TIM_Base_Start_IT(&htim6);
}