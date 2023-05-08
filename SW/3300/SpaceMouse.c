#include "SpaceMouse.h"

#include "usart.h"
#include "stdint.h"

/*4.2.1 Command structure:
All commands are single byte commands with MSB set to logic 1.
Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
1 X X X X X X X
Each command is acknowledged by a response, each response of the 3DX-Sensor is
terminated by an end-byte 0x8D (MSB is set).
4.2.2 3DX-Sensor commands:
SET_ZERO_POSITION
Function: sets the current position of the device as zero-position
Command: 173 (0xAD)
Returns: 0xAD 0x8D
During power-up of the device, the current position of the device is also set as the zeroposition. 
AUTO_DATA_ON
Function: starts automatic transmission of data (30ms intervals)
Command: 174 (0xAE)
Returns: 0xAE 0x8D
AUTO_DATA_OFF
Function: stops automatic transmission of data
Command: 175 (0xAF)
Returns: 0xAF 0x8D
REQUEST_DATA
Function: requests position data from the 3DX-Sensor
Command: 172 (0xAC)
Returns: 16 bytes data
Structure: B1 B2 … B16
Byte 1: start-byte 0x96 (150 decimal); every data set starts with this byte
Byte 2: high byte of X value
Byte 3: low byte of X value
Byte 4: high byte of Y value
Byte 5: low byte of Y value
Byte 6: high byte of Z value
Byte 7: low byte of Z value
Byte 8: high byte of A value
Byte 9: low byte of A value
Byte 10: high byte of B value
Byte 11: low byte of B value 
Byte 12: high byte of C value
Byte 13: low byte of C value
Byte 14: high byte of Checksum
Byte 15: low byte of Checksum
Byte 16: end-byte 0x8D; every response ends with this byte
X, Y, Z, A, B, C values and the Checksum are transmitted as unsigned 14-Bit values. This is 
due to the fact, that the MSB of payload data is always cleared (logic 0).
Calculating a value:
high byte (X) low byte (X)


14-bit value (unsigned)
Xvalue = (high byte (X) * 128 + low byte (X)) - 8192
Transmitted Checksum:
Checksumtrans = (high byte (Checksumtrans) * 128 + low byte (Checksumtrans))
Calculating the Checksum:
Checksumcalc = (Byte1 + Byte2 + … + Byte13) & 0x3FFF.
By masking the Checksum with 0x3FFF (logic AND operation), the value is reduced to a 14-Bit value.
*/

#define SPACEMOUSE_SET_ZERO_POSITION 0xAD
#define SPACEMOUSE_AUTODATA_ON 0xAE
#define SPACEMOUSE_AUTODATA_OFF 0xAF
#define SPACEMOUSE_REQUEST_DATA 0xAC
#define SPACEMOUSE_DATA_START_BYTE 0x96
#define SPACEMOUSE_END_BYTE 0x8D

struct SpaceMouse_RxData SpaceMouse_rxData;
uint8_t SpaceMouse_rxRawData[17];

enum SpaceMouse_State
{
    SPACEMOUSE_RESET,
    SPACEMOUSE_INIT,
    SPACEMOUSE_RUNNING
};

enum SpaceMouse_State state = SPACEMOUSE_RESET;

void decode()
{
    SpaceMouse_rxData.isConnected = 1;
    SpaceMouse_rxData.x = (SpaceMouse_rxRawData[1] << 7) + SpaceMouse_rxRawData[2] - 8192;
    SpaceMouse_rxData.y = (SpaceMouse_rxRawData[3] << 7) + SpaceMouse_rxRawData[4] - 8192;
    SpaceMouse_rxData.z = (SpaceMouse_rxRawData[5] << 7) + SpaceMouse_rxRawData[6] - 8192;
    SpaceMouse_rxData.a = (SpaceMouse_rxRawData[7] << 7) + SpaceMouse_rxRawData[8] - 8192;
    SpaceMouse_rxData.b = (SpaceMouse_rxRawData[9] << 7) + SpaceMouse_rxRawData[10] - 8192;
    SpaceMouse_rxData.c = (SpaceMouse_rxRawData[11] << 7) + SpaceMouse_rxRawData[12] - 8192;
}

uint16_t getChecksum()
{
    static uint32_t checksum = 0;
    checksum = 0;
    for(uint8_t i = 0; i < 13; ++i)
    {
        checksum += SpaceMouse_rxRawData[i];
    }
    return checksum &= 0x3FFF;
}

void rxCallback(UART_HandleTypeDef *huart, uint16_t length)
{
    switch (state)
    {

    case SPACEMOUSE_INIT:
        if(length == 2 && SpaceMouse_rxRawData[0] == SPACEMOUSE_AUTODATA_ON && SpaceMouse_rxRawData[1] == SPACEMOUSE_END_BYTE)
        {
            state = SPACEMOUSE_RUNNING;
        }
        break;

    case SPACEMOUSE_RUNNING:
        uint16_t checksumIn = (SpaceMouse_rxRawData[13] << 7) + SpaceMouse_rxRawData[14];
        if(length == 16 && SpaceMouse_rxRawData[0] == SPACEMOUSE_DATA_START_BYTE && SpaceMouse_rxRawData[15] == SPACEMOUSE_END_BYTE && checksumIn == getChecksum())
        {
            decode();
        }
        else
        {
            SpaceMouse_rxData.isConnected = 0;
        }
        break;
    default:
        SpaceMouse_rxData.isConnected = 0;
        break;
    }
    HAL_UARTEx_ReceiveToIdle_IT(&huart6, SpaceMouse_rxRawData, 17);
}

void SpaceMouse_init()
{
    SpaceMouse_rxData.isConnected = 0;
    HAL_UART_RegisterRxEventCallback(&huart6, rxCallback);
    HAL_UARTEx_ReceiveToIdle_IT(&huart6, SpaceMouse_rxRawData, 17);
    uint8_t txData[] = {SPACEMOUSE_AUTODATA_ON};
    txData[0] = SPACEMOUSE_AUTODATA_ON;
    state = SPACEMOUSE_INIT;
    HAL_UART_Transmit(&huart6, txData, 1, 100);
}


