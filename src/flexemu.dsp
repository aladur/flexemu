# Microsoft Developer Studio Project File - Name="flexemu" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=flexemu - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "flexemu.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "flexemu.mak" CFG="flexemu - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "flexemu - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "flexemu - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "flexemu - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "flexemu_Release"
# PROP Intermediate_Dir "flexemu_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp1 /MT /W3 /GX /Ox /Ot /Og /Oi /Ob2 /I "." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D WINVER=0x0400 /D _WIN32_WINNT=0x0400 /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib shell32.lib advapi32.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ELSEIF  "$(CFG)" == "flexemu - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "flexemu_Debug"
# PROP Intermediate_Dir "flexemu_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp1 /MTd /W3 /Gm /GX /Zi /Od /I "." /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D WINVER=0x0400 /D _WIN32_WINNT=0x0400 /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib shell32.lib advapi32.lib /nologo /subsystem:windows /incremental:no /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "flexemu - Win32 Release"
# Name "flexemu - Win32 Debug"
# Begin Group "Source files"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=.\absgui.cpp
# End Source File
# Begin Source File

SOURCE=.\acia1.cpp
# End Source File
# Begin Source File

SOURCE=.\bcommand.cpp
# End Source File
# Begin Source File

SOURCE=.\bdate.cpp
# End Source File
# Begin Source File

SOURCE=.\bdir.cpp
# End Source File
# Begin Source File

SOURCE=.\benv.cpp
# End Source File
# Begin Source File

SOURCE=.\bfileptr.cpp
# End Source File
# Begin Source File

SOURCE=.\bmembuf.cpp
# End Source File
# Begin Source File

SOURCE=.\bmutex.cpp
# End Source File
# Begin Source File

SOURCE=.\brcfile.cpp
# End Source File
# Begin Source File

SOURCE=.\bregistr.cpp
# End Source File
# Begin Source File

SOURCE=.\bstring.cpp
# End Source File
# Begin Source File

SOURCE=.\bthread.cpp
# End Source File
# Begin Source File

SOURCE=.\bthreadfactory.cpp
# End Source File
# Begin Source File

SOURCE=.\bthreadimp.cpp
# End Source File
# Begin Source File

SOURCE=.\btime.cpp
# End Source File
# Begin Source File

SOURCE=.\btimer.cpp
# End Source File
# Begin Source File

SOURCE=.\bwin32threadimp.cpp
# End Source File
# Begin Source File

SOURCE=.\cacttrns.cpp
# End Source File
# Begin Source File

SOURCE=.\clogfile.cpp
# End Source File
# Begin Source File

SOURCE=.\colors.cpp
# End Source File
# Begin Source File

SOURCE=.\command.cpp
# End Source File
# Begin Source File

SOURCE=.\csetfreq.cpp
# End Source File
# Begin Source File

SOURCE=.\da6809.cpp
# End Source File
# Begin Source File

SOURCE=.\e2floppy.cpp
# End Source File
# Begin Source File

SOURCE=.\e2video.cpp
# End Source File
# Begin Source File

SOURCE=.\fcinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\fcopyman.cpp
# End Source File
# Begin Source File

SOURCE=.\fdirent.cpp
# End Source File
# Begin Source File

SOURCE=.\ffilebuf.cpp
# End Source File
# Begin Source File

SOURCE=.\ffilecnt.cpp
# End Source File
# Begin Source File

SOURCE=.\flexerr.cpp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\flexmain.ico
# End Source File
# Begin Source File

SOURCE=.\foptman.cpp
# End Source File
# Begin Source File

SOURCE=.\iffilcnt.cpp
# End Source File
# Begin Source File

SOURCE=.\ifilecnt.cpp
# End Source File
# Begin Source File

SOURCE=.\inout.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\mc146818.cpp
# End Source File
# Begin Source File

SOURCE=.\mc6809.cpp
# End Source File
# Begin Source File

SOURCE=.\mc6809in.cpp
# End Source File
# Begin Source File

SOURCE=.\mc6809st.cpp
# End Source File
# Begin Source File

SOURCE=.\mc6821.cpp
# End Source File
# Begin Source File

SOURCE=.\mc6850.cpp
# End Source File
# Begin Source File

SOURCE=.\memory.cpp
# End Source File
# Begin Source File

SOURCE=.\misc1.cpp
# End Source File
# Begin Source File

SOURCE=.\mmu.cpp
# End Source File
# Begin Source File

SOURCE=.\ndircont.cpp
# End Source File
# Begin Source File

SOURCE=.\pia1.cpp
# End Source File
# Begin Source File

SOURCE=.\pia2.cpp
# End Source File
# Begin Source File

SOURCE=.\rfilecnt.cpp
# End Source File
# Begin Source File

SOURCE=.\schedule.cpp
# End Source File
# Begin Source File

SOURCE=.\wd1793.cpp
# End Source File
# Begin Source File

SOURCE=.\win32gui.cpp
# End Source File
# End Group
# Begin Group "Resource files"

# PROP Default_Filter "rc;ico"
# Begin Source File

SOURCE=.\bitmaps\flexcpu.ico
# End Source File
# Begin Source File

SOURCE=.\bitmaps\flexemu.ico
# End Source File
# Begin Source File

SOURCE=.\bitmaps\flexemu1.ico
# End Source File
# Begin Source File

SOURCE=.\bitmaps\flexemu2.ico
# End Source File
# Begin Source File

SOURCE=.\bitmaps\flexemu3.ico
# End Source File
# Begin Source File

SOURCE=.\bitmaps\floppy0.ico
# End Source File
# Begin Source File

SOURCE=.\bitmaps\floppy0.ico
# End Source File
# Begin Source File

SOURCE=.\bitmaps\floppy1.ico
# End Source File
# Begin Source File

SOURCE=.\bitmaps\floppy2.ico
# End Source File
# Begin Source File

SOURCE=.\bitmaps\irq0.ico
# End Source File
# Begin Source File

SOURCE=.\bitmaps\irq1.ico
# End Source File
# Begin Source File

SOURCE=.\bitmaps\irq2.ico
# End Source File
# Begin Source File

SOURCE=.\bitmaps\irq3.ico
# End Source File
# Begin Source File

SOURCE=.\bitmaps\irq4.ico
# End Source File
# Begin Source File

SOURCE=.\bitmaps\resize.ico
# End Source File
# Begin Source File

SOURCE=.\winres.rc
# End Source File
# End Group
# Begin Group "Header files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\absdisas.h
# End Source File
# Begin Source File

SOURCE=.\absgui.h
# End Source File
# Begin Source File

SOURCE=.\acia1.h
# End Source File
# Begin Source File

SOURCE=.\bcommand.h
# End Source File
# Begin Source File

SOURCE=.\bdate.h
# End Source File
# Begin Source File

SOURCE=.\bdelete.h
# End Source File
# Begin Source File

SOURCE=.\bdir.h
# End Source File
# Begin Source File

SOURCE=.\benv.h
# End Source File
# Begin Source File

SOURCE=.\bfileptr.h
# End Source File
# Begin Source File

SOURCE=.\bjoystck.h
# End Source File
# Begin Source File

SOURCE=.\bmembuf.h
# End Source File
# Begin Source File

SOURCE=.\bmutex.h
# End Source File
# Begin Source File

SOURCE=.\brcfile.h
# End Source File
# Begin Source File

SOURCE=.\bregistr.h
# End Source File
# Begin Source File

SOURCE=.\bsingle.h
# End Source File
# Begin Source File

SOURCE=.\bstring.h
# End Source File
# Begin Source File

SOURCE=.\bthread.h
# End Source File
# Begin Source File

SOURCE=.\bthreadfactory.h
# End Source File
# Begin Source File

SOURCE=.\bthreadimp.h
# End Source File
# Begin Source File

SOURCE=.\btime.h
# End Source File
# Begin Source File

SOURCE=.\btimer.h
# End Source File
# Begin Source File

SOURCE=.\bwin32threadimp.h
# End Source File
# Begin Source File

SOURCE=.\cacttrns.h
# End Source File
# Begin Source File

SOURCE=.\classes.h
# End Source File
# Begin Source File

SOURCE=.\clogfile.h
# End Source File
# Begin Source File

SOURCE=.\command.h
# End Source File
# Begin Source File

SOURCE=.\confignt.h
# End Source File
# Begin Source File

SOURCE=.\cpustate.h
# End Source File
# Begin Source File

SOURCE=.\csetfreq.h
# End Source File
# Begin Source File

SOURCE=.\da6809.h
# End Source File
# Begin Source File

SOURCE=.\debug.h
# End Source File
# Begin Source File

SOURCE=.\e2.h
# End Source File
# Begin Source File

SOURCE=.\e2floppy.h
# End Source File
# Begin Source File

SOURCE=.\e2video.h
# End Source File
# Begin Source File

SOURCE=.\extmem.h
# End Source File
# Begin Source File

SOURCE=.\fcinfo.h
# End Source File
# Begin Source File

SOURCE=.\fcopyman.h
# End Source File
# Begin Source File

SOURCE=.\fdate.h
# End Source File
# Begin Source File

SOURCE=.\fdirent.h
# End Source File
# Begin Source File

SOURCE=.\ffilebuf.h
# End Source File
# Begin Source File

SOURCE=.\ffilecnt.h
# End Source File
# Begin Source File

SOURCE=.\filecntb.h
# End Source File
# Begin Source File

SOURCE=.\filecnts.h
# End Source File
# Begin Source File

SOURCE=.\filecont.h
# End Source File
# Begin Source File

SOURCE=.\flexemu.h
# End Source File
# Begin Source File

SOURCE=.\flexerr.h
# End Source File
# Begin Source File

SOURCE=.\foptman.h
# End Source File
# Begin Source File

SOURCE=.\fstring.h
# End Source File
# Begin Source File

SOURCE=.\idircnt.h
# End Source File
# Begin Source File

SOURCE=.\iffilcnt.h
# End Source File
# Begin Source File

SOURCE=.\ifilcnti.h
# End Source File
# Begin Source File

SOURCE=.\ifilecnt.h
# End Source File
# Begin Source File

SOURCE=.\inout.h
# End Source File
# Begin Source File

SOURCE=.\intmem.h
# End Source File
# Begin Source File

SOURCE=.\iodevice.h
# End Source File
# Begin Source File

SOURCE=.\mc146818.h
# End Source File
# Begin Source File

SOURCE=.\mc6809.h
# End Source File
# Begin Source File

SOURCE=.\mc6809st.h
# End Source File
# Begin Source File

SOURCE=.\mc6821.h
# End Source File
# Begin Source File

SOURCE=.\mc6850.h
# End Source File
# Begin Source File

SOURCE=.\memory.h
# End Source File
# Begin Source File

SOURCE=.\misc1.h
# End Source File
# Begin Source File

SOURCE=.\mmu.h
# End Source File
# Begin Source File

SOURCE=.\ndircont.h
# End Source File
# Begin Source File

SOURCE=.\pia1.h
# End Source File
# Begin Source File

SOURCE=.\pia2.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\rfilecnt.h
# End Source File
# Begin Source File

SOURCE=.\schedcpu.h
# End Source File
# Begin Source File

SOURCE=.\schedule.h
# End Source File
# Begin Source File

SOURCE=.\typedefs.h
# End Source File
# Begin Source File

SOURCE=.\wd1793.h
# End Source File
# Begin Source File

SOURCE=.\win32gui.h
# End Source File
# End Group
# End Target
# End Project
