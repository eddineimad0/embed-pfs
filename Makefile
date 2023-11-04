# Verbose flag for debuging
ifneq($(V),1)
	Q := @
endif


SRC_DIR = ./src
INC_DIR = ./include
INTERM_DIR = ./out/interm
OUT_DIR = ./out/bin

BINARY = firmware

########################################################################
# Device specific

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
DEFS += -I$(INC_DIR)

#######################################################################
# Compiler

PREFIX ?= arm-none-eabi-

CC := $(PREFIX)gcc
AS := $(PREFIX)as
# STFLASH := $(shell which st-flash)
# OPT := -Os
# DEBUG := -ggdb3
CSTD ?= -std=c99

#######################################################################
# Src files

OBJS += $(INTERM_DIR)/$(BINARY).o
CFILES = ./src/mini_test.c

######################################################################
# C flags

CFLAGS += $(OPT) $(CSTD) $(DEBUG)
CFLAGS += $(ARCH_FLAGS)
CFLAGS += -Wall -Wextra -nostdlib
# CFLAGS += -fno-common -ffunction-sections -fdata-sections 

###############################################################################
# C preprocessor common flags

# CPPFLAGS	+= -MD
# CPPFLAGS	+= -Wall -Wundef
# CPPFLAGS	+= $(DEFS)

###############################################################################
# Linker flags
#
# LDFLAGS		+= --static -nostartfiles
# LDFLAGS		+= -T$(LDSCRIPT) $(LD_DIR)
# LDFLAGS		+= $(ARCH_FLAGS) $(DEBUG)
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
# Define a helper macro for debugging make errors online
# you can type "make print-OPENCM3_DIR" and it will show you
# how that ended up being resolved by all of the included
# makefiles.
print-%:
	@echo $*=$($*)

%.o: %.c
	@#printf "  CC      $(*).c\n"
	$(Q)$(CC) $(CFLAGS)  -o $(*).o -c $(*).c

%.o: %.S
	@#printf "  CC      $(*).S\n"
	$(Q)$(CC) $(CFLAGS) -o $(*).o -c $(*).S

clean:
	@#printf "  CLEAN\n"
	$(Q)rm  ./out/interm/*.o 


.PHONY: clean elf

