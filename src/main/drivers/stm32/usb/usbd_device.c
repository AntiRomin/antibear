#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "platform.h"

#include "usbd_core.h"
#include "usbd_msc.h"
#include "drivers/time.h"
#include "drivers/stm32/usb/usbd_io.h"
#include "drivers/stm32/usb/usbd_storage_if.h"
#include "drivers/stm32/usb/usbd_desc.h"

#include "drivers/stm32/usb/usbd_device.h"

USBD_HandleTypeDef USBD_HS_Device;

static void usbGenerateDisconnectPulse(void)
{
    /* pull down USB OTG HS DP to create USB disconnect pulse */
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin     = USB_OTG_HS_DP_Pin;
    GPIO_InitStruct.Mode    = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Speed   = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pull    = GPIO_NOPULL;
    HAL_GPIO_Init(USB_OTG_HS_DP_GPIO_Port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(USB_OTG_HS_DP_GPIO_Port, USB_OTG_HS_DP_Pin, GPIO_PIN_RESET);

    delay(200);

    HAL_GPIO_WritePin(USB_OTG_HS_DP_GPIO_Port, USB_OTG_HS_DP_Pin, GPIO_PIN_SET);
}

uint8_t usbStart(void)
{
    // Start USB
    usbGenerateDisconnectPulse();

    USBD_Init(&USBD_HS_Device, &HS_Desc, DEVICE_HS);

    // Register class
    USBD_RegisterClass(&USBD_HS_Device, USBD_MSC_CLASS);

    // Register interface callbacks
    USBD_MSC_RegisterStorage(&USBD_HS_Device, &USBD_Storage_Interface_HS_fops);

    USBD_Start(&USBD_HS_Device);

    HAL_PWREx_EnableUSBVoltageDetector();
    delay(100); // Cold boot failures observed without this, even when USB cable is not connected

    return 0;
}