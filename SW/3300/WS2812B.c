#include "WS2812B.h"

#include "stdint.h"
#include "usart.h"

#define WS2812B_TX_DATA_000 ((uint8_t)0b1101101)
#define WS2812B_TX_DATA_001 ((uint8_t)0b1101100)
#define WS2812B_TX_DATA_010 ((uint8_t)0b1100101)
#define WS2812B_TX_DATA_011 ((uint8_t)0b1100100)
#define WS2812B_TX_DATA_100 ((uint8_t)0b0101101)
#define WS2812B_TX_DATA_101 ((uint8_t)0b0101100)
#define WS2812B_TX_DATA_110 ((uint8_t)0b0100101)
#define WS2812B_TX_DATA_111 ((uint8_t)0b0100100)

#define WS2812B_BIT_MASK_0 ((uint32_t)0b111)
#define WS2812B_BIT_MASK_1 (((uint32_t)0b111) << 3)
#define WS2812B_BIT_MASK_2 (((uint32_t)0b111) << 6)
#define WS2812B_BIT_MASK_3 (((uint32_t)0b111) << 9)
#define WS2812B_BIT_MASK_4 (((uint32_t)0b111) << 12)
#define WS2812B_BIT_MASK_5 (((uint32_t)0b111) << 15)
#define WS2812B_BIT_MASK_6 (((uint32_t)0b111) << 18)
#define WS2812B_BIT_MASK_7 (((uint32_t)0b111) << 21)

#define WS2812B_BIT_MASK_RESULT_000 ((uint32_t)0b000)
#define WS2812B_BIT_MASK_RESULT_001 ((uint32_t)0b001)
#define WS2812B_BIT_MASK_RESULT_010 ((uint32_t)0b010)
#define WS2812B_BIT_MASK_RESULT_011 ((uint32_t)0b011)
#define WS2812B_BIT_MASK_RESULT_100 ((uint32_t)0b100)
#define WS2812B_BIT_MASK_RESULT_101 ((uint32_t)0b101)
#define WS2812B_BIT_MASK_RESULT_110 ((uint32_t)0b110)
#define WS2812B_BIT_MASK_RESULT_111 ((uint32_t)0b111)


const uint8_t table[] = 
{
    WS2812B_TX_DATA_000,
    WS2812B_TX_DATA_001,
    WS2812B_TX_DATA_010,
    WS2812B_TX_DATA_011,
    WS2812B_TX_DATA_100,
    WS2812B_TX_DATA_101,
    WS2812B_TX_DATA_110,
    WS2812B_TX_DATA_111
};


void WS2812B_convert(struct GRB grbArray[], uint16_t nLed, uint8_t* txBuffer)
{
    for(uint16_t i = 0; i < nLed; i++)
    {
        static uint32_t grb = 0; 
        static uint8_t* grb_g = (uint8_t*)&grb + 2;
        static uint8_t* grb_r = (uint8_t*)&grb + 1;
        static uint8_t* grb_b = (uint8_t*)&grb;

        *grb_g = grbArray[i].g;
        *grb_r = grbArray[i].r;
        *grb_b = grbArray[i].b;

        static uint8_t masked[8];

        masked[7] = (grb & WS2812B_BIT_MASK_0);
        masked[6] = (grb & WS2812B_BIT_MASK_1) >> 3;
        masked[5] = (grb & WS2812B_BIT_MASK_2) >> 6;
        masked[4] = (grb & WS2812B_BIT_MASK_3) >> 9;
        masked[3] = (grb & WS2812B_BIT_MASK_4) >> 12;
        masked[2] = (grb & WS2812B_BIT_MASK_5) >> 15;
        masked[1] = (grb & WS2812B_BIT_MASK_6) >> 18;
        masked[0] = (grb & WS2812B_BIT_MASK_7) >> 21;

        for(uint8_t j = 0; j < 8; j++)
        {
            txBuffer[i * 8 + j] = table[masked[j]];
        }

    }

}

void WS2812B_send(uint8_t* txBuffer, uint16_t nLed)
{
    HAL_UART_Transmit(&huart4, txBuffer, nLed * 8, 2);
}