# Verbose flag for debuging
ifneq ($(V),1)
	Q := @
endif

ifneq ($(OS),Windows_NT)
	CLEAN := unix_clean
	MKDIR := mkdir -p
else
	CLEAN := win_clean
	MKDIR := mkdir
endif

CODE_DIRS = firmware
INC_DIR = ./include
CFILES = $(foreach D,$(CODE_DIRS),$(wildcard $(D)/*.c))
OBJ_DIR = out/obj
OBJS = $(foreach F,$(notdir $(patsubst %.c,%.o,$(CFILES))),$(OBJ_DIR)/$(F))
BIN_DIR = out/bin

BINARY = firmware

########################################################################
# Device specifics

# LIBNAME = opencm3_stm32f1
# DEFS += -DSTM32F1
FPU_FLAGS = -mfloat-abi=soft # Cortex-m3 has no FPU
ARCH_FLAGS = -mthumb -mcpu=cortex-m3 $(FPU_FLAGS)

#######################################################################
# Linker

# LD_SCRIPT = ./scripts/linker.ld
# LD_LIBS += -l$(LIBNAME)
# LD_DIR += -L$(OPENCM3_DIR)/lib

#######################################################################
# Include
# DEFS += -I$(INC_DIR)

#######################################################################
# Compiler

PREFIX ?= arm-none-eabi-
CC := $(PREFIX)gcc
AS := $(PREFIX)as
LD := $(PREFIX)gcc
# STFLASH := $(shell which st-flash)
OPT := -O0
DEBUG := -ggdb3
CSTD ?= -std=c99

#######################################################################
# Src files


######################################################################
# C compiler flags

CFLAGS += $(OPT) $(CSTD) $(DEBUG)
CFLAGS += $(ARCH_FLAGS)
CFLAGS += -Wall -Wextra -Werror
# CFLAGS += -fno-common -ffunction-sections -fdata-sections 

###############################################################################
# C preprocessor common flags

# CPPFLAGS	+= -MD
# CPPFLAGS	+= -Wall -Wundef
# CPPFLAGS	+= $(DEFS)

###############################################################################
# Linker flags
#
LDFLAGS += -nostdlib
LDFLAGS	+= $(ARCH_FLAGS) $(DEBUG)
# LDFLAGS		+= --static -nostartfiles
# LDFLAGS		+= -T$(LDSCRIPT) $(LD_DIR)
# LDFLAGS		+= -Wl,-Map=$(*).map -Wl,--cref
# LDFLAGS		+= -Wl,--gc-sections
# ifeq ($(V),99)
# LDFLAGS		+= -Wl,--print-gc-sections
# endif
#
###############################################################################
# Used libraries

# LD_LIBS		+= -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group

###############################################################################
###############################################################################
###############################################################################


all: elf

elf: $(BINARY).elf

$(BINARY).elf: $(OBJS)
	$(Q)$(LD) $(LDFLAGS) $(filter %.o, $^) -o $(BIN_DIR)/$@ 

%.o: %.c
	$(Q)$(CC) $(CFLAGS)  -c $< -o $(OBJ_DIR)/$(notdir $@) 

%.o: %.S
	$(Q)$(CC) $(CFLAGS) -c $< -o $(OBJ_DIR)/$(notdir $@)

clean: $(CLEAN)

win_clean:
	@#printf "  CLEAN\n"
	$(Q)del /S /Q $(BIN_DIR)\*
	$(Q)del /S /Q $(OBJ_DIR)\*.o

unix_clean:
	$(Q)rm -rf  $(BIN_DIR)/* 
	rm -f $(OBJ_DIR)/*.o

.PHONY: clean elf

