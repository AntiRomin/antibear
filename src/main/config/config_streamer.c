#include <string.h>

#include "platform.h"

#include "drivers/system.h"

#include "config/config_streamer.h"

#if !defined(FLASH_PAGE_SIZE)
#error "Flash page size not defined for target."
#endif

void config_streamer_init(config_streamer_t *c)
{
    memset(c, 0, sizeof(*c));
}

void config_streamer_start(config_streamer_t *c, uintptr_t base, uint32_t size)
{
    // base must start at FLASH_PAGE_SIZE boundary when using embedded flash.
    c->address = base;
    c->size = size;
    if (!c->unlocked) {
        HAL_FLASH_Unlock();
        c->unlocked = true;
    }

    c->err = 0;
}

/*
MCUs with uniform array of equal size sectors, handled in two banks having contiguous address.
(Devices with non-contiguous flash layout is not currently useful anyways.)

H743
2 bank * 8 sector/bank * 128K/sector (2MB)
Bank 1 0x08000000 - 0x080FFFFF 128KB * 8
Bank 2 0x08100000 - 0x081FFFFF 128KB * 8

Note that FLASH_BANK_SIZE constant used in the following code changes depending on
bank operation mode. The code assumes dual bank operation, in which case the
FLASH_BANK_SIZE constant is set to one half of the available flash size in HAL.
*/

#if defined(STM32H743xx)
#define FLASH_PAGE_PER_BANK 8
#endif

static void getFLASHSectorForEEPROM(uint32_t address, uint32_t *bank, uint32_t *sector)
{
    if (address >= FLASH_BANK1_BASE && address < FLASH_BANK1_BASE + FLASH_BANK_SIZE) {
        *bank = FLASH_BANK_1;
    } else {
        // Not good
        while (1) {
            // TODO: Handle Error
        }
    }

    address -= FLASH_BANK1_BASE;
    *sector = address / FLASH_PAGE_SIZE;
}

static int write_word(config_streamer_t *c, config_streamer_buffer_align_type_t *buffer)
{
    if (c->err != 0) {
        return c->err;
    }

    if (c->address % FLASH_PAGE_SIZE == 0) {
        FLASH_EraseInitTypeDef EraseInitStruct = {
            .TypeErase    = FLASH_TYPEERASE_SECTORS,
            .VoltageRange = FLASH_VOLTAGE_RANGE_3, // 2.7-3.6V
            .NbSectors    = 1
        };
        getFLASHSectorForEEPROM(c->address, &EraseInitStruct.Banks, &EraseInitStruct.Sector);
        uint32_t SECTORError;
        const HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError);
        if (status != HAL_OK) {
            return -1;
        }
    }

    const HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, c->address, (uint64_t)(uint32_t)buffer);
    if (status != HAL_OK) {
        return -2;
    }

    c->address += CONFIG_STREAMER_BUFFER_SIZE;
    return 0;
}

int config_streamer_write(config_streamer_t *c, const uint8_t *p, uint32_t size)
{
    for (const uint8_t *pat = p; pat != (uint8_t *)p + size; pat++) {
        c->buffer.b[c->at++] = *pat;

        if (c->at == sizeof(c->buffer)) {
            c->err = write_word(c, &c->buffer.w);
            c->at = 0;
        }
    }
    return c->err;
}

int config_streamer_status(config_streamer_t *c)
{
    return c->err;
}

int config_streamer_flush(config_streamer_t *c)
{
    if (c->at != 0) {
        memset(c->buffer.b + c->at, 0, sizeof(c->buffer) - c->at);
        c->err = write_word(c, &c->buffer.w);
        c->at = 0;
    }
    return c->err;
}

int config_streamer_finish(config_streamer_t *c)
{
    if (c->unlocked) {
        HAL_FLASH_Lock();
        c->unlocked = false;
    }
    return c->err;
}