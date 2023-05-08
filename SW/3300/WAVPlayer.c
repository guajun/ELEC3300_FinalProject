#include "WAVPlayer.h"

// #include "ff.h"
// #include "fatfs.h"
#include "i2s.h"
#include "stdint.h"
#include "sdmmc.h"

#define WAV_PLAYER_BLOCK_NBR 12
#define WAV_PLAYER_BUFFER_SIZE (512 * WAV_PLAYER_BLOCK_NBR)

#define SONG_1_BLOCK_ADDRESS 65600


__attribute__((section(".D1"))) uint8_t WAVPlayer_txData[WAV_PLAYER_BUFFER_SIZE] = {};
__attribute__((section(".D1"))) uint8_t WAVPlayer_txData2[WAV_PLAYER_BUFFER_SIZE] = {};


// uint8_t currentBuffer = 0;

// uint32_t fileBytesRead = 0;
// uint32_t * pFileBytesRead = &fileBytesRead;

uint32_t WAVPlayer_readIndex = SONG_1_BLOCK_ADDRESS;
uint32_t WAVPlayer_dataSize = 0;

uint8_t WAVPlayer_initialized = 0;

void getBuffer(uint8_t * buffer)
{
    HAL_SD_ReadBlocks_DMA(&hsd1, buffer, WAVPlayer_readIndex, WAV_PLAYER_BLOCK_NBR);
    WAVPlayer_readIndex += WAV_PLAYER_BLOCK_NBR;
}

void checkLength()
{
    if(WAVPlayer_dataSize < ((WAVPlayer_readIndex - SONG_1_BLOCK_ADDRESS) << 9) )
    {
      HAL_I2S_DMAStop(&hi2s1);
    }
}

static void WAVPlayer_i2sM0TxCpltCallback(DMA_HandleTypeDef *hdma)
{
    getBuffer(WAVPlayer_txData2);
    checkLength();
}

static void WAVPlayer_i2sM1TxCpltCallback(DMA_HandleTypeDef *hdma)
{
    getBuffer(WAVPlayer_txData);
    checkLength();
}

/**
  * @brief  DMA I2S transmit process half complete callback
  * @param  hdma pointer to a DMA_HandleTypeDef structure that contains
  *         the configuration information for the specified DMA module.
  * @retval None
  */
static void I2S_DMATxHalfCplt(DMA_HandleTypeDef *hdma)
{
  /* Derogation MISRAC2012-Rule-11.5 */
  I2S_HandleTypeDef *hi2s = (I2S_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

  /* Call user Tx half complete callback */
#if (USE_HAL_I2S_REGISTER_CALLBACKS == 1UL)
  hi2s->TxHalfCpltCallback(hi2s);
#else
  HAL_I2S_TxHalfCpltCallback(hi2s);
#endif /* USE_HAL_I2S_REGISTER_CALLBACKS */
}

/**
  * @brief  DMA I2S communication error callback
  * @param  hdma pointer to a DMA_HandleTypeDef structure that contains
  *         the configuration information for the specified DMA module.
  * @retval None
  */
static void I2S_DMAError(DMA_HandleTypeDef *hdma)
{
  /* Derogation MISRAC2012-Rule-11.5 */
  I2S_HandleTypeDef *hi2s = (I2S_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

  /* Disable Rx and Tx DMA Request */
  CLEAR_BIT(hi2s->Instance->CFG1, (SPI_CFG1_RXDMAEN | SPI_CFG1_TXDMAEN));
  hi2s->TxXferCount = (uint16_t) 0UL;
  hi2s->RxXferCount = (uint16_t) 0UL;

  hi2s->State = HAL_I2S_STATE_READY;

  /* Set the error code and execute error callback*/
  SET_BIT(hi2s->ErrorCode, HAL_I2S_ERROR_DMA);
  /* Call user error callback */
#if (USE_HAL_I2S_REGISTER_CALLBACKS == 1UL)
  hi2s->ErrorCallback(hi2s);
#else
  HAL_I2S_ErrorCallback(hi2s);
#endif /* USE_HAL_I2S_REGISTER_CALLBACKS */
}

HAL_StatusTypeDef HAL_I2S_Transmit_DMA_Doublebuffer(I2S_HandleTypeDef *hi2s, uint16_t *pData, uint16_t *pData2, uint16_t Size)
{
  HAL_StatusTypeDef errorcode = HAL_OK;

  if ((pData == NULL) || (Size == 0UL))
  {
    return  HAL_ERROR;
  }

  if (hi2s->State != HAL_I2S_STATE_READY)
  {
    return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hi2s);

  /* Set state and reset error code */
  hi2s->State       = HAL_I2S_STATE_BUSY_TX;
  hi2s->ErrorCode   = HAL_I2S_ERROR_NONE;
  hi2s->pTxBuffPtr  = pData;
  hi2s->TxXferSize  = Size;
  hi2s->TxXferCount = Size;

  /* Init field not used in handle to zero */
  hi2s->pRxBuffPtr  = NULL;
  hi2s->RxXferSize  = (uint16_t)0UL;
  hi2s->RxXferCount = (uint16_t)0UL;

  /* Set the I2S Tx DMA Half transfer complete callback */
  hi2s->hdmatx->XferHalfCpltCallback = I2S_DMATxHalfCplt;

  /* Set the I2S Tx DMA transfer complete callback */
  hi2s->hdmatx->XferCpltCallback = WAVPlayer_i2sM0TxCpltCallback;

  /* Set the I2S Tx DMA Half transfer complete callback */
  hi2s->hdmatx->XferM1HalfCpltCallback = I2S_DMATxHalfCplt;

  /* Set the I2S Tx DMA transfer complete callback */
  hi2s->hdmatx->XferM1CpltCallback = WAVPlayer_i2sM1TxCpltCallback;

  /* Set the DMA error callback */
  hi2s->hdmatx->XferErrorCallback = I2S_DMAError;

  /* Enable the Tx DMA Stream/Channel */
  if (HAL_OK != HAL_DMAEx_MultiBufferStart_IT(hi2s->hdmatx, 
                                            (uint32_t)hi2s->pTxBuffPtr, 
                                            (uint32_t)&hi2s->Instance->TXDR,
                                            (uint32_t)pData2, 
                                            hi2s->TxXferCount))
  {
    /* Update I2S error code */
    SET_BIT(hi2s->ErrorCode, HAL_I2S_ERROR_DMA);
    hi2s->State = HAL_I2S_STATE_READY;

    __HAL_UNLOCK(hi2s);
    errorcode = HAL_ERROR;
    return errorcode;
  }

  /* Check if the I2S Tx request is already enabled */
  if (HAL_IS_BIT_CLR(hi2s->Instance->CFG1, SPI_CFG1_TXDMAEN))
  {
    /* Enable Tx DMA Request */
    SET_BIT(hi2s->Instance->CFG1, SPI_CFG1_TXDMAEN);
  }

  /* Check if the I2S is already enabled */
  if (HAL_IS_BIT_CLR(hi2s->Instance->CR1, SPI_CR1_SPE))
  {
    /* Enable I2S peripheral */
    __HAL_I2S_ENABLE(hi2s);
  }

  /* Start the transfer */
  SET_BIT(hi2s->Instance->CR1, SPI_CR1_CSTART);

  __HAL_UNLOCK(hi2s);
  return errorcode;
}

void WAVPlayer_i2sInitTxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    // currentBuffer = !currentBuffer;
    // if(currentBuffer)
    // {
    //     // send 2, write 1
    //     HAL_I2S_Transmit_DMA(&hi2s1, (uint16_t *)WAVPlayer_txData2, WAV_PLAYER_BUFFER_SIZE >> 1);
    //     getBuffer(WAVPlayer_txData);
    // }
    // else
    // {
    //     // send 1, write 2
    //     HAL_I2S_Transmit_DMA(&hi2s1, (uint16_t *)WAVPlayer_txData, WAV_PLAYER_BUFFER_SIZE >> 1);
    //     getBuffer(WAVPlayer_txData2);
    // }

    HAL_I2S_Transmit_DMA_Doublebuffer(&hi2s1, (uint16_t *)WAVPlayer_txData2, (uint16_t *)WAVPlayer_txData, WAV_PLAYER_BUFFER_SIZE >> 1);
}

void WAVPlayer_sdRxCpltCallback(SD_HandleTypeDef *hsd)
{
    if(!WAVPlayer_initialized)
    {
        *( ((uint8_t *)&WAVPlayer_dataSize) + 0) = WAVPlayer_txData[0xA8];
        *( ((uint8_t *)&WAVPlayer_dataSize) + 1) = WAVPlayer_txData[0xA9];
        *( ((uint8_t *)&WAVPlayer_dataSize) + 2) = WAVPlayer_txData[0xAA];
        *( ((uint8_t *)&WAVPlayer_dataSize) + 3) = WAVPlayer_txData[0xAB];

        HAL_I2S_Transmit_DMA(&hi2s1, (uint16_t *)(WAVPlayer_txData + 44), (WAV_PLAYER_BUFFER_SIZE - 44) >> 1);

        getBuffer(WAVPlayer_txData2);
        WAVPlayer_initialized = !WAVPlayer_initialized;
    }

}

void WAVPlayer_init()
{
    HAL_I2S_RegisterCallback(&hi2s1, HAL_I2S_TX_COMPLETE_CB_ID, WAVPlayer_i2sInitTxCpltCallback);
    HAL_SD_RegisterCallback(&hsd1, HAL_SD_RX_CPLT_CB_ID, WAVPlayer_sdRxCpltCallback);
}

void WAVPlayer_play()
{
    HAL_I2S_DMAStop(&hi2s1);
    // currentBuffer = 0;
    getBuffer(WAVPlayer_txData);
}