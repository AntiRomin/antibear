/**
  ******************************************************************************
  * @file           : usbd_storage_if.c
  * @version        : v1.0_Cube
  * @brief          : Memory management layer.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "platform.h"

#include "usbd_storage_if.h"

#include "drivers/flash.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define STORAGE_LUN_NBR                  1
#define STORAGE_BLK_NBR                  0x10000
#define STORAGE_BLK_SIZ                  0x200

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* USB Mass storage Standard Inquiry Data. */
const uint8_t STORAGE_Inquirydata[] = { // 36
    /* LUN 0 */
    0x00,
    0x80,
    0x02,
    0x02,
    (STANDARD_INQUIRY_DATA_LEN - 5),
    0x00,
    0x00,
    0x00,
    'S', 'T', 'M', ' ', ' ', ' ', ' ', ' ', /* Manufacturer : 8 bytes */
    'P', 'r', 'o', 'd', 'u', 'c', 't', ' ', /* Product      : 16 Bytes */
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    '0', '.', '0' ,'1'                      /* Version      : 4 Bytes */
};

/* Private function prototypes -----------------------------------------------*/
/* Extern function prototypes ------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static int8_t STORAGE_Init(uint8_t lun);
static int8_t STORAGE_GetCapacity(uint8_t lun, uint32_t *block_num, uint16_t *block_size);
static int8_t STORAGE_IsReady(uint8_t lun);
static int8_t STORAGE_IsWriteProtected(uint8_t lun);
static int8_t STORAGE_Read(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
static int8_t STORAGE_Write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
static int8_t STORAGE_GetMaxLun(void);

USBD_StorageTypeDef USBD_Storage_Interface_HS_fops = {
    STORAGE_Init,
    STORAGE_GetCapacity,
    STORAGE_IsReady,
    STORAGE_IsWriteProtected,
    STORAGE_Read,
    STORAGE_Write,
    STORAGE_GetMaxLun,
    (int8_t *)STORAGE_Inquirydata,
};

/**
 * @brief  Initializes the storage unit (medium).
 * @param  lun: Logical unit number.
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */
int8_t STORAGE_Init(uint8_t lun)
{
    UNUSED(lun);
    // flash already init at init();

    return USBD_OK;
}

/**
 * @brief  Returns the medium capacity.
 * @param  lun: Logical unit number.
 * @param  block_num: Number of total block number.
 * @param  block_size: Block size.
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */
int8_t STORAGE_GetCapacity(uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
    UNUSED(lun);

    *block_num = flashGetGeometry(FLASH_ID_W25Q256JV)->sectors;
    *block_size = flashGetGeometry(FLASH_ID_W25Q256JV)->sectorSize;

    return USBD_OK;
}

/**
 * @brief   Checks whether the medium is ready.
 * @param  lun:  Logical unit number.
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */
int8_t STORAGE_IsReady(uint8_t lun)
{
    UNUSED(lun);

    if (flashIsReady(FLASH_ID_W25Q256JV))
    {
        return USBD_OK;
    }
    else
    {
        return USBD_FAIL;
    }
}

/**
 * @brief  Checks whether the medium is write protected.
 * @param  lun: Logical unit number.
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */
int8_t STORAGE_IsWriteProtected(uint8_t lun)
{
    UNUSED(lun);
    return USBD_OK;
}

/**
 * @brief  Reads data from the medium.
 * @param  lun: Logical unit number.
 * @param  buf: data buffer.
 * @param  blk_addr: Logical block address.
 * @param  blk_len: Blocks number.
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */
int8_t STORAGE_Read(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
    UNUSED(lun);
    flashReadBytes(FLASH_ID_W25Q256JV,
                   blk_addr * flashGetGeometry(FLASH_ID_W25Q256JV)->sectorSize,
                   buf,
                   blk_len * flashGetGeometry(FLASH_ID_W25Q256JV)->sectorSize);
    return USBD_OK;
}

/**
 * @brief  Writes data into the medium.
 * @param  lun: Logical unit number.
 * @param  buf: data buffer.
 * @param  blk_addr: Logical block address.
 * @param  blk_len: Blocks number.
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */
int8_t STORAGE_Write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
    UNUSED(lun);
    for (int i = 0; i < blk_len; i++) {
        flashEraseSector(FLASH_ID_W25Q256JV,
                         (blk_addr + i) * flashGetGeometry(FLASH_ID_W25Q256JV)->sectorSize);
        flashPageProgram(FLASH_ID_W25Q256JV,
                         (blk_addr + i) * flashGetGeometry(FLASH_ID_W25Q256JV)->sectorSize,
                         buf + (i * flashGetGeometry(FLASH_ID_W25Q256JV)->sectorSize),
                         flashGetGeometry(FLASH_ID_W25Q256JV)->sectorSize,
                         NULL);
    }
    return USBD_OK;
}

/**
 * @brief  Returns the Max Supported LUNs.
 * @param  None
 * @retval Lun(s) number.
 */
int8_t STORAGE_GetMaxLun(void)
{
    return (STORAGE_LUN_NBR - 1);
}