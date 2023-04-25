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
high byte (X) low byte (X)*/


// #define ZeroPosition 0xAD
// #define AutoDataOn 0xAE
// #define AutoDataOff 0xAF
// #define RequestData 0xAC
// #define EndByte 0x8D;
// #define StartByte 0x96;

#define SPACEMOUSE_SET_ZERO_POSITION 0xAD
#define SPACEMOUSE_AUTODATA_ON 0xAE
#define SPACEMOUSE_AUTODATA_OFF 0xAF
#define SPACEMOUSE_REQUEST_DATA 0xAC
#define SPACEMOUSE_DATA_START_BYTE 0x96
#define SPACEMOUSE_END_BYTE 0x8D


// void ZeroPosition_Set()
// {
//     uint8_t MsgByteArray[1] = {ZeroPosition};
//     uint8_t RecByteArray[2];
//     HAL_UART_Transmit(&huart6, MsgByteArray, 1,10);
//     HAL_UART_Receive_IT(&huart6, RecByteArray, 2);
//     //check if receive correct
//     if ((RecByteArray[0] != ZeroPosition) | (RecByteArray[1] != EndByte))
//     {
//         ZeroPosition_Set();
//     }
// }

// void AutoData_On()
// {
//     uint8_t MsgByteArray[1] = {AutoDataOn};
//     uint8_t RecByteArray[2];
//     HAL_UART_Transmit(&huart6, MsgByteArray, 1);
//     HAL_UART_Receive_IT(&huart6, RecByteArray, 2);
//     //check if receive correct
//     if ((RecByteArray[0] != ZeroPosition) | (RecByteArray[1] != EndByte))
//     {
//         AutoData_On();
//     }
// }

// void AutoData_Off()
// {
//     uint8_t MsgByteArray[1] = {AutoDataOff};
//     uint8_t RecByteArray[2];
//     HAL_UART_Transmit(&huart6, MsgByteArray, 1);
//     HAL_UART_Receive_IT(&huart6, RecByteArray, 2);
//     //check if receive correct
//     if ((RecByteArray[0] != ZeroPosition) | (RecByteArray[1] != EndByte))
//     {
//         AutoData_Off();
//     }
// }

// void RequestData_Send(uint8_t* RecByteArray)
// {
//     uint8_t MsgByteArray[1] = {RequestData};
//     HAL_UART_Transmit(&huart6, MsgByteArray, 1);
//     HAL_UART_Receive_IT(&huart6, RecByteArray, 16);
//     //check if receive correct
//     uint16_t ChecksumRec = ((RecByteArray[13] & 0b1111111) << 7) | (RecByteArray[14] & 0b1111111);
//     uint16_t ChecksumCal = 0;
//     for (int i = 0; i < 13; i++)
//     {
//         ChecksumCal += (RecByteArray[i] & 0b1111111);
//     }
//     ChecksumCal = ChecksumCal & 0x3FFF;
    
//     if ((RecByteArray[0] != StartByte) | (ChecksumRec != ChecksumCal) | (RecByteArray[15] != StartByte))
//     {
//         RequestData_Send(RecByteArray);
//     }

// }

// uint16_t AxisValue_get(char a)
// {
//     uint8_t RecByteArray[16];
//     uint16_t Value = 0;
//     RequestData_Send(RecByteArray);
//     switch (a)
//     {
//     case 'X':
//         Value = ((RecByteArray[1] & 0b1111111)<<7) | (RecByteArray[2] & 0b1111111);
//         break;
//     case 'Y':
//         Value = ((RecByteArray[3] & 0b1111111)<<7) | (RecByteArray[4] & 0b1111111);
//         break;
//     case 'Z':
//         Value = ((RecByteArray[5] & 0b1111111)<<7) | (RecByteArray[6] & 0b1111111);
//         break;
//     case 'A':
//         Value = ((RecByteArray[7] & 0b1111111)<<7) | (RecByteArray[8] & 0b1111111);
//         break;
//     case 'B':
//         Value = ((RecByteArray[9] & 0b1111111)<<7) | (RecByteArray[10] & 0b1111111);
//         break;
//     case 'C':
//         Value = ((RecByteArray[11] & 0b1111111)<<7) | (RecByteArray[12] & 0b1111111);
//         break;
//     default:
//         break;
//     }
//     return Value;
// }

uint8_t rxData[17] = {};

enum SpaceMouse_State
{
    SPACEMOUSE_INIT,
    SPACEMOUSE_RUNNING
};

enum SpaceMouse_State state = SPACEMOUSE_INIT;

void SpaceMouse_rxCallback(UART_HandleTypeDef *huart6, uint16_t length)
{
    switch (state)
    {
    case SPACEMOUSE_INIT:
        // check auto data
        if(length == 2 && rxData[0] == SPACEMOUSE_AUTODATA_ON && rxData[1] == SPACEMOUSE_END_BYTE)
        {
            state = SPACEMOUSE_RUNNING;
        }
         
        break;
    case SPACEMOUSE_RUNNING:

        break;
    default:
        break;
    }
}

void SpaceMouse_init()
{
    // HAL_UART_RegisterCallback(&huart6, HAL_UART_RX_COMPLETE_CB_ID, SpaceMouse_rxCallback);
    HAL_UART_RegisterRxEventCallback(&huart6, SpaceMouse_rxCallback);
    HAL_UARTEx_ReceiveToIdle_IT(&huart6, rxData, 20);
    uint8_t txData[] = {SPACEMOUSE_AUTODATA_ON};
    HAL_UART_Transmit(&huart6, txData, 1, 1);
}


/*
14-bit value (unsigned)
Xvalue = (high byte (X) * 128 + low byte (X)) - 8192
Transmitted Checksum:
Checksumtrans = (high byte (Checksumtrans) * 128 + low byte (Checksumtrans))
Calculating the Checksum:
Checksumcalc = (Byte1 + Byte2 + … + Byte13) & 0x3FFF.
By masking the Checksum with 0x3FFF (logic AND operation), the value is reduced to a 14-
Bit value.
*/