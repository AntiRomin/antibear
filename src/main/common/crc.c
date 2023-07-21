#include <stdint.h>

#include "common/crc.h"

#include "platform.h"

// Fowler–Noll–Vo hash function; see https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
uint32_t fnv_update(uint32_t hash, const void *data, uint32_t length)
{
    const uint8_t *p = (const uint8_t *)data;
    const uint8_t *pend = p + length;

    for (; p != pend; p++) {
        hash *= FNV_PRIME;
        hash ^= *p;
    }

    return hash;
}