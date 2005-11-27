# myPS2 Makefile
#
# Author: ntba2
# Date: 11-6-2005
# This works with Visual Studio 6.
#

EE_OBJS_DIR		= obj
EE_BIN_DIR		= bin

EE_BIN	=	MYPS2.ELF

EE_OBJS =	$(EE_OBJS_DIR)/main.o			\
			$(EE_OBJS_DIR)/gr.o				\
			$(EE_OBJS_DIR)/ui_main.o		\
			$(EE_OBJS_DIR)/ui_menu.o		\
			$(EE_OBJS_DIR)/ui_picview.o		\
			$(EE_OBJS_DIR)/ui_manager.o		\
			$(EE_OBJS_DIR)/ui_elfloader.o	\
			$(EE_OBJS_DIR)/ui_install.o		\
			$(EE_OBJS_DIR)/ui_options.o		\
			$(EE_OBJS_DIR)/gamepad.o		\
			$(EE_OBJS_DIR)/elf.o			\
			$(EE_OBJS_DIR)/timer.o			\
			$(EE_OBJS_DIR)/arial.o			\
			$(EE_OBJS_DIR)/imgscale.o		\
			$(EE_OBJS_DIR)/file.o			\
			$(EE_OBJS_DIR)/sysconf.o		\
			$(EE_OBJS_DIR)/images.o			\
			$(EE_OBJS_DIR)/loader.o			\
			$(EE_OBJS_DIR)/usb_mass.o		\
			$(EE_OBJS_DIR)/iomanx.o		\
			$(EE_OBJS_DIR)/filexio.o		\
			$(EE_OBJS_DIR)/ps2dev9.o		\
			$(EE_OBJS_DIR)/ps2atad.o		\
			$(EE_OBJS_DIR)/ps2hdd.o		\
			$(EE_OBJS_DIR)/ps2fs.o		\
			$(EE_OBJS_DIR)/poweroff.o		\
			$(EE_OBJS_DIR)/cdvd.o			\
			$(EE_OBJS_DIR)/npm_usbd.o		\
			$(EE_OBJS_DIR)/ps2ip.o			\
			$(EE_OBJS_DIR)/ps2smap.o		\
			$(EE_OBJS_DIR)/ps2ftpd.o

EE_LIBS	=	-lc			\
			-lpacket	\
			-ldma		\
			-lpad		\
			-lcdvdfs	\
			-lfileXio	\
			-ljpg		\
			-lmf		\
			-lhdd		\
			-lmc		\
			-lpatches	\
			-lpoweroff	\
			-lusb

# add local lib directory
EE_LDFLAGS += -L./lib

all: $(EE_OBJS_DIR) $(EE_BIN_DIR) $(EE_BIN)
	mv $(EE_BIN) $(EE_BIN_DIR)/$(EE_BIN)

$(EE_OBJS_DIR)/loader.s:
	bin2s loader/bin/loader.elf $(EE_OBJS_DIR)/loader.s loader_elf

$(EE_OBJS_DIR)/usb_mass.s:
	bin2s usb_irx/bin/usb_mass.irx $(EE_OBJS_DIR)/usb_mass.s usb_mass_irx

$(EE_OBJS_DIR)/iomanx.s:
	bin2s irx/IOMANX.IRX $(EE_OBJS_DIR)/iomanx.s iomanx_irx

$(EE_OBJS_DIR)/filexio.s:
	bin2s irx/FILEXIO.IRX $(EE_OBJS_DIR)/filexio.s filexio_irx

$(EE_OBJS_DIR)/ps2dev9.s:
	bin2s irx/PS2DEV9.IRX $(EE_OBJS_DIR)/ps2dev9.s ps2dev9_irx

$(EE_OBJS_DIR)/ps2atad.s:
	bin2s irx/PS2ATAD.IRX $(EE_OBJS_DIR)/ps2atad.s ps2atad_irx

$(EE_OBJS_DIR)/ps2hdd.s:
	bin2s irx/PS2HDD.IRX $(EE_OBJS_DIR)/ps2hdd.s ps2hdd_irx

$(EE_OBJS_DIR)/ps2fs.s:
	bin2s irx/PS2FS.IRX $(EE_OBJS_DIR)/ps2fs.s ps2fs_irx

$(EE_OBJS_DIR)/poweroff.s:
	bin2s irx/POWEROFF.IRX $(EE_OBJS_DIR)/poweroff.s poweroff_irx

$(EE_OBJS_DIR)/cdvd.s:
	bin2s irx/CDVD.IRX $(EE_OBJS_DIR)/cdvd.s cdvd_irx

$(EE_OBJS_DIR)/npm_usbd.s:
	bin2s irx/NPM_USBD.IRX $(EE_OBJS_DIR)/npm_usbd.s npm_usbd_irx

$(EE_OBJS_DIR)/ps2ip.s:
	bin2s irx/PS2IP.IRX $(EE_OBJS_DIR)/ps2ip.s ps2ip_irx

$(EE_OBJS_DIR)/ps2smap.s:
	bin2s irx/PS2SMAP.IRX $(EE_OBJS_DIR)/ps2smap.s ps2smap_irx

$(EE_OBJS_DIR)/ps2ftpd.s:
	bin2s ps2ftpd/bin/ps2ftpd.irx $(EE_OBJS_DIR)/ps2ftpd.s ps2ftpd_irx

clean:
	rm -f -r $(EE_OBJS_DIR) $(EE_BIN_DIR)

include Makefile.pref
include Makefile.eeglobal

