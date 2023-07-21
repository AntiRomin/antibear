#pragma once

#include <stdint.h>
#include <stdbool.h>

// Streams data out to the EEPROM, padding to the write size as
// needed, and updating the checksum as it goes.

#if defined(STM32H743xx)
#define CONFIG_STREAMER_BUFFER_SIZE 32  // Flash word = 256-bits
typedef uint64_t config_streamer_buffer_align_type_t;
#endif

typedef struct config_streamer_s {
    uintptr_t address;
    int size;
    union {
        uint8_t b[CONFIG_STREAMER_BUFFER_SIZE];
        config_streamer_buffer_align_type_t w;
    } buffer;
    int at;
    int err;
    bool unlocked;
} config_streamer_t;

void config_streamer_init(config_streamer_t *c);

void config_streamer_start(config_streamer_t *c, uintptr_t base, uint32_t size);
int config_streamer_write(config_streamer_t *c, const uint8_t *p, uint32_t size);
int config_streamer_flush(config_streamer_t *c);

int config_streamer_finish(config_streamer_t *c);
int config_streamer_status(config_streamer_t *c);