OPENOCD ?= openocd
OPENOCD_IF ?= interface/stlink.cfg

ifeq ($(TARGET_MCU),STM32H7)
OPENOCD_CFG := target/stm32h7x.cfg
else
endif

ifneq ($(OPENOCD_CFG),)
OPENOCD_COMMAND = $(OPENOCD) -f $(OPENOCD_IF) -f $(OPENOCD_CFG)
endif
