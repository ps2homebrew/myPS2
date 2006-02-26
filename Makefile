# myPS2 main Makefile
#
# Author: ntba2
# Date: 12-22-2005
#

all:
	make -C elf/loader/
	make -C irx/ps2ftpd/
	make -C irx/usb_mass/
	make -C lib/libjpg/
	make -C lib/dmaKit/
	make -C lib/gsLib/
	make -C lib/libps2time/
	make -C src/
	cp src/bin/MYPS2.ELF MYPS2.ELF

clean:
	make -C elf/loader/ clean
	make -C irx/ps2ftpd/ clean
	make -C irx/usb_mass/ clean
	make -C lib/libjpg/ clean
	make -C lib/dmaKit/ clean
	make -C lib/gsLib/ clean
	make -C lib/libps2time/ clean
	make -C src/ clean
	rm -f MYPS2.ELF