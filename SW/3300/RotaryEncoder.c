#include "RotaryEncoder.h"
#include "tim.h"

#include <stdint.h>
void RotaryEncoder_init()
{
    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_1 | TIM_CHANNEL_2);
}

uint32_t RotaryEncoder_read()
{
    return __HAL_TIM_GET_COUNTER(&htim2);
}