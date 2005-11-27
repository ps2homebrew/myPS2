# myPS2 ps2ftpd Makefile
#
# Author: ntba2
# Date: 11-23-2005
# Notes: This works with Visual Studio 6.
#
# Compiles ps2ftpd.irx
#

IOP_OBJS_DIR	= obj
IOP_BIN_DIR	= bin

IOP_OBJS	= $(IOP_OBJS_DIR)/main.o	\
		  $(IOP_OBJS_DIR)/FileSystem.o	\
		  $(IOP_OBJS_DIR)/FtpServer.o	\
		  $(IOP_OBJS_DIR)/FtpClient.o	\
		  $(IOP_OBJS_DIR)/FtpCommands.o	\
		  $(IOP_OBJS_DIR)/FtpMessages.o	\
		  $(IOP_OBJS_DIR)/imports.o

IOP_BIN		= ps2ftpd.irx


IOP_CFLAGS += -Wall -fno-builtin
IOP_LDFLAGS += -s
IOP_INCS += -I$(PS2SDK)/iop/include

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