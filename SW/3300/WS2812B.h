#pragma once
#include "stdint.h"
struct GRB
{
    uint8_t g : 8;
    uint8_t r : 8;
    uint8_t b : 8;
};

void WS2812B_convert(struct GRB grbArray[], uint16_t nLed);
void WS2812B_send(uint16_t nLed);


