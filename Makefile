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
INC_DIR = include
OBJ_DIR = out/obj
BIN_DIR = out/bin
LDSCRIPT = scripts/linker.ld

BINARY = firmware

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
DEBUG := -ggdb3
CSTD ?= -std=c99

#######################################################################
# Translation units
CFILES = $(foreach D,$(CODE_DIRS),$(wildcard $(D)/*.c))
OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(CFILES))


######################################################################
# C compiler flags

CFLAGS += $(OPT) $(CSTD) $(DEBUG)
CFLAGS += $(ARCH_FLAGS)
CFLAGS += -Wall -Wextra -Wshadow -Wredundant-decls -Werror 
CFLAGS += -fno-common -ffunction-sections -fdata-sections 

###############################################################################
# C preprocessor common flags

CPPFLAGS	+= -MD
CPPFLAGS	+= -Wall -Wundef
CPPFLAGS	+= $(DEFS)

###############################################################################
# Linker flags
#
LDFLAGS += -nostdlib
LDFLAGS	+= --static -nostartfiles
LDFLAGS	+= $(ARCH_FLAGS) $(DEBUG)
LDFLAGS	+= -T$(LDSCRIPT)
LDFLAGS	+= -Wl,-Map=$(*).map
LDFLAGS += -Wl,--cref
LDFLAGS	+= -Wl,--gc-sections
# ifeq ($(V),99)
# LDFLAGS		+= -Wl,--print-gc-sections
# endif
#
###############################################################################
# Used libraries
LD_LIBS += -L$(OPENCM3_DIR)/lib
LD_LIBS += -l$(LIBNAME)
LD_LIBS	+= -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group

###############################################################################
###############################################################################
###############################################################################
$(info [+] $(CFILES))

all: bin elf 

elf: $(BIN_DIR)/$(BINARY).elf
bin: $(BIN_DIR)/$(BINARY).bin

$(OBJ_DIR)/%.o:%.c
	$(Q)$(DIR_GUARD)
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@ 

$(OBJ_DIR)/%.o:%.S
	$(Q)$(CC) $(CFLAGS) -c $< -o $(OBJ_DIR)/$(notdir $@)

%.bin:%.elf
	$(Q)$(DIR_GUARD)
	$(Q)$(OBJCOPY) -Obinary $< $@

%.elf: $(OBJS) $(LDSCRIPT) $(OPENCM3_DIR)/lib/lib$(LIBNAME).a Makefile
	$(Q)$(DIR_GUARD)
	$(Q)$(LD) $(LDFLAGS) $(filter %.o, $^) $(LD_LIBS) -o $@ 

clean: $(CLEAN)

win_clean:
	$(Q)if exist $(subst /,\,$(BIN_DIR)) rmdir /Q /S $(subst /,\,$(BIN_DIR))
	$(Q)if exist $(subst /,\,$(BIN_DIR)) rmdir /Q /S $(subst /,\,$(OBJ_DIR))

unix_clean:
	$(Q)rm -rf  $(BIN_DIR)/* 
	rm -f $(OBJ_DIR)/*.o

.PHONY: clean elf

