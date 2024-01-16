COMMON_SRC = \
            build/version.c \
            build/debug.c \
            $(TARGET_DIR_SRC) \
            main.c \
            $(addprefix pg/, $(notdir $(wildcard $(SRC_DIR)/pg/*.c))) \
            $(addprefix common/,$(notdir $(wildcard $(SRC_DIR)/common/*.c))) \
            $(addprefix config/,$(notdir $(wildcard $(SRC_DIR)/config/*.c))) \
            drivers/system.c \
            core/init.c

FLASH_SRC += \
            drivers/flash.c \
            drivers/flash_w25q256jv.c

FREERTOS_DIR = \
            $(ROOT)/lib/main/FreeRTOS/Source \
            $(ROOT)/lib/main/FreeRTOS/Source/include \
            $(ROOT)/lib/main/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 \
            $(ROOT)/lib/main/FreeRTOS/Source/portable/MemMang \
            $(ROOT)/src/main/FreeRTOS

FREERTOS_SRC = \
            $(notdir $(wildcard $(ROOT)/lib/main/FreeRTOS/Source/*.c)) \
            $(notdir $(wildcard $(ROOT)/lib/main/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1/*.c)) \
            $(ROOT)/lib/main/FreeRTOS/Source/portable/MemMang/heap_4.c \
            FreeRTOS/FreeRTOSRetarget.c \
            FreeRTOS/FreeRTOSApp.c


INCLUDE_DIRS    := $(INCLUDE_DIRS) \
                   $(FREERTOS_DIR)
VPATH           := $(VPATH):$(FREERTOS_DIR)

FATFS_DIR = \
            $(ROOT)/lib/main/FatFS/source \
            $(ROOT)/src/main/FatFS

FATFS_SRC = \
            $(notdir $(wildcard $(ROOT)/lib/main/FatFs/Source/*.c)) \
            FatFs/mmc.c \
            FatFs/mmc_qspi_flash.c \
            FatFs/FatFs_app.c

EXCLUDES  = $(ROOT)/lib/main/FatFS/source/Templates/diskio.c

FATFS_SRC := $(filter-out ${EXCLUDES}, $(FATFS_SRC))

INCLUDE_DIRS    := $(INCLUDE_DIRS) \
                   $(FATFS_DIR)
VPATH           := $(VPATH):$(FATFS_DIR)

COMMON_DEVICE_SRC = \
            $(CMSIS_SRC) \
            $(DEVICE_STDPERIPH_SRC)

COMMON_SRC := $(COMMON_SRC) $(COMMON_DEVICE_SRC)

SPEED_OPTIMISED_SRC := ""
SIZE_OPTIMISED_SRC  := ""

SPEED_OPTIMISED_SRC := $(SPEED_OPTIMISED_SRC) \
            drivers/system.c \
            $(CMSIS_SRC) \
            $(DEVICE_STDPERIPH_SRC) \

SIZE_OPTIMISED_SRC := $(SIZE_OPTIMISED_SRC) \
            core/init.c \
            config/config_streamer.c \

# check if target.mk supplied
SRC := $(STARTUP_SRC) $(MCU_COMMON_SRC)

# Files that should not be optimized, useful for debugging IMPRECISE cpu faults.
# Specify FULL PATH, e.g. "./lib/STM32H7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_sdmmc.c"
NOT_OPTIMISED_SRC := $(NOT_OPTIMISED_SRC) \

ifneq ($(DSP_LIB),)

INCLUDE_DIRS += $(DSP_LIB)/Include
SRC += $(wildcard $(DSP_LIB)/Source/*/*.S)

endif

SRC += $(FLASH_SRC) $(FREERTOS_SRC) $(FATFS_SRC) $(COMMON_SRC)

#excludes
SRC   := $(filter-out $(MCU_EXCLUDES), $(SRC))

SRC += $(USB_SRC)

# end target specific make file checks
