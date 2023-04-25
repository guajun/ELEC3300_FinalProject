#include "BreathingLight.h"

#include "tim.h"
#include "stdint.h"

void BreathingLight_init()
{
    HAL_TIM_PWM_Start(&htim15, TIM_CHANNEL_1);
    __HAL_TIM_SET_COMPARE(&htim15, TIM_CHANNEL_1, 1000 - 1);
}
void BreathingLight_update()
{
    uint16_t ccr = 0;

    ccr = HAL_GetTick() % (2000 - 1);

    if((HAL_GetTick() & (uint32_t)0b1111111111) < 512)
    {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
    }

    __HAL_TIM_SET_COMPARE(&htim15, TIM_CHANNEL_1, ccr <= 1000 ? ccr : 2000 - ccr);
}