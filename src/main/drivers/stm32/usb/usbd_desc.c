/**
  ******************************************************************************
  * @file           : App/usbd_desc.c
  * @version        : v1.0_Cube
  * @brief          : This file implements the USB device descriptors.
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
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_conf.h"
#include "platform.h"
#include "build/version.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define USBD_VID                      0x0483
#define USBD_PID                      0x572A
#define USBD_LANGID_STRING            0x409
#define USBD_MANUFACTURER_STRING      PROJECT_NAME

#define USBD_PRODUCT_HS_STRING        "Anti-Bear Mass Storage in HS Mode"

#define USBD_CONFIGURATION_HS_STRING  "MSC Config"
#define USBD_INTERFACE_HS_STRING      "MSC Interface"

#define USB_SIZ_BOS_DESC              0x0C
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
uint8_t *USBD_HS_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t *USBD_HS_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t *USBD_HS_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t *USBD_HS_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t *USBD_HS_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t *USBD_HS_ConfigStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t *USBD_HS_InterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);

/* Private variables ---------------------------------------------------------*/
USBD_DescriptorsTypeDef HS_Desc = {
    USBD_HS_DeviceDescriptor,
    USBD_HS_LangIDStrDescriptor,
    USBD_HS_ManufacturerStrDescriptor,
    USBD_HS_ProductStrDescriptor,
    USBD_HS_SerialStrDescriptor,
    USBD_HS_ConfigStrDescriptor,
    USBD_HS_InterfaceStrDescriptor,
};

/* USB Standard Device Descriptor */
#if defined( __ICCARM__ ) /* IAR Compiler */
    #pragma data_alignment=4
#endif
__ALIGN_BEGIN uint8_t USBD_HS_DeviceDesc[USB_LEN_DEV_DESC] __ALIGN_END = {
    0x12,                           /* bLength */
    USB_DESC_TYPE_DEVICE,           /* bDescriptorType */
    0x00,                           /* bcdUSB */
    0x02,
    0x00,                           /* bDeviceClass */
    0x00,                           /* bDeviceSubClass */
    0x00,                           /* bDeviceProtocol */
    USB_MAX_EP0_SIZE,               /* bMaxPacketSize */
    LOBYTE(USBD_VID),               /* idVendor */
    HIBYTE(USBD_VID),               /* idVendor */
    LOBYTE(USBD_PID),               /* idProduct */
    HIBYTE(USBD_PID),               /* idProduct */
    0x00,                           /* bcdDevice rel. 2.00 */
    0x02,
    USBD_IDX_MFC_STR,               /* Index of manufacturer string */
    USBD_IDX_PRODUCT_STR,           /* Index of product string */
    USBD_IDX_SERIAL_STR,            /* Index of serial number string */
    USBD_MAX_NUM_CONFIGURATION      /* bNumConfigurations */
};

/* BOS descriptor. */
#if (USBD_LPM_ENABLED == 1)
#if defined( __ICCARM__ ) /* IAR Compiler */
    #pragma data_alignment=4
#endif
__ALIGN_BEGIN uint8_t USBD_HS_BOSDesc[USB_SIZ_BOS_DESC] __ALIGN_END =
{
    0x05,
    USB_DESC_TYPE_BOS,
    0x0C,
    0x00,
    0x01,  /* 1 device capability */
           /* device capability */
    0x07,
    USB_DEVICE_CAPABITY_TYPE,
    0x02,
    0x02,  /*LPM capability bit set */
    0x00,
    0x00,
    0x00
};
#endif

/* USB lang identifier descriptor */
#if defined( __ICCARM__ ) /* IAR Compiler */
    #pragma data_alignment=4
#endif
__ALIGN_BEGIN uint8_t USBD_LangIDDesc[USB_LEN_LANGID_STR_DESC] __ALIGN_END = {
    USB_LEN_LANGID_STR_DESC,
    USB_DESC_TYPE_STRING,
    LOBYTE(USBD_LANGID_STRING),
    HIBYTE(USBD_LANGID_STRING)
};

/* USB string serial descriptor */
#if defined( __ICCARM__ ) /* IAR Compiler */
    #pragma data_alignment=4
#endif
__ALIGN_BEGIN uint8_t USBD_StringSerial[USB_SIZ_STRING_SERIAL] __ALIGN_END = {
    USB_SIZ_STRING_SERIAL,
    USB_DESC_TYPE_STRING,
};

/* Internal string descriptor */
#if defined( __ICCARM__ ) /* IAR Compiler */
    #pragma data_alignment=4
#endif
__ALIGN_BEGIN uint8_t USBD_StrDesc[USBD_MAX_STR_DESC_SIZ] __ALIGN_END;

/* Private functions ---------------------------------------------------------*/
static void Get_SerialNum(void);
static void IntToUnicode(uint32_t value, uint8_t * pbuf, uint8_t len);

/**
 * @brief  Return the device descriptor
 * @param  speed : Current device speed
 * @param  length : Pointer to data length variable
 * @retval Pointer to descriptor buffer
 */
uint8_t *USBD_HS_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    UNUSED(speed);
    *length = sizeof(USBD_HS_DeviceDesc);
    return USBD_HS_DeviceDesc;
}

/**
 * @brief  Return the LangID string descriptor
 * @param  speed : Current device speed
 * @param  length : Pointer to data length variable
 * @retval Pointer to descriptor buffer
 */
uint8_t *USBD_HS_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    UNUSED(speed);
    *length = sizeof(USBD_LangIDDesc);
    return USBD_LangIDDesc;
}

/**
 * @brief  Return the product string descriptor
 * @param  speed : current device speed
 * @param  length : pointer to data length variable
 * @retval pointer to descriptor buffer
 */
uint8_t *USBD_HS_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    if (speed == 0)
    {
        USBD_GetString((uint8_t *)USBD_PRODUCT_HS_STRING, USBD_StrDesc, length);
    }
    else
    {
        USBD_GetString((uint8_t *)USBD_PRODUCT_HS_STRING, USBD_StrDesc, length);
    }
    return USBD_StrDesc;
}

/**
 * @brief  Return the manufacturer string descriptor
 * @param  speed : Current device speed
 * @param  length : Pointer to data length variable
 * @retval Pointer to descriptor buffer
 */
uint8_t *USBD_HS_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    UNUSED(speed);
    USBD_GetString((uint8_t *)USBD_MANUFACTURER_STRING, USBD_StrDesc, length);
    return USBD_StrDesc;
}

/**
 * @brief  Return the serial number string descriptor
 * @param  speed : Current device speed
 * @param  length : Pointer to data length variable
 * @retval Pointer to descriptor buffer
 */
uint8_t *USBD_HS_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    UNUSED(speed);
    *length = USB_SIZ_STRING_SERIAL;

    /* Update the serial number string descriptor with the data from the unique ID */
    Get_SerialNum();

    return (uint8_t *)USBD_StringSerial;
}

/**
 * @brief  Return the configuration string descriptor
 * @param  speed : Current device speed
 * @param  length : Pointer to data length variable
 * @retval Pointer to descriptor buffer
 */
uint8_t *USBD_HS_ConfigStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    if(speed == USBD_SPEED_HIGH)
    {
        USBD_GetString((uint8_t *)USBD_CONFIGURATION_HS_STRING, USBD_StrDesc, length);
    }
    else
    {
        USBD_GetString((uint8_t *)USBD_CONFIGURATION_HS_STRING, USBD_StrDesc, length);
    }
    return USBD_StrDesc;
}

/**
 * @brief  Return the interface string descriptor
 * @param  speed : Current device speed
 * @param  length : Pointer to data length variable
 * @retval Pointer to descriptor buffer
 */
uint8_t *USBD_HS_InterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    if(speed == 0)
    {
        USBD_GetString((uint8_t *)USBD_INTERFACE_HS_STRING, USBD_StrDesc, length);
    }
    else
    {
        USBD_GetString((uint8_t *)USBD_INTERFACE_HS_STRING, USBD_StrDesc, length);
    }
    return USBD_StrDesc;
}

#if (USBD_LPM_ENABLED == 1)
/**
 * @brief  Return the BOS descriptor
 * @param  speed : Current device speed
 * @param  length : Pointer to data length variable
 * @retval Pointer to descriptor buffer
 */
uint8_t *USBD_HS_USR_BOSDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    UNUSED(speed);
    *length = sizeof(USBD_HS_BOSDesc);
    return (uint8_t*)USBD_HS_BOSDesc;
}
#endif /* (USBD_LPM_ENABLED == 1) */

/**
 * @brief  Create the serial number string descriptor
 * @param  None
 * @retval None
 */
static void Get_SerialNum(void)
{
    uint32_t deviceserial0, deviceserial1, deviceserial2;

    deviceserial0 = U_ID_0;
    deviceserial1 = U_ID_1;
    deviceserial2 = U_ID_2;

    deviceserial0 += deviceserial2;

    if (deviceserial0 != 0)
    {
        IntToUnicode(deviceserial0, &USBD_StringSerial[2], 8);
        IntToUnicode(deviceserial1, &USBD_StringSerial[18], 4);
    }
}

/**
 * @brief  Convert Hex 32Bits value into char
 * @param  value: value to convert
 * @param  pbuf: pointer to the buffer
 * @param  len: buffer length
 * @retval None
 */
static void IntToUnicode(uint32_t value, uint8_t *pbuf, uint8_t len)
{
    uint8_t idx = 0;

    for (idx = 0; idx < len; idx++)
    {
        if (((value >> 28)) < 0xA)
        {
            pbuf[2 * idx] = (value >> 28) + '0';
        }
        else
        {
            pbuf[2 * idx] = (value >> 28) + 'A' - 10;
        }

        value = value << 4;

        pbuf[2 * idx + 1] = 0;
    }
}
