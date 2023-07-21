#pragma once

#define FNV_PRIME           16777619
#define FNV_OFFSET_BASIS    2166136261

uint32_t fnv_update(uint32_t hash, const void *data, uint32_t length);