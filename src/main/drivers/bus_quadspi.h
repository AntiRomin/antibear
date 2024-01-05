#pragma once

void quadSpiInit(void);

bool quadSpiTransmit1LINE(uint8_t instruction, uint8_t dummyCycles, const uint8_t *out, int length);
bool quadSpiReceive1LINE(uint8_t instruction, uint8_t dummyCycles, uint8_t *in, int length);
bool quadSpiReceive4LINES(uint8_t instruction, uint8_t dummyCycles, uint8_t *in, int length);

bool quadSpiReceiveWithAddress1LINE(uint8_t instruction, uint8_t dummyCycles, uint32_t address, uint8_t addressSize, uint8_t *in, int length);
bool quadSpiReceiveWithAddress4LINES(uint8_t instruction, uint8_t dummyCycles, uint32_t address, uint8_t addressSize, uint8_t *in, int length);

bool quadSpiReceive4LINESWithAddress4LINES(uint8_t instruction, uint8_t dummyCycles, uint32_t address, uint8_t addressSize, uint8_t *in, int length);
bool quadSpiReceive4LINESWithAddressAndAlternate4LINES(uint8_t instruction, uint8_t dummyCycles, uint32_t address, uint8_t addressSize, uint32_t alternate, uint8_t alternateSize, uint8_t *in, int length);

bool quadSpiTransmitWithAddress1LINE(uint8_t instruction, uint8_t dummyCycles, uint32_t address, uint8_t addressSize, const uint8_t *out, int length);
bool quadSpiTransmitWithAddress4LINES(uint8_t instruction, uint8_t dummyCycles, uint32_t address, uint8_t addressSize, const uint8_t *out, int length);

bool quadSpiInstructionWithAddress1LINE(uint8_t instruction, uint8_t dummyCycles, uint32_t address, uint8_t addressSize);

bool quadSpiInstructionWithData1LINE(uint8_t instruction, uint8_t dummyCycles, const uint8_t *out, int length);