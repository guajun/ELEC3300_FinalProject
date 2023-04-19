#pragma once
#include "stdint.h"
struct GRB
{
    uint8_t g : 8;
    uint8_t r : 8;
    uint8_t b : 8;
};

void WS2812B_convert(struct GRB grbArray[], uint16_t nLed, uint8_t* txBuffer);
void WS2812B_send(uint8_t* txBuffer, uint16_t nLed);


