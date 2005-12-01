# myPS2 libmad Makefile
#
# Author: ntba2
# Date: 11-29-2005
# Notes: This works with Visual Studio 6.
#
# Compiles and copies libmad.a lib into
# myPS2 lib directory

EE_TARGET_DIR	= ../lib
EE_OBJS_DIR	= obj
EE_BIN_DIR	= bin

EE_OBJS		= $(EE_OBJS_DIR)/bit.o		\
		  $(EE_OBJS_DIR)/decoder.o	\
		  $(EE_OBJS_DIR)/fixed.o	\
		  $(EE_OBJS_DIR)/frame.o	\
		  $(EE_OBJS_DIR)/huffman.o	\
		  $(EE_OBJS_DIR)/layer12.o	\
		  $(EE_OBJS_DIR)/layer3.o	\
		  $(EE_OBJS_DIR)/stream.o	\
		  $(EE_OBJS_DIR)/synth.o	\
		  $(EE_OBJS_DIR)/timer.o	\
		  $(EE_OBJS_DIR)/version.o 

EE_LIB		= libmad.a

EE_CFLAGS += -DHAVE_CONFIG_H -DHAVE_SYS_TYPES_H -DHAVE_ERRNO_H -DHAVE_UNISTD_H \
	-DHAVE_FCNTL_H -I$(PS2SDK)/common/include/sys/  -Iinclude/

all: $(EE_OBJS_DIR) $(EE_BIN_DIR) $(EE_LIB)
	mv $(EE_LIB) $(EE_BIN_DIR)/$(EE_LIB)
	cp $(EE_BIN_DIR)/$(EE_LIB) $(EE_TARGET_DIR)/$(EE_LIB)

clean:
	rm -f -r $(EE_OBJS_DIR) $(EE_BIN_DIR)
	rm -f $(EE_TARGET_DIR)/$(EE_LIB)

include ../Makefile.pref
include ../Makefile.eeglobal

