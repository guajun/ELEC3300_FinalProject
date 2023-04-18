#include "TemperatureSensor.h"

#include "adc.h"
uint16_t VValue;
void TemSensor_compliteCallback(ADC_HandleTypeDef *hadc){
    VValue = HAL_ADC_GetValue(hadc);
};

void TemperatureSensor_Init(void)
{
    HAL_ADCEx_Calibration_Start(&hadc3,ADC_CALIB_OFFSET,ADC_SINGLE_ENDED); // Calib
    HAL_ADC_Start_IT(&hadc3);
    HAL_ADC_RegisterCallback(&hadc3, HAL_ADC_CONVERSION_COMPLETE_CB_ID,TemSensor_compliteCallback);
}

float TemperatureSensor_getTemp()
{
    float Voltage =(float)(VValue *3.3/4096);
    return ((Voltage-0.76)/0.0025)+25;  // temperature
}