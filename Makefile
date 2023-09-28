###############################################################################
#
# Makefile for building the antibear.
#
# Invoke this with 'make help' to see the list of supported targets.
#
###############################################################################


# Things that the user might override on the commandline
#

# The target to build
TARGET    ?= antibear

# Compile-time options
OPTIONS   ?=

# Debugger optons:
#   empty - ordinary build with all optimizations enabled
#   INFO - ordinary build with debug symbols and all optimizations enabled
#   GDB - debug build with minimum number of optimizations
DEBUG     ?=

# Insert the debugging hardfault debugger
# releases should not be built with this flag as it does not disable pwm output
DEBUG_HARDFAULTS ?=

# Flash size (KB).  Some low-end chips actually have more flash than advertised, use this to override.
FLASH_SIZE ?=

###############################################################################
# Things that need to be maintained as the source changes
#

# Working directories
ROOT            := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
SRC_DIR         := $(ROOT)/src/main
OBJECT_DIR      := $(ROOT)/obj/main
BIN_DIR         := $(ROOT)/obj
CMSIS_DIR       := $(ROOT)/lib/main/CMSIS
INCLUDE_DIRS    := $(SRC_DIR) \
                   $(ROOT)/src/main/target \
                   $(ROOT)/src/main/startup \
                   $(ROOT)/src/main/FreeRTOS
LINKER_DIR      := $(ROOT)/src/link
MAKE_SCRIPT_DIR := $(ROOT)/mk
DEBUG_DIR		:= $(ROOT)/obj/debug

## V                 : Set verbosity level based on the V= parameter
##                     V=0 Low
##                     V=1 High
include $(MAKE_SCRIPT_DIR)/build_verbosity.mk

# Search path for sources
VPATH           := $(SRC_DIR):$(SRC_DIR)/startup
CSOURCES        := $(shell find $(SRC_DIR) -name '*.c')

VERSION_MAJOR := $(shell grep " VERSION_MAJOR" src/main/build/version.h | awk '{print $$3}' )
VERSION_MINOR := $(shell grep " VERSION_MINOR" src/main/build/version.h | awk '{print $$3}' )
VERSION_PATCH := $(shell grep " VERSION_PATCH" src/main/build/version.h | awk '{print $$3}' )

VERSION       := $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)

# default xtal value
HSE_VALUE       ?= 25000000

include $(ROOT)/src/main/target/target.mk

REVISION := norevision
ifeq ($(shell git diff --shortstat),)
REVISION := $(shell git log -1 --format="%h")
endif

LD_FLAGS        :=
EXTRA_LD_FLAGS  :=

#
# Default Tool options - can be overridden in {mcu}.mk files.
#
ifeq ($(DEBUG),GDB)
OPTIMISE_DEFAULT      := -Og

LTO_FLAGS             := $(OPTIMISE_DEFAULT)
DEBUG_FLAGS            = -ggdb2 -gdwarf-5 -DDEBUG
else
ifeq ($(DEBUG),INFO)
DEBUG_FLAGS            = -ggdb2
endif
OPTIMISATION_BASE     := -flto -fuse-linker-plugin -ffast-math -fmerge-all-constants
OPTIMISE_DEFAULT      := -O2
OPTIMISE_SPEED        := -Ofast
OPTIMISE_SIZE         := -Os

LTO_FLAGS             := $(OPTIMISATION_BASE) $(OPTIMISE_SPEED)
endif

VPATH 			:= $(VPATH):$(MAKE_SCRIPT_DIR)

# start specific includes
ifeq ($(TARGET_MCU),)
$(error No TARGET_MCU specified. Is the target.mk valid for $(TARGET)?)
endif

ifeq ($(TARGET_MCU_FAMILY),)
$(error No TARGET_MCU_FAMILY specified. Is the target.mk valid for $(TARGET)?)
endif

TARGET_FLAGS  	:= -D$(TARGET) -D$(TARGET_MCU_FAMILY) $(TARGET_FLAGS)

include $(MAKE_SCRIPT_DIR)/mcu.mk

# openocd specific includes
include $(MAKE_SCRIPT_DIR)/openocd.mk

# Configure default flash sizes for the targets (largest size specified gets hit first) if flash not specified already.
ifeq ($(TARGET_FLASH_SIZE),)
ifneq ($(MCU_FLASH_SIZE),)
TARGET_FLASH_SIZE := $(MCU_FLASH_SIZE)
else
$(error MCU_FLASH_SIZE not configured for target $(TARGET))
endif
endif

DEVICE_FLAGS  := $(DEVICE_FLAGS) -DTARGET_FLASH_SIZE=$(TARGET_FLASH_SIZE)

ifneq ($(HSE_VALUE),)
DEVICE_FLAGS  := $(DEVICE_FLAGS) -DHSE_VALUE=$(HSE_VALUE)
endif

TARGET_DIR     = $(ROOT)/src/main/target
TARGET_DIR_SRC = $(notdir $(wildcard $(TARGET_DIR)/*.c))

.DEFAULT_GOAL := hex

INCLUDE_DIRS    := $(INCLUDE_DIRS) \
                   $(TARGET_DIR)

VPATH           := $(VPATH):$(TARGET_DIR)

include $(MAKE_SCRIPT_DIR)/source.mk

###############################################################################
# Things that might need changing to use different tools
#

# Find out if ccache is installed on the system
CCACHE := ccache
RESULT = $(shell (which $(CCACHE) > /dev/null 2>&1; echo $$?) )
ifneq ($(RESULT),0)
CCACHE :=
endif

PYTHON      := python
ARM_SDK_PREFIX = arm-none-eabi-

# Tool names
CROSS_CC    := $(CCACHE) $(ARM_SDK_PREFIX)gcc
CROSS_CXX   := $(CCACHE) $(ARM_SDK_PREFIX)g++
CROSS_GDB   := $(ARM_SDK_PREFIX)gdb
OBJCOPY     := $(ARM_SDK_PREFIX)objcopy
OBJDUMP     := $(ARM_SDK_PREFIX)objdump
READELF     := $(ARM_SDK_PREFIX)readelf
SIZE        := $(ARM_SDK_PREFIX)size
DFUSE-PACK  := src/utils/dfuse-pack.py

#
# Tool options.
#
CC_DEBUG_OPTIMISATION   := $(OPTIMISE_DEFAULT)
CC_DEFAULT_OPTIMISATION := $(OPTIMISATION_BASE) $(OPTIMISE_DEFAULT)
CC_SPEED_OPTIMISATION   := $(OPTIMISATION_BASE) $(OPTIMISE_SPEED)
CC_SIZE_OPTIMISATION    := $(OPTIMISATION_BASE) $(OPTIMISE_SIZE)
CC_NO_OPTIMISATION      :=

#
# Added after GCC version update, remove once the warnings have been fixed
#
TEMPORARY_FLAGS :=

EXTRA_WARNING_FLAGS := -Wold-style-definition

CFLAGS     += $(ARCH_FLAGS) \
              $(addprefix -D,$(OPTIONS)) \
              $(addprefix -I,$(INCLUDE_DIRS)) \
              $(DEBUG_FLAGS) \
              -std=gnu17 \
              -Wall -Wextra -Wpedantic -Wunsafe-loop-optimizations -Wdouble-promotion \
              $(EXTRA_WARNING_FLAGS) \
              -ffunction-sections \
              -fdata-sections \
              -fno-common \
              $(TEMPORARY_FLAGS) \
              $(DEVICE_FLAGS) \
              -D_GNU_SOURCE \
              -DUSE_STDPERIPH_DRIVER \
              -D$(TARGET) \
              $(TARGET_FLAGS) \
              -D'__TARGET__="$(TARGET)"' \
              -D'__REVISION__="$(REVISION)"' \
              $(CONFIG_REVISION_DEFINE) \
              -pipe \
              -MMD -MP \
              $(EXTRA_FLAGS)

ASFLAGS     = $(ARCH_FLAGS) \
              $(DEBUG_FLAGS) \
              -x assembler-with-cpp \
              $(addprefix -I,$(INCLUDE_DIRS)) \
              -MMD -MP

ifeq ($(LD_FLAGS),)
LD_FLAGS     = -lm \
              -nostartfiles \
              --specs=nano.specs \
              -lc \
              -lnosys \
              $(ARCH_FLAGS) \
              $(LTO_FLAGS) \
              $(DEBUG_FLAGS) \
              -static \
              -Wl,-gc-sections,-Map,$(TARGET_MAP) \
              -Wl,-L$(LINKER_DIR) \
              -Wl,--cref \
              -Wl,--no-wchar-size-warning \
              -Wl,--print-memory-usage \
              -T$(LD_SCRIPT) \
               $(EXTRA_LD_FLAGS)
endif

ifeq ($(DEBUG),)
CFLAGS     += -Werror
endif

###############################################################################
# No user-serviceable parts below
###############################################################################

CPPCHECK        = cppcheck $(CSOURCES) --enable=all --platform=unix64 \
                  --std=c99 --inline-suppr --quiet --force \
                  $(addprefix -I,$(INCLUDE_DIRS)) \
                  -I/usr/include -I/usr/include/linux

TARGET_NAME := $(TARGET)

ifneq ($(VERSION),)
TARGET_NAME := $(TARGET_NAME)_$(VERSION)
endif

ifeq ($(REV),yes)
TARGET_NAME := $(TARGET_NAME)_$(REVISION)
endif

TARGET_FULLNAME = $(TARGET_NAME)
#
# Things we will build
#
TARGET_BIN      = $(BIN_DIR)/$(TARGET_FULLNAME).bin
TARGET_HEX      = $(BIN_DIR)/$(TARGET_FULLNAME).hex
TARGET_DFU      = $(BIN_DIR)/$(TARGET_FULLNAME).dfu
TARGET_ZIP      = $(BIN_DIR)/$(TARGET_FULLNAME).zip
TARGET_OBJ_DIR  = $(OBJECT_DIR)/$(TARGET_NAME)
TARGET_ELF      = $(OBJECT_DIR)/$(TARGET_NAME).elf
TARGET_UNPATCHED_BIN = $(OBJECT_DIR)/$(TARGET_NAME)_UNPATCHED.bin
TARGET_LST      = $(OBJECT_DIR)/$(TARGET_NAME).lst
TARGET_OBJS     = $(addsuffix .o,$(addprefix $(TARGET_OBJ_DIR)/,$(basename $(SRC))))
TARGET_DEPS     = $(addsuffix .d,$(addprefix $(TARGET_OBJ_DIR)/,$(basename $(SRC))))
TARGET_MAP      = $(OBJECT_DIR)/$(TARGET_NAME).map

TARGET_EF_HASH      := $(shell echo -n "$(EXTRA_FLAGS)" | openssl dgst -md5 | awk '{print $$2;}')
TARGET_EF_HASH_FILE := $(TARGET_OBJ_DIR)/.efhash_$(TARGET_EF_HASH)

CLEAN_ARTIFACTS := $(TARGET_BIN)
CLEAN_ARTIFACTS += $(TARGET_HEX_REV) $(TARGET_HEX)
CLEAN_ARTIFACTS += $(TARGET_ELF) $(TARGET_OBJS) $(TARGET_MAP)
CLEAN_ARTIFACTS += $(TARGET_LST)
CLEAN_ARTIFACTS += $(TARGET_DFU)

# Make sure build date and revision is updated on every incremental build
$(TARGET_OBJ_DIR)/build/version.o : $(SRC)

# List of buildable ELF files and their object dependencies.
# It would be nice to compute these lists, but that seems to be just beyond make.

$(TARGET_LST): $(TARGET_ELF)
	$(V0) $(OBJDUMP) -S --disassemble $< > $@

$(TARGET_BIN): $(TARGET_ELF)
	@echo "Creating BIN $(TARGET_BIN)" "$(STDOUT)"
	$(V1) $(OBJCOPY) -O binary $< $@

$(TARGET_HEX): $(TARGET_ELF)
	@echo "Creating HEX $(TARGET_HEX)" "$(STDOUT)"
	$(V1) $(OBJCOPY) -O ihex --set-start 0x8000000 $< $@

$(TARGET_DFU): $(TARGET_HEX)
	@echo "Creating DFU $(TARGET_DFU)" "$(STDOUT)"
	$(V1) $(PYTHON) $(DFUSE-PACK) -i $< $@

$(TARGET_ELF): $(TARGET_OBJS) $(LD_SCRIPT) $(LD_SCRIPTS)
	@echo "Linking $(TARGET_NAME)" "$(STDOUT)"
	$(V1) $(CROSS_CC) -o $@ $(filter-out %.ld,$^) $(LD_FLAGS)
	$(V1) $(SIZE) $(TARGET_ELF)
ifeq ($(DEBUG),GDB)
	mkdir -p $(DEBUG_DIR)
	cp $(TARGET_ELF) $(DEBUG_DIR)/debug.elf
endif

# Compile

## compile_file takes two arguments: (1) optimisation description string and (2) optimisation compiler flag
define compile_file
	echo "%% ($(1)) $<" "$(STDOUT)" && \
	$(CROSS_CC) -c -o $@ $(CFLAGS) $(2) $<
endef

ifeq ($(DEBUG),GDB)
$(TARGET_OBJ_DIR)/%.o: %.c
	$(V1) mkdir -p $(dir $@)
	$(V1) $(if $(findstring $<,$(NOT_OPTIMISED_SRC)), \
		$(call compile_file,not optimised, $(CC_NO_OPTIMISATION)) \
	, \
		$(call compile_file,debug,$(CC_DEBUG_OPTIMISATION)) \
	)
else
$(TARGET_OBJ_DIR)/%.o: %.c
	$(V1) mkdir -p $(dir $@)
	$(V1) $(if $(findstring $<,$(NOT_OPTIMISED_SRC)), \
		$(call compile_file,not optimised,$(CC_NO_OPTIMISATION)) \
	, \
		$(if $(findstring $(subst ./src/main/,,$<),$(SPEED_OPTIMISED_SRC)), \
			$(call compile_file,speed optimised,$(CC_SPEED_OPTIMISATION)) \
		, \
			$(if $(findstring $(subst ./src/main/,,$<),$(SIZE_OPTIMISED_SRC)), \
				$(call compile_file,size optimised,$(CC_SIZE_OPTIMISATION)) \
			, \
				$(call compile_file,optimised,$(CC_DEFAULT_OPTIMISATION)) \
			) \
		) \
	)
endif

# Assemble
$(TARGET_OBJ_DIR)/%.o: %.s
	$(V1) mkdir -p $(dir $@)
	@echo "%% $(notdir $<)" "$(STDOUT)"
	$(V1) $(CROSS_CC) -c -o $@ $(ASFLAGS) $<

$(TARGET_OBJ_DIR)/%.o: %.S
	$(V1) mkdir -p $(dir $@)
	@echo "%% $(notdir $<)" "$(STDOUT)"
	$(V1) $(CROSS_CC) -c -o $@ $(ASFLAGS) $<

antibear:
	$(V0) @echo "Building target $@" && \
	$(MAKE) hex TARGET=$@ && \
	echo "Building $@ succeeded."

## clean             : clean up temporary / machine-generated files
clean:
	@echo "Cleaning $(TARGET_NAME)"
	$(V0) rm -f $(CLEAN_ARTIFACTS)
	$(V0) rm -rf $(TARGET_OBJ_DIR)
	$(V0) rm -rf $(DEBUG_DIR)
	@echo "Cleaning $(TARGET_NAME) succeeded."

ifneq ($(OPENOCD_COMMAND),)
openocd-gdb: $(TARGET_ELF)
	$(V0) $(OPENOCD_COMMAND) & $(CROSS_GDB) $(TARGET_ELF) -ex "target remote localhost:3333" -ex "load"
endif

## antibear_zip    : build target and zip it (useful for posting to GitHub)
antibear_zip:
	$(V0) $(MAKE) hex TARGET=$(subst _zip,,$@)
	$(V0) $(MAKE) zip TARGET=$(subst _zip,,$@)

zip:
	$(V0) zip $(TARGET_ZIP) $(TARGET_HEX)

binary:
	$(V0) $(MAKE) -j $(TARGET_BIN)

hex:
	$(V0) $(MAKE) -j $(TARGET_HEX)

## antibear_rev    : build target and add revision to filename
antibear_rev:
	$(V0) $(MAKE) hex REV=yes TARGET=$(subst _rev,,$@)

## cppcheck          : run static analysis on C source code
cppcheck: $(CSOURCES)
	$(V0) $(CPPCHECK)

cppcheck-result.xml: $(CSOURCES)
	$(V0) $(CPPCHECK) --xml-version=2 2> cppcheck-result.xml

# mkdirs
$(DIRECTORIES):
	mkdir -p $@

## version           : print firmware version
version:
	@echo $(VERSION)

## help              : print this help message and exit
help:
	@echo ""
	@echo "Makefile for the antibear"
	@echo ""
	@echo "Usage:"
	@echo "        make [V=<verbosity>] [OPTIONS=\"<options>\"] [EXTRA_FLAGS=\"<extra_flags>\"]"
	@echo ""

## targets           : print a list of all valid target platforms (for consumption by scripts)
targets:
	@echo "Target name:      $(TARGET)"

## target-mcu        : print the MCU type of the target
target-mcu:
	@echo "$(TARGET_MCU_FAMILY) : $(TARGET_MCU)"

$(TARGET_EF_HASH_FILE):
	$(V1) mkdir -p $(dir $@)
	$(V0) rm -f $(TARGET_OBJ_DIR)/.efhash_*
	@echo "EF HASH -> $(TARGET_EF_HASH_FILE)"
	$(V1) touch $(TARGET_EF_HASH_FILE)

# rebuild everything when makefile changes or the extra flags have changed
$(TARGET_OBJS): $(TARGET_EF_HASH_FILE) Makefile $(TARGET_DIR)/target.mk $(wildcard make/*)

# include auto-generated dependencies
-include $(TARGET_DEPS)
