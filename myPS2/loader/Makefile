# myPS2 loader Makefile
#
# Author: ntba2
# Date: 11-19-2005
# Notes: This works with Visual Studio 6.
#
# Compiles loader.elf

EE_INCS := $(EE_INCS) -I$(PS2DEV)/sbv-1.0-lite/include

EE_CFLAGS  := -mips3 -ffreestanding -fno-builtin -G0 \
		-fshort-double -mlong64 -mhard-float -mno-abicalls -O2 -EL -Wall \
		-c $(EE_INCS) $(EE_CFLAGS)

EE_ASFLAGS := -EL -G0 $(EE_ASFLAGS)

LOADADDR  = 0x90000

STACKADDR = 0xb0000

ifeq ($(DEBUG),1)
LOADADDR  = 0x1700000
STACKADDR = 0x1720000
endif

LDPARAMS := -Wl,--defsym -Wl,_stack_size=0x8000 -Wl,--defsym -Wl,_stack=$(STACKADDR)

EE_LDFLAGS += -Wl,-Ttext -Wl,$(LOADADDR) -s $(LDPARAMS)

EE_LDFLAGS := $(EE_LDFLAGS) -L$(PS2DEV)/sbv-1.0-lite/lib \

EE_BIN = loader.elf

EE_OBJS = $(EE_OBJS_DIR)/loader.o		\
		  $(EE_OBJS_DIR)/fakehost.o		\
		  $(EE_OBJS_DIR)/poweroff.o

EE_LIBS = -lfileXio -lpatches -lpoweroff -ldebug -lc

EE_OBJS_DIR = obj
EE_BIN_DIR	= bin

all: $(EE_OBJS_DIR) $(EE_BIN_DIR) $(EE_BIN)
	mv $(EE_BIN) $(EE_BIN_DIR)/$(EE_BIN)

$(EE_OBJS_DIR)/fakehost.s:
	bin2s $(PS2SDK)/iop/irx/fakehost.irx $(EE_OBJS_DIR)/fakehost.s fakehost_irx

$(EE_OBJS_DIR)/poweroff.s:
	bin2s $(PS2SDK)/iop/irx/poweroff.irx $(EE_OBJS_DIR)/poweroff.s poweroff_irx

clean:
	rm -f -r $(EE_OBJS_DIR) $(EE_BIN_DIR)

include ../Makefile.pref
include ../Makefile.eeglobal
