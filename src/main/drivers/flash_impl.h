#pragma once

struct flashVTable_s;

typedef struct flashDevice_s {
    //
    // members to be configured by the flash chip implementation
    //

    const struct flashVTable_s *vTable;
    flashGeometry_t geometry;
    uint32_t currentWriteAddress;
    bool isLargeFlash;
    // Whether we've performed an action that could have made the device busy
    // for writes. This allows us to avoid polling for writable status
    // when it is definitely read already
    bool couldBeBusy;
    uint32_t timeoutAt;

    //
    // members configured by tht flash detection system, read-only from the flash chip implementation's perspective.
    //

    void (*callback)(uint32_t arg);
    uint32_t callbackArg;
} flashDevice_t;

typedef struct flashVTable_s {
    void (*configure)(flashDevice_t *fdevice, uint32_t configurationFlags);

    bool (*isReady)(flashDevice_t *fdevice);
    bool (*waitForReady)(flashDevice_t *fdevice);

    void (*eraseSector)(flashDevice_t *fdevice, uint32_t address);
    void (*eraseCompletely)(flashDevice_t *fdevice);

    void (*pageProgramBegin)(flashDevice_t *fdevice, uint32_t address, void (*callback)(uint32_t length));
    uint32_t (*pageProgramContinue)(flashDevice_t *fdevice, uint8_t const **buffer, uint32_t *bufferSizes, uint32_t bufferCount);
    void (*pageProgramFinish)(flashDevice_t *fdevice);
    void (*pageProgram)(flashDevice_t *fdevice, uint32_t address, const uint8_t *data, uint32_t length, void (*callback)(uint32_t length));

    void (*flush)(flashDevice_t *fdevice);

    int (*readBytes)(flashDevice_t *fdevice, uint32_t address, uint8_t *buffer, uint32_t length);

    const flashGeometry_t *(*getGeometry)(flashDevice_t *fdevice);
} flashVTable_t;