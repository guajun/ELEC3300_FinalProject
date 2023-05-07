#include "HT4310.h"

#include "stdint.h"
#include "fdcan.h"

FDCAN_TxHeaderTypeDef HT4310_txHeader = {
        0x001, FDCAN_STANDARD_ID, FDCAN_DATA_FRAME, FDCAN_DLC_BYTES_8, FDCAN_ESI_PASSIVE, FDCAN_BRS_OFF, FDCAN_CLASSIC_CAN, FDCAN_NO_TX_EVENTS, 0};

HT4310 HT4310_insts[HT4310_NUM];

static void rxFifoCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t rxFifo0ITs)
{
    while (HAL_FDCAN_GetRxFifoFillLevel(hfdcan, FDCAN_RX_FIFO0))
    {
        static uint8_t rxData[8];
        static FDCAN_RxHeaderTypeDef rxHeader;
        HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rxHeader, rxData);
    }
}
void HT4310_update()
{

    for(uint8_t i = 0; i < HT4310_NUM; ++i)
    {
        switch (HT4310_insts[i].state)
        {
        case HT4310_INIT:

            uint8_t txData[8] = {};
            HT4310_txHeader.Identifier = (0x21 << 4) | (HT4310_insts[i].motorID); // Reset pos
            HT4310_txHeader.DataLength = FDCAN_DLC_BYTES_0;
            HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &HT4310_txHeader, txData);
            HT4310_insts[i].state = HT4310_RUNNING;

        case HT4310_RUNNING:
            HT4310_txHeader.Identifier = (0x55 << 4) | (HT4310_insts[i].motorID); // set target pos
            HT4310_txHeader.DataLength = FDCAN_DLC_BYTES_4;

            txData[0] = *((uint8_t *)&HT4310_insts[i].control.position);
            txData[1] = *((uint8_t *)&HT4310_insts[i].control.position + 1);
            txData[2] = *((uint8_t *)&HT4310_insts[i].control.position + 2);
            txData[3] = *((uint8_t *)&HT4310_insts[i].control.position + 3);

            HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &HT4310_txHeader, txData);
            break;
        default:
            break;
        }
    }

}
void HT4310_init()
{
    HT4310_insts[0].motorID = 1;
    HT4310_insts[1].motorID = 2;

    FDCAN_FilterTypeDef filter =   {FDCAN_STANDARD_ID,
                                    FDCAN_DATA_FRAME,
                                    FDCAN_FILTER_MASK,
                                    FDCAN_FILTER_TO_RXFIFO0,
                                    0,
                                    0
                                    };

    HAL_FDCAN_ConfigFilter(&hfdcan2, &filter);
    HAL_FDCAN_RegisterRxFifo0Callback(&hfdcan2, rxFifoCallback);
    HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO0_NEW_MESSAGE | FDCAN_IT_RX_FIFO1_NEW_MESSAGE, (uint32_t)NULL);
    HAL_FDCAN_ConfigGlobalFilter(&hfdcan2, FDCAN_REJECT, FDCAN_REJECT, FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE);
    HAL_FDCAN_Start(&hfdcan2);
}