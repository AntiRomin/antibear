#pragma once

extern uint8_t debugMode;

void debugInit(void);

uint32_t debugTotalRxBytesWaiting(void);
uint32_t debugTotalTxBytesFree(void);
bool debugIsTxBufferEmpty(void);

uint8_t debugRead(void);
void debugWrite(uint8_t ch);
void debugPrint(const char *str);
void debugPrintCount(const char *str, size_t count);