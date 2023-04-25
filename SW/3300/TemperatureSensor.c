#include "TemperatureSensor.h"

#include "adc.h"
#include "stdint.h"

#define TEMPERATURE_SENSOR_VREF ((uint16_t)3100)

uint16_t adcData = 0;

void TemperatureSensor_callback(ADC_HandleTypeDef *hadc)
{
    adcData = HAL_ADC_GetValue(hadc);
};

void TemperatureSensor_init(void)
{
    HAL_ADCEx_Calibration_Start(&hadc3, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED); // Calib
    HAL_ADC_RegisterCallback(&hadc3, HAL_ADC_CONVERSION_COMPLETE_CB_ID, TemperatureSensor_callback);
    HAL_ADC_Start_IT(&hadc3);
}

uint16_t TemperatureSensor_getTemp()
{
    return __HAL_ADC_CALC_TEMPERATURE(TEMPERATURE_SENSOR_VREF, adcData, ADC_RESOLUTION_12B);
}