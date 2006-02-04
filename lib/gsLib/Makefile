# myPS2 Makefile for gsLib
#
# Author: ntba2
# Date: 12-30-2005
#

EE_INCS		= -I.
EE_LIB		= libgslib.a

EE_CFLAGS	+= -DHAVE_LIBJPG -DHAVE_LIBPNG

EE_OBJS =	gsCore.o	\
		gsTexture.o	\
		gsPrimitive.o	\
		gsFont.o	\
		gsMisc.o

all:	$(EE_LIB)
	
clean:
	rm -f *.o *.a

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
