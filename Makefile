# Verbose flag for debuging
ifneq ($(V),1)
	Q := @
endif

ifneq ($(OS),Windows_NT)
	CLEAN := unix_clean
	DIR_GUARD = mkdir -p $(@D)
else
	CLEAN := win_clean
	DIR_GUARD = if not exist $(subst /,\,$(@D)) md $(subst /,\,$(@D))
endif

CODE_DIRS = firmware
BOOT_SRC_DIRS = boot
DRIVER_SRC_DIRS = driver
UTIL_SRC_DIRS = util
INC_DIR = include
OBJ_DIR = out/obj
BIN_DIR = out/bin
SCRIPTS_DIR = scripts
LDSCRIPT = $(SCRIPTS_DIR)/linker.ld
BOOT_LDSCRIPT = $(SCRIPTS_DIR)/bootlinker.ld

BINARIES += $(BIN_DIR)/bootloader.bin
BINARIES += $(BIN_DIR)/firmware.bin

ELFS += $(BIN_DIR)/bootloader.elf
ELFS += $(BIN_DIR)/firmware.elf

OPENCM3_DIR = ./libs/libopencm3

########################################################################
# Device specifics

LIBNAME = opencm3_stm32f1
FPU_FLAGS = -mfloat-abi=soft # Cortex-m3 has no FPU
ARCH_FLAGS = -mthumb -mcpu=cortex-m3 $(FPU_FLAGS)
DEFS += -DSTM32F1

#######################################################################
# Include
DEFS += -I$(INC_DIR)
DEFS += -I$(OPENCM3_DIR)/include

#######################################################################
# Compiler

PREFIX ?= arm-none-eabi-
CC := $(PREFIX)gcc
AS := $(PREFIX)as
LD := $(PREFIX)gcc
OBJCOPY := $(PREFIX)objcopy
OBJDUMP := $(PREFIX)objdump
OPT := -Os
DEBUG := -g3
CSTD ?= -std=c99

#######################################################################
# Translation units
CFILES = $(foreach D,$(CODE_DIRS),$(wildcard $(D)/*.c))
ASMFILES += $(foreach D,$(CODE_DIRS),$(wildcard $(D)/*.S))
OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(CFILES))
OBJS += $(patsubst %.S,$(OBJ_DIR)/%.o,$(ASMFILES))

BOOT_CFILES = $(foreach D,$(BOOT_SRC_DIRS),$(wildcard $(D)/*.c))
BOOT_OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(BOOT_CFILES))

DRIVERS_CFILES = $(foreach D,$(DRIVER_SRC_DIRS),$(wildcard $(D)/*.c))
DRIVERS_OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(DRIVERS_CFILES))

UTIL_CFILES = $(foreach D,$(UTIL_SRC_DIRS),$(wildcard $(D)/*.c))
UTIL_OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(UTIL_CFILES))


######################################################################
# C compiler flags

CFLAGS += $(OPT) $(CSTD) $(DEBUG)
CFLAGS += $(ARCH_FLAGS)
CFLAGS += -Wall -Wextra -Wshadow  
CFLAGS += -Wconversion  -Wredundant-decls -Werror 
CFLAGS += -fno-common -ffunction-sections -fdata-sections 

###############################################################################
# C preprocessor common flags

CPPFLAGS	+= -MD
CPPFLAGS	+= -Wall -Wundef
CPPFLAGS	+= $(DEFS)

###############################################################################
# Linker flags

LDFLAGS += -nostdlib
LDFLAGS	+= --static -nostartfiles
LDFLAGS	+= $(ARCH_FLAGS) $(DEBUG)
LDFLAGS	+= -Wl,-Map=$@.map
LDFLAGS += -Wl,--cref
LDFLAGS	+= -Wl,--gc-sections

###############################################################################
# Used libraries

LD_LIBS += -L$(OPENCM3_DIR)/lib
LD_LIBS += -l$(LIBNAME)
LD_LIBS	+= -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group

###############################################################################
###############################################################################
###############################################################################
$(info [+] $(CFILES))

all: firmware 

flash: firmware
	st-flash.exe --reset write $(BIN_DIR)/firmware.bin 0x8000000

debug: firmware
	st-flash.exe --reset write $(BIN_DIR)/firmware.bin 0x8000000
	st-util -p 4500

bootloader: $(BIN_DIR)/bootloader.bin
firmware: $(BIN_DIR)/firmware.bin

$(OBJ_DIR)/%.o:%.c
	$(Q)$(DIR_GUARD)
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@ 

$(OBJ_DIR)/%.o:%.S
	$(Q)$(DIR_GUARD)
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/firmware.bin:$(BIN_DIR)/firmware.elf
	$(Q)$(DIR_GUARD)
	$(Q)$(OBJCOPY) -Obinary $< $@

$(BIN_DIR)/firmware.elf: $(OBJS) $(BOOT_OBJS) $(DRIVERS_OBJS) $(UTIL_OBJS) $(LDSCRIPT) $(OPENCM3_DIR)/lib/lib$(LIBNAME).a Makefile
	$(Q)$(DIR_GUARD)
	$(Q)$(LD) $(LDFLAGS) -T$(LDSCRIPT) $(OBJS) $(DRIVERS_OBJS) $(UTIL_OBJS) $(LD_LIBS) -o $@ 

$(BIN_DIR)/bootloader.bin: $(BIN_DIR)/bootloader.elf
	$(Q)$(DIR_GUARD)
	$(Q)$(OBJCOPY) -Obinary $< $@
	$(Q)python3 $(SCRIPTS_DIR)/pad-bootloader.py
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -c firmware/bootloader-section.S -o $(OBJ_DIR)/firmware/bootloader-section.o

$(BIN_DIR)/bootloader.elf: $(BOOT_OBJS) $(DRIVERS_OBJS) $(UTIL_OBJS) $(BOOT_LDSCRIPT) $(OPENCM3_DIR)/lib/lib$(LIBNAME).a Makefile
	$(Q)$(DIR_GUARD)
	$(Q)$(LD) $(LDFLAGS) -T$(BOOT_LDSCRIPT) $(BOOT_OBJS) $(DRIVERS_OBJS) $(UTIL_OBJS) $(LD_LIBS) -o $@ 

clean: $(CLEAN)

win_clean:
	$(Q)if exist $(subst /,\,$(BIN_DIR)) rmdir /Q /S $(subst /,\,$(BIN_DIR))
	$(Q)if exist $(subst /,\,$(OBJ_DIR)) rmdir /Q /S $(subst /,\,$(OBJ_DIR))

unix_clean:
	$(Q)rm -rf  $(BIN_DIR)/* 
	$(Q)rm -f $(OBJ_DIR)/*.o

.PHONY: clean elf
