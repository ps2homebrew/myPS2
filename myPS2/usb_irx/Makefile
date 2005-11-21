# myPS2 usb_irx Makefile
#
# Author: ntba2
# Date: 11-17-2005
# Notes: This works with Visual Studio 6.
#
# Compiles usb_mass.irx module
#

IOP_OBJS_DIR	= obj
IOP_BIN_DIR		= bin

IOP_OBJS		= $(IOP_OBJS_DIR)/mass_stor.o	\
				  $(IOP_OBJS_DIR)/fat_driver.o	\
				  $(IOP_OBJS_DIR)/scache.o		\
				  $(IOP_OBJS_DIR)/usb_mass.o	\
				  $(IOP_OBJS_DIR)/imports.o

IOP_BIN			= usb_mass.irx
IOP_LDFLAGS		= -Wl,-s
IOP_CFLAGS		= -D_PS2_

#uncomment to enable write support
WRITE_SUPPORT	= 1

#uncomment to compile in dump/info functions
#COMPILE_DUMPS	= 1

ifdef WRITE_SUPPORT
IOP_CFLAGS		+= -DWRITE_SUPPORT
IOP_OBJS		+= $(IOP_OBJS_DIR)/fat_write.o
endif

ifdef COMPILE_DUMPS
IOP_CFLAGS += -DCOMPILE_DUMPS
endif

all: $(IOP_OBJS_DIR) $(IOP_BIN_DIR) $(IOP_BIN)
	mv $(IOP_BIN) $(IOP_BIN_DIR)/$(IOP_BIN)

clean:
	rm -f -r $(IOP_OBJS_DIR) $(IOP_BIN_DIR)


include ../Makefile.pref
include ../Makefile.iopglobal


# A rule to build imports.lst.
$(IOP_OBJS_DIR)/%.o : %.lst
	echo "#include \"irx_imports.h\"" > build-imports.c
	cat $< >> build-imports.c
	$(IOP_CC) $(IOP_CFLAGS) build-imports.c -o $@
	-rm -f build-imports.c
