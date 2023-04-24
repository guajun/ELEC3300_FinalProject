#include "SDCard.h"

#include "ff.h"
#include "fatfs.h"
#include <string.h>

FRESULT res; /* FatFs function common result code */
uint32_t byteswritten;
uint8_t wtext[] = "STM32 FATFS works great!";


void SDCard_init()
{
    res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 1);

    res = f_open(&SDFile, "STM32.TXT", FA_CREATE_ALWAYS | FA_WRITE);

    res = f_write(&SDFile, wtext, strlen((char *)wtext), (void *)&byteswritten);

    res = f_close(&SDFile);

    // res = f_mount(&SDFatFS, (TCHAR const*)NULL, 0);
}
