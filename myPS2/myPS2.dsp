# Microsoft Developer Studio Project File - Name="myPS2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=myPS2 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "myPS2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "myPS2.mak" CFG="myPS2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "myPS2 - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "myPS2 - Win32 Debug" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "myPS2 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Cmd_Line "NMAKE /f myPS2.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "myPS2.exe"
# PROP BASE Bsc_Name "myPS2.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "vc6/Release"
# PROP Intermediate_Dir "vc6/Release"
# PROP Cmd_Line "make"
# PROP Rebuild_Opt "clean"
# PROP Target_File "myPS2.elf"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "myPS2 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Cmd_Line "NMAKE /f myPS2.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "myPS2.exe"
# PROP BASE Bsc_Name "myPS2.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "vc6/Debug"
# PROP Intermediate_Dir "vc6/Debug"
# PROP Cmd_Line "make"
# PROP Rebuild_Opt "clean"
# PROP Target_File "myPS2.elf"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "myPS2 - Win32 Release"
# Name "myPS2 - Win32 Debug"

!IF  "$(CFG)" == "myPS2 - Win32 Release"

!ELSEIF  "$(CFG)" == "myPS2 - Win32 Debug"

!ENDIF 

# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\arial.c
# End Source File
# Begin Source File

SOURCE=.\elf.c
# End Source File
# Begin Source File

SOURCE=.\file.c
# End Source File
# Begin Source File

SOURCE=.\gamepad.c
# End Source File
# Begin Source File

SOURCE=.\gr.c
# End Source File
# Begin Source File

SOURCE=.\images.c
# End Source File
# Begin Source File

SOURCE=.\imgscale.c
# End Source File
# Begin Source File

SOURCE=.\main.c
# End Source File
# Begin Source File

SOURCE=.\mcbxml.c
# End Source File
# Begin Source File

SOURCE=.\mp3.c
# End Source File
# Begin Source File

SOURCE=.\net.c
# End Source File
# Begin Source File

SOURCE=.\scheduler.c
# End Source File
# Begin Source File

SOURCE=.\sysconf.c
# End Source File
# Begin Source File

SOURCE=.\timer.c
# End Source File
# Begin Source File

SOURCE=.\ui_elfloader.c
# End Source File
# Begin Source File

SOURCE=.\ui_install.c
# End Source File
# Begin Source File

SOURCE=.\ui_main.c
# End Source File
# Begin Source File

SOURCE=.\ui_manager.c
# End Source File
# Begin Source File

SOURCE=.\ui_menu.c
# End Source File
# Begin Source File

SOURCE=.\ui_music.c
# End Source File
# Begin Source File

SOURCE=.\ui_options.c
# End Source File
# Begin Source File

SOURCE=.\ui_picview.c
# End Source File
# Begin Source File

SOURCE=.\ui_radio.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\elf.h
# End Source File
# Begin Source File

SOURCE=.\file.h
# End Source File
# Begin Source File

SOURCE=.\gamepad.h
# End Source File
# Begin Source File

SOURCE=.\gr.h
# End Source File
# Begin Source File

SOURCE=.\gr_regs.h
# End Source File
# Begin Source File

SOURCE=.\imgscale.h
# End Source File
# Begin Source File

SOURCE=.\mcbxml.h
# End Source File
# Begin Source File

SOURCE=.\misc.h
# End Source File
# Begin Source File

SOURCE=.\mp3.h
# End Source File
# Begin Source File

SOURCE=.\net.h
# End Source File
# Begin Source File

SOURCE=.\scheduler.h
# End Source File
# Begin Source File

SOURCE=.\sysconf.h
# End Source File
# Begin Source File

SOURCE=.\ui.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Makefile
# End Source File
# Begin Source File

SOURCE=.\Makefile.eeglobal
# End Source File
# Begin Source File

SOURCE=.\Makefile.iopglobal
# End Source File
# Begin Source File

SOURCE=.\Makefile.pref
# End Source File
# End Group
# End Target
# End Project
