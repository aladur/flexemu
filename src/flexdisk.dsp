# Microsoft Developer Studio Project File - Name="flexdisk" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=flexdisk - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "flexdisk.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "flexdisk.mak" CFG="flexdisk - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "flexdisk - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "flexdisk - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "flexdisk - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "flexdisk_Release"
# PROP Intermediate_Dir "flexdisk_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "__WXMSW__" /D "__WINDOWS__" /D DEBUG=0 /D "__WIN95__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib comctl32.lib wsock32.lib wxbase28.lib /nologo /subsystem:windows /machine:I386 /out:"flexdisk_Release/FLEXplorer.exe"
# SUBTRACT LINK32 /pdb:none /incremental:yes /nodefaultlib

!ELSEIF  "$(CFG)" == "flexdisk - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "flexdisk"
# PROP BASE Intermediate_Dir "flexdisk"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "flexdisk_Debug"
# PROP Intermediate_Dir "flexdisk_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "." /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "__WXMSW__" /D "__WINDOWS__" /D DEBUG=1 /D "__WXDEBUG__" /D "__WIN95__" /D "__WIN32__" /D WINVER=0x0400 /D "STRICT" /FR /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib comctl32.lib rpcrt4.lib wsock32.lib wxbase28d.lib /nologo /subsystem:windows /incremental:no /debug /machine:I386 /out:"flexdisk_Debug/FLEXplorer.exe" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ENDIF 

# Begin Target

# Name "flexdisk - Win32 Release"
# Name "flexdisk - Win32 Debug"
# Begin Group "Source files"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=.\baddrrng.cpp
# End Source File
# Begin Source File

SOURCE=.\bbintree.cpp
# End Source File
# Begin Source File

SOURCE=.\bdate.cpp
# End Source File
# Begin Source File

SOURCE=.\bdir.cpp
# End Source File
# Begin Source File

SOURCE=.\bfileptr.cpp
# End Source File
# Begin Source File

SOURCE=.\bhashtbl.cpp
# End Source File
# Begin Source File

SOURCE=.\bident.cpp
# End Source File
# Begin Source File

SOURCE=.\bintervl.cpp
# End Source File
# Begin Source File

SOURCE=.\blist.cpp
# End Source File
# Begin Source File

SOURCE=.\bmembuf.cpp
# End Source File
# Begin Source File

SOURCE=.\bprocess.cpp
# End Source File
# Begin Source File

SOURCE=.\bregistr.cpp
# End Source File
# Begin Source File

SOURCE=.\bsortlst.cpp
# End Source File
# Begin Source File

SOURCE=.\bstring.cpp
# End Source File
# Begin Source File

SOURCE=.\buint.cpp
# End Source File
# Begin Source File

SOURCE=.\contpdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\da.cpp
# End Source File
# Begin Source File

SOURCE=.\da6809.cpp
# End Source File
# Begin Source File

SOURCE=.\dircont.cpp
# End Source File
# Begin Source File

SOURCE=.\disconf.cpp
# End Source File
# Begin Source File

SOURCE=.\fcinfo.cpp

!IF  "$(CFG)" == "flexdisk - Win32 Release"

!ELSEIF  "$(CFG)" == "flexdisk - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fclipbrd.cpp
# End Source File
# Begin Source File

SOURCE=.\fcopyman.cpp

!IF  "$(CFG)" == "flexdisk - Win32 Release"

!ELSEIF  "$(CFG)" == "flexdisk - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fdcframe.cpp
# End Source File
# Begin Source File

SOURCE=.\fddnd.cpp
# End Source File
# Begin Source File

SOURCE=.\fdirent.cpp

!IF  "$(CFG)" == "flexdisk - Win32 Release"

!ELSEIF  "$(CFG)" == "flexdisk - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fdlist.cpp
# End Source File
# Begin Source File

SOURCE=.\fdpframe.cpp
# End Source File
# Begin Source File

SOURCE=.\ffilebuf.cpp
# End Source File
# Begin Source File

SOURCE=.\ffilecnt.cpp

!IF  "$(CFG)" == "flexdisk - Win32 Release"

!ELSEIF  "$(CFG)" == "flexdisk - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\flexdisk.cpp

!IF  "$(CFG)" == "flexdisk - Win32 Release"

!ELSEIF  "$(CFG)" == "flexdisk - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\flexerr.cpp

!IF  "$(CFG)" == "flexdisk - Win32 Release"

!ELSEIF  "$(CFG)" == "flexdisk - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\flexerrl.cpp
# End Source File
# Begin Source File

SOURCE=.\fmenufac.cpp
# End Source File
# Begin Source File

SOURCE=.\idircnt.cpp
# End Source File
# Begin Source File

SOURCE=.\iffilcnt.cpp
# End Source File
# Begin Source File

SOURCE=.\ifilecnt.cpp
# End Source File
# Begin Source File

SOURCE=.\misc1.cpp

!IF  "$(CFG)" == "flexdisk - Win32 Release"

!ELSEIF  "$(CFG)" == "flexdisk - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ndircont.cpp

!IF  "$(CFG)" == "flexdisk - Win32 Release"

!ELSEIF  "$(CFG)" == "flexdisk - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\optdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\rfilecnt.cpp

!IF  "$(CFG)" == "flexdisk - Win32 Release"

!ELSEIF  "$(CFG)" == "flexdisk - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# End Group
# Begin Group "Header files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\absdisas.h
# End Source File
# Begin Source File

SOURCE=.\baddrrng.h
# End Source File
# Begin Source File

SOURCE=.\bbintree.h
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

SOURCE=.\bhashtbl.h
# End Source File
# Begin Source File

SOURCE=.\bident.h
# End Source File
# Begin Source File

SOURCE=.\bintervl.h
# End Source File
# Begin Source File

SOURCE=.\blist.h
# End Source File
# Begin Source File

SOURCE=.\bmembuf.h
# End Source File
# Begin Source File

SOURCE=.\bobserv.h
# End Source File
# Begin Source File

SOURCE=.\bprocess.h
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

SOURCE=.\bsortlst.h
# End Source File
# Begin Source File

SOURCE=.\bstring.h
# End Source File
# Begin Source File

SOURCE=.\buint.h
# End Source File
# Begin Source File

SOURCE=.\confignt.h
# End Source File
# Begin Source File

SOURCE=.\contpdlg.h
# End Source File
# Begin Source File

SOURCE=.\da.h
# End Source File
# Begin Source File

SOURCE=.\da6809.h
# End Source File
# Begin Source File

SOURCE=.\debug.h
# End Source File
# Begin Source File

SOURCE=.\dircont.h
# End Source File
# Begin Source File

SOURCE=.\disconf.h
# End Source File
# Begin Source File

SOURCE=.\fcinfo.h
# End Source File
# Begin Source File

SOURCE=.\fclipbrd.h
# End Source File
# Begin Source File

SOURCE=.\fcopyman.h
# End Source File
# Begin Source File

SOURCE=.\fdcframe.h
# End Source File
# Begin Source File

SOURCE=.\fddnd.h
# End Source File
# Begin Source File

SOURCE=.\fdirent.h
# End Source File
# Begin Source File

SOURCE=.\fdlist.h
# End Source File
# Begin Source File

SOURCE=.\fdpframe.h
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

SOURCE=.\flexdisk.h
# End Source File
# Begin Source File

SOURCE=.\flexemu.h
# End Source File
# Begin Source File

SOURCE=.\flexerr.h
# End Source File
# Begin Source File

SOURCE=.\flexerrl.h
# End Source File
# Begin Source File

SOURCE=.\fmenufac.h
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

SOURCE=.\misc1.h
# End Source File
# Begin Source File

SOURCE=.\ndircont.h
# End Source File
# Begin Source File

SOURCE=.\optdlg.h
# End Source File
# Begin Source File

SOURCE=.\rfilecnt.h
# End Source File
# Begin Source File

SOURCE=.\typedefs.h
# End Source File
# End Group
# Begin Group "Resource files"

# PROP Default_Filter "rc"
# Begin Source File

SOURCE=.\wx\msw\blank.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\bullseye.cur
# End Source File
# Begin Source File

SOURCE=.\bitmaps\chart.ico
# End Source File
# Begin Source File

SOURCE=.\bitmaps\contain.ico
# End Source File
# Begin Source File

SOURCE=.\bitmaps\copy.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\cut.bmp
# End Source File
# Begin Source File

SOURCE=.\wx\msw\error.ico
# End Source File
# Begin Source File

SOURCE=.\bitmaps\flexdisk.ico
# End Source File
# Begin Source File

SOURCE=.\flexdisk.rc
# End Source File
# Begin Source File

SOURCE=.\flexdisk.wxr
# End Source File
# Begin Source File

SOURCE=.\wx\msw\hand.cur
# End Source File
# Begin Source File

SOURCE=.\bitmaps\help.bmp
# End Source File
# Begin Source File

SOURCE=.\wx\msw\info.ico
# End Source File
# Begin Source File

SOURCE=.\wx\msw\magnif1.cur
# End Source File
# Begin Source File

SOURCE=.\bitmaps\mondrian.ico
# End Source File
# Begin Source File

SOURCE=.\bitmaps\new.bmp
# End Source File
# Begin Source File

SOURCE=.\wx\msw\noentry.cur
# End Source File
# Begin Source File

SOURCE=.\bitmaps\open.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\open_con.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\open_dir.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmaps\paste.bmp
# End Source File
# Begin Source File

SOURCE=.\wx\msw\pbrush.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\pencil.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\pntleft.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\pntright.cur
# End Source File
# Begin Source File

SOURCE=.\bitmaps\print.bmp
# End Source File
# Begin Source File

SOURCE=.\wx\msw\query.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\question.ico
# End Source File
# Begin Source File

SOURCE=.\wx\msw\roller.cur
# End Source File
# Begin Source File

SOURCE=.\bitmaps\save.bmp
# End Source File
# Begin Source File

SOURCE=.\wx\msw\size.cur
# End Source File
# Begin Source File

SOURCE=.\wx\msw\tip.ico
# End Source File
# Begin Source File

SOURCE=.\wx\msw\warning.ico
# End Source File
# Begin Source File

SOURCE=.\wx\msw\watch1.cur
# End Source File
# End Group
# End Target
# End Project
