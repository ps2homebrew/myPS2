# Microsoft Developer Studio Project File - Name="libmad" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=libmad - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libmad.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libmad.mak" CFG="libmad - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libmad - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "libmad - Win32 Debug" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "libmad - Win32 Release"

# PROP BASE Use_MFC
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Cmd_Line "NMAKE /f libmad.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "libmad.exe"
# PROP BASE Bsc_Name "libmad.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../vc6/Release"
# PROP Intermediate_Dir "../vc6/Release"
# PROP Cmd_Line "make"
# PROP Rebuild_Opt "clean"
# PROP Target_File "libmad.a"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "libmad - Win32 Debug"

# PROP BASE Use_MFC
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Cmd_Line "NMAKE /f libmad.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "libmad.exe"
# PROP BASE Bsc_Name "libmad.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../vc6/Debug"
# PROP Intermediate_Dir "../vc6/Debug"
# PROP Cmd_Line "make"
# PROP Rebuild_Opt "clean"
# PROP Target_File "libmad.a"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "libmad - Win32 Release"
# Name "libmad - Win32 Debug"

!IF  "$(CFG)" == "libmad - Win32 Release"

!ELSEIF  "$(CFG)" == "libmad - Win32 Debug"

!ENDIF 

# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\bit.c
# End Source File
# Begin Source File

SOURCE=.\decoder.c
# End Source File
# Begin Source File

SOURCE=.\fixed.c
# End Source File
# Begin Source File

SOURCE=.\frame.c
# End Source File
# Begin Source File

SOURCE=.\huffman.c
# End Source File
# Begin Source File

SOURCE=.\layer12.c
# End Source File
# Begin Source File

SOURCE=.\layer3.c
# End Source File
# Begin Source File

SOURCE=.\minimad.c
# End Source File
# Begin Source File

SOURCE=.\stream.c
# End Source File
# Begin Source File

SOURCE=.\synth.c
# End Source File
# Begin Source File

SOURCE=.\timer.c
# End Source File
# Begin Source File

SOURCE=.\version.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\bit.h
# End Source File
# Begin Source File

SOURCE=.\include\config.h
# End Source File
# Begin Source File

SOURCE=.\include\decoder.h
# End Source File
# Begin Source File

SOURCE=.\include\fixed.h
# End Source File
# Begin Source File

SOURCE=.\include\frame.h
# End Source File
# Begin Source File

SOURCE=.\include\global.h
# End Source File
# Begin Source File

SOURCE=.\include\huffman.h
# End Source File
# Begin Source File

SOURCE=.\include\layer12.h
# End Source File
# Begin Source File

SOURCE=.\include\layer3.h
# End Source File
# Begin Source File

SOURCE=.\include\mad.h
# End Source File
# Begin Source File

SOURCE=.\include\stream.h
# End Source File
# Begin Source File

SOURCE=.\include\synth.h
# End Source File
# Begin Source File

SOURCE=.\include\timer.h
# End Source File
# Begin Source File

SOURCE=.\include\version.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Makefile
# End Source File
# End Group
# End Target
# End Project
