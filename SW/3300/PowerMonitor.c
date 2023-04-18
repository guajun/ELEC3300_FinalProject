#include "PowerMonitor.h"

#include "adc.h"
uint16_t VValue;
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc){
    VValue = HAL_ADC_GetValue(hadc);
};

void PowerMonitor_Init(void)
{
    HAL_ADCEx_Calibration_Start(&hadc1,ADC_CALIB_OFFSET,ADC_SINGLE_ENDED); // Calib
    HAL_ADC_Start_IT(&hadc1);
    HAL_ADC_RegisterCallback(&hadc1, HAL_ADC_CONVERSION_COMPLETE_CB_ID,void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc));
}

float PowerMonitor_getPower()
{
    float Voltage =(float)(VValue *3.3/4096);
    return (Voltage*Voltage)/0.004;
}