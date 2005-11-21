# myPS2 libjpg Makefile
#
# Author: ntba2
# Date: 11-17-2005
# Notes: This works with Visual Studio 6.
#
# Compiles libjpg.a and copies lib to myPS2 lib directory.

SYSDEPMEM	= jmemnobs.o

# library object files common to compression and decompression
COMOBJECTS	= $(EE_OBJS_DIR)/jcomapi.o	\
			  $(EE_OBJS_DIR)/jutils.o	\
			  $(EE_OBJS_DIR)/jerror.o	\
			  $(EE_OBJS_DIR)/jmemmgr.o	\
			  $(EE_OBJS_DIR)/$(SYSDEPMEM)

# compression library object files
CLIBOBJECTS	= $(EE_OBJS_DIR)/jcapimin.o	\
			  $(EE_OBJS_DIR)/jcapistd.o	\
			  $(EE_OBJS_DIR)/jctrans.o	\
			  $(EE_OBJS_DIR)/jcparam.o	\
			  $(EE_OBJS_DIR)/jdatadst.o	\
			  $(EE_OBJS_DIR)/jcinit.o	\
			  $(EE_OBJS_DIR)/jcmaster.o	\
			  $(EE_OBJS_DIR)/jcmarker.o	\
			  $(EE_OBJS_DIR)/jcmainct.o	\
			  $(EE_OBJS_DIR)/jcprepct.o	\
			  $(EE_OBJS_DIR)/jccoefct.o	\
			  $(EE_OBJS_DIR)/jccolor.o	\
			  $(EE_OBJS_DIR)/jcsample.o	\
			  $(EE_OBJS_DIR)/jchuff.o	\
			  $(EE_OBJS_DIR)/jcphuff.o	\
			  $(EE_OBJS_DIR)/jcdctmgr.o	\
			  $(EE_OBJS_DIR)/jfdctfst.o	\
			  $(EE_OBJS_DIR)/jfdctflt.o	\
			  $(EE_OBJS_DIR)/jfdctint.o

# decompression library object files
DLIBOBJECTS	= $(EE_OBJS_DIR)/jdapimin.o	\
			  $(EE_OBJS_DIR)/jdapistd.o	\
			  $(EE_OBJS_DIR)/jdtrans.o	\
			  $(EE_OBJS_DIR)/jdmaster.o	\
			  $(EE_OBJS_DIR)/jdinput.o	\
			  $(EE_OBJS_DIR)/jdmarker.o	\
			  $(EE_OBJS_DIR)/jdhuff.o	\
			  $(EE_OBJS_DIR)/jdphuff.o	\
			  $(EE_OBJS_DIR)/jdmainct.o	\
			  $(EE_OBJS_DIR)/jdcoefct.o	\
			  $(EE_OBJS_DIR)/jdpostct.o	\
			  $(EE_OBJS_DIR)/jddctmgr.o	\
			  $(EE_OBJS_DIR)/jidctfst.o	\
			  $(EE_OBJS_DIR)/jidctflt.o	\
			  $(EE_OBJS_DIR)/jidctint.o	\
			  $(EE_OBJS_DIR)/jidctred.o	\
			  $(EE_OBJS_DIR)/jdsample.o	\
			  $(EE_OBJS_DIR)/jdcolor.o	\
			  $(EE_OBJS_DIR)/jquant1.o	\
			  $(EE_OBJS_DIR)/jquant2.o	\
			  $(EE_OBJS_DIR)/jdmerge.o

# ps2 jpg library object files
PS2OBJECTS = $(EE_OBJS_DIR)/libjpg.o

# These objectfiles are included in libjpeg.a
LIBOBJECTS= $(CLIBOBJECTS) $(DLIBOBJECTS) $(COMOBJECTS) $(PS2OBJECTS)

EE_TARGET_DIR	= ../lib
EE_OBJS_DIR		= obj
EE_BIN_DIR		= bin

EE_INCS			= -I$(LIBJPG)/include -I.
EE_LIB			= libjpg.a
EE_OBJS			= $(LIBOBJECTS)


all:	$(EE_OBJS_DIR) $(EE_BIN_DIR) $(EE_LIB)
	mv $(EE_LIB) $(EE_BIN_DIR)/$(EE_LIB)
	cp $(EE_BIN_DIR)/$(EE_LIB) $(EE_TARGET_DIR)/$(EE_LIB)

clean:
	rm -f -r $(EE_OBJS_DIR) $(EE_BIN_DIR)
	rm -f $(EE_TARGET_DIR)/$(EE_LIB)

$(EE_OBJS_DIR):

include ../Makefile.pref
include ../Makefile.eeglobal
