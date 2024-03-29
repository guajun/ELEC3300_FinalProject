#include "DM4310.h"

#include "stdint.h"
#include "fdcan.h"
#include "string.h"

#define DM4310_MST_ID 0x000
// #define DM4310_SLV_ID 0x001;
DM4310 DM4310_insts[DM4310_NUM];

FDCAN_TxHeaderTypeDef DM4310_txHeader = {
        0x001, FDCAN_STANDARD_ID, FDCAN_DATA_FRAME, FDCAN_DLC_BYTES_8, FDCAN_ESI_PASSIVE, FDCAN_BRS_OFF, FDCAN_CLASSIC_CAN, FDCAN_NO_TX_EVENTS, 0};

static float uintToFloat(int xUint, float xMin, float xMax, int bits)
{
    /// converts unsigned int to float, given range and number of bits ///
    float span = xMax - xMin;
    float offset = xMin;
    return ((float)xUint) * span / ((float)((1 << bits) - 1)) + offset;
}


static void rxFifoCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t rxFifo0ITs)
{
    while (HAL_FDCAN_GetRxFifoFillLevel(hfdcan, FDCAN_RX_FIFO0))
    {
        uint8_t rxData[8];
        static FDCAN_RxHeaderTypeDef rxHeader;
        HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rxHeader, rxData);  
    }
}

void DM4310_update()
{
    for(uint8_t i = 0; i < DM4310_NUM; ++i)
    {
        switch (DM4310_insts[i].state)
        {
        case DM4310_INIT:

            uint8_t enableTxData[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC};
            DM4310_txHeader.Identifier = 0x100 + DM4310_insts[i].motorID;
            HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &DM4310_txHeader, enableTxData);
            DM4310_insts[i].state = DM4310_RUNNING;

        case DM4310_RUNNING:
            uint8_t txData[8] = {};
            DM4310_txHeader.Identifier = 0x100 + DM4310_insts[i].motorID;

            txData[0] = *((uint8_t *)&DM4310_insts[i].control.position);
            txData[1] = *((uint8_t *)&DM4310_insts[i].control.position + 1);
            txData[2] = *((uint8_t *)&DM4310_insts[i].control.position + 2);
            txData[3] = *((uint8_t *)&DM4310_insts[i].control.position + 3);
            txData[4] = *((uint8_t *)&DM4310_insts[i].control.rpm);
            txData[5] = *((uint8_t *)&DM4310_insts[i].control.rpm + 1);
            txData[6] = *((uint8_t *)&DM4310_insts[i].control.rpm + 2);
            txData[7] = *((uint8_t *)&DM4310_insts[i].control.rpm + 3);

            HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &DM4310_txHeader, txData);
            break;
        default:
            break;
        }
    }

}

void DM4310_init()
{
    DM4310_insts[0].motorID = 1;
    DM4310_insts[1].motorID = 2;

    DM4310_insts[0].control.rpm = 2;
    DM4310_insts[1].control.rpm = 2;

    FDCAN_FilterTypeDef filter =   {FDCAN_STANDARD_ID,
                                    FDCAN_DATA_FRAME,
                                    FDCAN_FILTER_MASK,
                                    FDCAN_FILTER_TO_RXFIFO0,
                                    0,
                                    0
                                    };
    HAL_FDCAN_ConfigFilter(&hfdcan1, &filter);
    HAL_FDCAN_RegisterRxFifo0Callback(&hfdcan1, rxFifoCallback);
    HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE | FDCAN_IT_RX_FIFO1_NEW_MESSAGE, (uint32_t)NULL);
    HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_REJECT, FDCAN_REJECT, FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE);
    HAL_FDCAN_Start(&hfdcan1);
}

