#include "BreathingLight.h"

#include "tim.h"
void BreathingLight_init()
{
    HAL_TIM_PWM_Start(&htim15, TIM_CHANNEL_1);
    __HAL_TIM_SET_COMPARE(&htim15, TIM_CHANNEL_1, 500-1);
}
void BreathingLight_update()
{
    static int i = 0;
    static uint8_t dir = 0;

	if(i >= 499)
    {
        dir = 1;
        
    }
    else if(i == 0)
    {
        dir = 0;
    }

    __HAL_TIM_SET_COMPARE(&htim15, TIM_CHANNEL_1,  dir ? i--: i++);

}