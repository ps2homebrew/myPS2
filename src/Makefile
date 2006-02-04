# myPS2 Makefile
#
# Author: ntba2
# Date: 12-22-2005
#

EE_OBJS_DIR	= obj
EE_BIN_DIR	= bin
EE_BIN		= MYPS2.ELF

#DEVELOPER	= 1

EE_OBJS =	$(EE_OBJS_DIR)/main.o			\
		$(EE_OBJS_DIR)/GUI.o			\
		$(EE_OBJS_DIR)/GUI_Load.o		\
		$(EE_OBJS_DIR)/GUI_Ctrl_Button.o	\
		$(EE_OBJS_DIR)/GUI_Ctrl_Combo.o		\
		$(EE_OBJS_DIR)/GUI_Ctrl_Dirview.o	\
		$(EE_OBJS_DIR)/GUI_Ctrl_Image.o		\
		$(EE_OBJS_DIR)/GUI_Ctrl_Label.o		\
		$(EE_OBJS_DIR)/GUI_Ctrl_List.o		\
		$(EE_OBJS_DIR)/GUI_Ctrl_Progress.o	\
		$(EE_OBJS_DIR)/GUI_Ctrl_Slider.o	\
		$(EE_OBJS_DIR)/GUI_Ctrl_ThumbnailPanel.o\
		$(EE_OBJS_DIR)/GUI_Ctrl_ToggleButton.o	\
		$(EE_OBJS_DIR)/GUI_M_DlgKeyboard.o	\
		$(EE_OBJS_DIR)/GUI_M_DlgMsgBox.o	\
		$(EE_OBJS_DIR)/GUI_M_DlgPartition.o	\
		$(EE_OBJS_DIR)/GUI_M_DlgYesNo.o		\
		$(EE_OBJS_DIR)/GUI_M_Language.o		\
		$(EE_OBJS_DIR)/GUI_M_Main.o		\
		$(EE_OBJS_DIR)/GUI_M_MyFiles.o		\
		$(EE_OBJS_DIR)/GUI_M_MyMusic.o		\
		$(EE_OBJS_DIR)/GUI_M_MyPictures.o	\
		$(EE_OBJS_DIR)/GUI_M_MyPrograms.o	\
		$(EE_OBJS_DIR)/GUI_M_Network.o		\
		$(EE_OBJS_DIR)/GUI_M_Partitions.o	\
		$(EE_OBJS_DIR)/GUI_M_Radio.o		\
		$(EE_OBJS_DIR)/GUI_M_Settings.o		\
		$(EE_OBJS_DIR)/GUI_M_Skins.o		\
		$(EE_OBJS_DIR)/GUI_M_View.o		\
		$(EE_OBJS_DIR)/gamepad.o		\
		$(EE_OBJS_DIR)/elf.o			\
		$(EE_OBJS_DIR)/timer.o			\
		$(EE_OBJS_DIR)/imgscale.o		\
		$(EE_OBJS_DIR)/file.o			\
		$(EE_OBJS_DIR)/sysconf.o		\
		$(EE_OBJS_DIR)/mp3.o			\
		$(EE_OBJS_DIR)/scheduler.o		\
		$(EE_OBJS_DIR)/net.o			\
		$(EE_OBJS_DIR)/bootscreen.o		\
		$(EE_OBJS_DIR)/mass_rpc.o		\
		$(EE_OBJS_DIR)/cdvd_rpc.o		\
		$(EE_OBJS_DIR)/charset.o		\
		$(EE_OBJS_DIR)/sjpcm_rpc.o		\
		$(EE_OBJS_DIR)/loader.o			\
		$(EE_OBJS_DIR)/usb_mass.o		\
		$(EE_OBJS_DIR)/iomanx.o			\
		$(EE_OBJS_DIR)/filexio.o		\
		$(EE_OBJS_DIR)/ps2dev9.o		\
		$(EE_OBJS_DIR)/ps2atad.o		\
		$(EE_OBJS_DIR)/ps2hdd.o			\
		$(EE_OBJS_DIR)/ps2fs.o			\
		$(EE_OBJS_DIR)/poweroff.o		\
		$(EE_OBJS_DIR)/cdvd.o			\
		$(EE_OBJS_DIR)/ps2ip.o			\
		$(EE_OBJS_DIR)/ps2ips.o			\
		$(EE_OBJS_DIR)/ps2smap.o		\
		$(EE_OBJS_DIR)/ps2ftpd.o		\
		$(EE_OBJS_DIR)/sjpcm.o			\
		$(EE_OBJS_DIR)/boot_bkg.o		\
		$(EE_OBJS_DIR)/boot_font.o		\
		$(EE_OBJS_DIR)/boot_dat.o		\


EE_LIBS	=	-lc		\
		-lgslib		\
		-ldmaKit	\
		-lpad		\
		-lfileXio	\
		-ljpg		\
		-lmf		\
		-lhdd		\
		-lpng		\
		-lm		\
		-lz		\
		-lmc		\
		-lpatches	\
		-lpoweroff	\
		-lmad		\
		-lsample	\
		-lcdvd		\
		-ldebug		\
		-lps2ip		\
		-lscew		\
		-lexpat		\
		-liconv		\


# add lib directories
EE_LDFLAGS +=	-L../lib/gsLib -L../lib/dmaKit -L../lib/libmad -L../lib/libsample -L../lib/libjpg \
		-L../lib/libexpat -L../lib/libscew -L../lib/libpng -L../lib/libz \
		-L../lib/libiconv

# add include directories
EE_INCS += -I../lib/libexpat

ifdef DEVELOPER
EE_CFLAGS += -D_DEVELOPER
endif

all: $(EE_OBJS_DIR) $(EE_BIN_DIR) $(EE_BIN)
	mv $(EE_BIN) $(EE_BIN_DIR)/$(EE_BIN)

$(EE_OBJS_DIR):
	@$(MKDIR) -p $(EE_OBJS_DIR)

$(EE_BIN_DIR):
	@$(MKDIR) -p $(EE_BIN_DIR)

$(EE_OBJS_DIR)/%.o : %.c
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)/%.o : %.s
	$(EE_AS) $(EE_ASFLAGS) $< -o $@

$(EE_OBJS_DIR)/%.o : %.S
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)/loader.s:
	bin2s ../elf/loader/loader.elf $(EE_OBJS_DIR)/loader.s loader_elf

$(EE_OBJS_DIR)/usb_mass.s:
	bin2s ../irx/usb_mass/usb_mass.irx $(EE_OBJS_DIR)/usb_mass.s usb_mass_irx

$(EE_OBJS_DIR)/iomanx.s:
	bin2s $(PS2SDK)/iop/irx/iomanx.irx $(EE_OBJS_DIR)/iomanx.s iomanx_irx

$(EE_OBJS_DIR)/filexio.s:
	bin2s $(PS2SDK)/iop/irx/filexio.irx $(EE_OBJS_DIR)/filexio.s filexio_irx

$(EE_OBJS_DIR)/ps2dev9.s:
	bin2s $(PS2SDK)/iop/irx/ps2dev9.irx $(EE_OBJS_DIR)/ps2dev9.s ps2dev9_irx

$(EE_OBJS_DIR)/ps2atad.s:
	bin2s $(PS2SDK)/iop/irx/ps2atad.irx $(EE_OBJS_DIR)/ps2atad.s ps2atad_irx

$(EE_OBJS_DIR)/ps2hdd.s:
	bin2s $(PS2SDK)/iop/irx/ps2hdd.irx $(EE_OBJS_DIR)/ps2hdd.s ps2hdd_irx

$(EE_OBJS_DIR)/ps2fs.s:
	bin2s $(PS2SDK)/iop/irx/ps2fs.irx $(EE_OBJS_DIR)/ps2fs.s ps2fs_irx

$(EE_OBJS_DIR)/poweroff.s:
	bin2s $(PS2SDK)/iop/irx/poweroff.irx $(EE_OBJS_DIR)/poweroff.s poweroff_irx

$(EE_OBJS_DIR)/cdvd.s:
	bin2s ../irx/cdvd/cdvd.irx $(EE_OBJS_DIR)/cdvd.s cdvd_irx

$(EE_OBJS_DIR)/ps2ip.s:
	bin2s $(PS2SDK)/iop/irx/PS2IP.IRX $(EE_OBJS_DIR)/ps2ip.s ps2ip_irx

$(EE_OBJS_DIR)/ps2ips.s:
	bin2s $(PS2SDK)/iop/irx/ps2ips.irx $(EE_OBJS_DIR)/ps2ips.s ps2ips_irx

$(EE_OBJS_DIR)/ps2smap.s:
	bin2s $(PS2SDK)/iop/irx/PS2SMAP.IRX $(EE_OBJS_DIR)/ps2smap.s ps2smap_irx

$(EE_OBJS_DIR)/ps2ftpd.s:
	bin2s ../irx/ps2ftpd/ps2ftpd.irx $(EE_OBJS_DIR)/ps2ftpd.s ps2ftpd_irx

$(EE_OBJS_DIR)/sjpcm.s:
	bin2s ../irx/sjpcm/sjpcm.irx $(EE_OBJS_DIR)/sjpcm.s sjpcm_irx

$(EE_OBJS_DIR)/boot_bkg.s:
	bin2s ../data/boot_bkg.png $(EE_OBJS_DIR)/boot_bkg.s boot_bkg

$(EE_OBJS_DIR)/boot_font.s:
	bin2s ../data/boot_font.png $(EE_OBJS_DIR)/boot_font.s boot_font

$(EE_OBJS_DIR)/boot_dat.s:
	bin2s ../data/boot_dat.dat $(EE_OBJS_DIR)/boot_dat.s boot_dat

clean:
	rm -f -r $(EE_OBJS_DIR)
	rm -f $(EE_BIN_DIR)/$(EE_BIN)
ifndef DEVELOPER
	rm -f -r $(EE_OBJS_DIR) $(EE_BIN_DIR)
endif

include $(PS2SDK)/Defs.make
include $(PS2SDK)/samples/Makefile.eeglobal
