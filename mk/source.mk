COMMON_SRC = \
            build/version.c \
            $(TARGET_DIR_SRC) \
            main.c \
            $(addprefix pg/, $(notdir $(wildcard $(SRC_DIR)/pg/*.c))) \
            $(addprefix common/,$(notdir $(wildcard $(SRC_DIR)/common/*.c))) \
            $(addprefix config/,$(notdir $(wildcard $(SRC_DIR)/config/*.c))) \
            drivers/system.c \
            core/init.c

FLASH_SRC += \
            drivers/flash_w25q256jv.c \

FATFS_SRC += \
            FatFs/diskio.c \
            FatFs/diskio_quad_flash.c

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

SRC += $(FLASH_SRC) $(FATFS_SRC) $(COMMON_SRC)

#excludes
SRC   := $(filter-out $(MCU_EXCLUDES), $(SRC))

# end target specific make file checks
