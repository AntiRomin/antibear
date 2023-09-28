#
# H7 Make file include
#

ifeq ($(DEBUG_HARDFAULTS),H7)
CFLAGS          += -DDEBUG_HARDFAULTS
endif

#CMSIS
CMSIS_DIR      := $(ROOT)/lib/main/CMSIS

#STDPERIPH
STDPERIPH_DIR   = $(ROOT)/lib/main/STM32H7/Drivers/STM32H7xx_HAL_Driver
STDPERIPH_SRC   = $(notdir $(wildcard $(STDPERIPH_DIR)/Src/*.c))

EXCLUDES        = \
                stm32h7xx_hal_adc.c \
                stm32h7xx_hal_adc_ex.c \
                stm32h7xx_hal_cec.c \
                stm32h7xx_hal_comp.c \
                stm32h7xx_hal_cordic.c \
                stm32h7xx_hal_crc.c \
                stm32h7xx_hal_crc_ex.c \
                stm32h7xx_hal_cryp.c \
                stm32h7xx_hal_cryp_ex.c \
                stm32h7xx_hal_dac.c \
                stm32h7xx_hal_dac_ex.c \
                stm32h7xx_hal_dcmi.c \
                stm32h7xx_hal_dfsdm.c \
                stm32h7xx_hal_dfsdm_ex.c \
                stm32h7xx_hal_dma_ex.c \
                stm32h7xx_hal_dma2d.c \
                stm32h7xx_hal_dsi.c \
                stm32h7xx_hal_dts.c \
                stm32h7xx_hal_eth.c \
                stm32h7xx_hal_eth_ex.c \
                stm32h7xx_hal_exti.c \
                stm32h7xx_hal_fdcan.c \
                stm32h7xx_hal_fmac.c \
                stm32h7xx_hal_gfxmmu.c \
                stm32h7xx_hal_hash.c \
                stm32h7xx_hal_hash_ex.c \
                stm32h7xx_hal_hcd.c \
                stm32h7xx_hal_hrtim.c \
                stm32h7xx_hal_hsem.c \
                stm32h7xx_hal_i2c.c \
                stm32h7xx_hal_i2c_ex.c \
                stm32h7xx_hal_i2s.c \
                stm32h7xx_hal_i2s_ex.c \
                stm32h7xx_hal_irda.c \
                stm32h7xx_hal_iwdg.c \
                stm32h7xx_hal_jpeg.c \
                stm32h7xx_hal_lptim.c \
                stm32h7xx_hal_ltdc.c \
                stm32h7xx_hal_ltdc_ex.c \
                stm32h7xx_hal_mdios.c \
                stm32h7xx_hal_mmc.c \
                stm32h7xx_hal_mmc_ex.c \
                stm32h7xx_hal_msp_template.c \
                stm32h7xx_hal_nand.c \
                stm32h7xx_hal_nor.c \
                stm32h7xx_hal_opamp.c \
                stm32h7xx_hal_opamp_ex.c \
                stm32h7xx_hal_ospi.c \
                stm32h7xx_hal_otfdec.c \
                stm32h7xx_hal_pcd.c \
                stm32h7xx_hal_pcd_ex.c \
                stm32h7xx_hal_pssi.c \
                stm32h7xx_hal_ramecc.c \
                stm32h7xx_hal_rng.c \
                stm32h7xx_hal_rng_ex.c \
                stm32h7xx_hal_rtc.c \
                stm32h7xx_hal_rtc_ex.c \
                stm32h7xx_hal_sai.c \
                stm32h7xx_hal_sai_ex.c \
                stm32h7xx_hal_sd.c \
                stm32h7xx_hal_sd_ex.c \
                stm32h7xx_hal_sdram.c \
                stm32h7xx_hal_smartcard.c \
                stm32h7xx_hal_smartcard_ex.c \
                stm32h7xx_hal_smbus.c \
                stm32h7xx_hal_smbus_ex.c \
                stm32h7xx_hal_spdifrx.c \
                stm32h7xx_hal_spi.c \
                stm32h7xx_hal_spi_ex.c \
                stm32h7xx_hal_sram.c \
                stm32h7xx_hal_swpmi.c \
                stm32h7xx_hal_tim_ex.c \
                stm32h7xx_hal_timebase_rtc_alarm_template.c \
                stm32h7xx_hal_timebase_rtc_wakeup_template.c \
                stm32h7xx_hal_timebase_tim_template.c \
                stm32h7xx_hal_uart_ex.c \
                stm32h7xx_hal_usart.c \
                stm32h7xx_hal_usart_ex.c \
                stm32h7xx_hal_wwdg.c \
                stm32h7xx_ll_adc.c \
                stm32h7xx_ll_bdma.c \
                stm32h7xx_ll_comp.c \
                stm32h7xx_ll_cordic.c \
                stm32h7xx_ll_crc.c \
                stm32h7xx_ll_crs.c \
                stm32h7xx_ll_dac.c \
                stm32h7xx_ll_delayblock.c \
                stm32h7xx_ll_dma.c \
                stm32h7xx_ll_dma2d.c \
                stm32h7xx_ll_exti.c \
                stm32h7xx_ll_fmac.c \
                stm32h7xx_ll_fmc.c \
                stm32h7xx_ll_gpio.c \
                stm32h7xx_ll_hrtim.c \
                stm32h7xx_ll_i2c.c \
                stm32h7xx_ll_lptim.c \
                stm32h7xx_ll_lpuart.c \
                stm32h7xx_ll_mdma.c \
                stm32h7xx_ll_opamp.c \
                stm32h7xx_ll_pwr.c \
                stm32h7xx_ll_rcc.c \
                stm32h7xx_ll_rng.c \
                stm32h7xx_ll_rtc.c \
                stm32h7xx_ll_sdmmc.c \
                stm32h7xx_ll_spi.c \
                stm32h7xx_ll_swpmi.c \
                stm32h7xx_ll_tim.c \
                stm32h7xx_ll_usart.c \
                stm32h7xx_ll_usb.c \
                stm32h7xx_ll_utils.c

STDPERIPH_SRC   := $(filter-out ${EXCLUDES}, $(STDPERIPH_SRC))

VPATH := $(VPATH):$(STDPERIPH_DIR)/src

DEVICE_STDPERIPH_SRC := $(STDPERIPH_SRC)

#CMSIS
VPATH           := $(VPATH):$(CMSIS_DIR)/Include:$(CMSIS_DIR)/Device/ST/STM32H7xx
VPATH           := $(VPATH):$(STDPERIPH_DIR)/Src
CMSIS_SRC       :=
INCLUDE_DIRS    := $(INCLUDE_DIRS) \
                   $(STDPERIPH_DIR)/Inc \
                   $(CMSIS_DIR)/Core/Include \
                   $(ROOT)/lib/main/STM32H7/Drivers/CMSIS/Device/ST/STM32H7xx/Include \
                   $(ROOT)/src/main/drivers/stm32

#Flags
ARCH_FLAGS      = -mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -fsingle-precision-constant

# Flags that are used in the STM32 libraries
DEVICE_FLAGS    = -DUSE_HAL_DRIVER -DUSE_FULL_LL_DRIVER

#
# H743xI : 2M FLASH, 512KB AXI SRAM + 512KB D2 & D3 SRAM
#
ifeq ($(TARGET_MCU),STM32H743xx)
DEVICE_FLAGS       += -DSTM32H743xx
DEFAULT_LD_SCRIPT   = $(LINKER_DIR)/stm32_flash_h743_2m.ld
STARTUP_SRC         = startup_stm32h743xx.s
MCU_FLASH_SIZE     := 2048
DEVICE_FLAGS       += -DMAX_MPU_REGIONS=16

else
$(error Unknown MCU for H7 target)
endif

ifeq ($(LD_SCRIPT),)
LD_SCRIPT = $(DEFAULT_LD_SCRIPT)
endif

ifneq ($(FIRMWARE_SIZE),)
DEVICE_FLAGS   += -DFIRMWARE_SIZE=$(FIRMWARE_SIZE)
endif

DEVICE_FLAGS    += -DHSE_VALUE=$(HSE_VALUE) -DHSE_STARTUP_TIMEOUT=1000 -DSTM32

MCU_COMMON_SRC = \
            drivers/stm32/bus_quadspi.c \
            drivers/stm32/system_stm32h7xx.c \
            startup/system_stm32h7xx.c \
            startup/stm32h7xx_hal_timebase_tim.c

DSP_LIB := $(ROOT)/lib/main/CMSIS/DSP
DEVICE_FLAGS += -DARM_MATH_MATRIX_CHECK -DARM_MATH_ROUNDING -D__FPU_PRESENT=1 -DUNALIGNED_SUPPORT_DISABLE -DARM_MATH_CM7
