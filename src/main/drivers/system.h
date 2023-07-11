#pragma once

#include <stdint.h>
#include <stdbool.h>

void systemInit(void);

// bootloader/IAP
void systemReset(void);
void cycleCounterInit(void);

// memory
void memProtReset(void);
void memProtConfigure(void);
void initialiseMemorySections(void);