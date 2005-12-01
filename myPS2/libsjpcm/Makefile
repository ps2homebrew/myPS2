# myPS2 libsjpcm Makefile
#
# Author: ntba2
# Date: 11-29-2005
# Notes: This works with Visual Studio 6.
#
# Compiles libsjpcm.a lib
#

EE_TARGET_DIR	= ../lib
EE_OBJS_DIR		= obj
EE_BIN_DIR		= bin

EE_OBJS		= $(EE_OBJS_DIR)/sjpcm_rpc.o

EE_LIB		= libsjpcm.a
EE_LIBS		= -lc

all:	$(EE_OBJS_DIR) $(EE_BIN_DIR) $(EE_LIB)
	mv $(EE_LIB) $(EE_BIN_DIR)/$(EE_LIB)
	cp $(EE_BIN_DIR)/$(EE_LIB) $(EE_TARGET_DIR)/$(EE_LIB)

clean:
	rm -f -r $(EE_OBJS_DIR) $(EE_BIN_DIR)
	rm -f $(EE_TARGET_DIR)/$(EE_LIB)

include ../Makefile.pref
include ../Makefile.eeglobal
