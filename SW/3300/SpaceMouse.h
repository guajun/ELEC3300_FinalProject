#pragma once

#include "stdint.h"
void SpaceMouse_init();

struct SpaceMouse_RxData
{
    uint8_t isConnected;
    int16_t x;
    int16_t y;
    int16_t z;
    int16_t a;
    int16_t b;
    int16_t c;
};
extern struct SpaceMouse_RxData SpaceMouse_rxData;