#pragma once

#include "build/version.h"

#if !defined(DEBUG)
#define FAST_CODE                   __attribute__((section(".tcm_code"))) __attribute__((optimize("-O2", "-freorder-blocks-algorithm=simple")))
#else
#define FAST_CODE                   __attribute__((section(".tcm_code")))
#endif

#define FAST_DATA_ZERO_INIT         __attribute__ ((section(".fastram_bss"), aligned(4)))
#define FAST_DATA                   __attribute__ ((section(".fastram_data"), aligned(4)))

#define SDRAM2_DATA_ZERO_INIT       __attribute__ ((section(".sdram2_bss"), aligned(4)))
#define SDRAM2_DATA                 __attribute__ ((section(".sdram2_data"), aligned(4)))