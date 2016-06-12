; flexemu.nsi
;
;--------------------------------

;--------------------------------------
; Additional function declarations
;
; GetWindowsVersion
 ;
 ; Based on Yazno's function, http://yazno.tripod.com/powerpimpit/
 ; Updated by Joost Verburg
 ;
 ; Returns on top of stack
 ;
 ; Windows Version (95, 98, ME, NT x.x, 2000, XP, 2003)
 ; or
 ; '' (Unknown Windows Version)
 ;
 ; Usage:
 ;   Call GetWindowsVersion
 ;   Pop $R0
 ;   ; at this point $R0 is "NT 4.0" or whatnot
 
 Function GetWindowsVersion
 
   Push $R0
   Push $R1
 
   ReadRegStr $R0 HKLM \
   "SOFTWARE\Microsoft\Windows NT\CurrentVersion" CurrentVersion

   IfErrors 0 lbl_winnt
   
   ; we are not NT
   ReadRegStr $R0 HKLM \
   "SOFTWARE\Microsoft\Windows\CurrentVersion" VersionNumber
 
   StrCpy $R1 $R0 1
   StrCmp $R1 '4' 0 lbl_error
 
   StrCpy $R1 $R0 3
 
   StrCmp $R1 '4.0' lbl_win32_95
   StrCmp $R1 '4.9' lbl_win32_ME lbl_win32_98
 
   lbl_win32_95:
     StrCpy $R0 '95'
   Goto lbl_done
 
   lbl_win32_98:
     StrCpy $R0 '98'
   Goto lbl_done
 
   lbl_win32_ME:
     StrCpy $R0 'ME'
   Goto lbl_done
 
   lbl_winnt:
 
   StrCpy $R1 $R0 1
 
   StrCmp $R1 '3' lbl_winnt_x
   StrCmp $R1 '4' lbl_winnt_x
 
   StrCpy $R1 $R0 3
 
   StrCmp $R1 '5.0' lbl_winnt_2000
   StrCmp $R1 '5.1' lbl_winnt_XP
   StrCmp $R1 '5.2' lbl_winnt_2003 lbl_error
 
   lbl_winnt_x:
     StrCpy $R0 "NT $R0" 6
   Goto lbl_done
 
   lbl_winnt_2000:
     Strcpy $R0 '2000'
   Goto lbl_done
 
   lbl_winnt_XP:
     Strcpy $R0 'XP'
   Goto lbl_done
 
   lbl_winnt_2003:
     Strcpy $R0 '2003'
   Goto lbl_done
 
   lbl_error:
     Strcpy $R0 ''
   lbl_done:
 
   Pop $R1
   Exch $R0
 
 FunctionEnd
;
; Additional function declarations
;--------------------------------------

!define PROGNAME    "Flexemu"
!define PROGVERSION "2.15"

SetDateSave on
SetDatablockOptimize on
BGGradient 000000 008000 00FF00
CRCCheck on
XPStyle on

LicenseBkColor /windows
LicenseData ".\Flexemu\Copying.txt"

VIAddVersionKey ProductName     "$PROGNAME"
VIAddVersionKey LegalCopyright  "(C) 1997-2004 W. Schwotzer"
VIAddVersionKey Comment         "an MC6809 emulator running FLEX"
VIAddVersionKey ProductVersion  "$PROGVERSION"
VIAddVersionKey FileDescription "an MC6809 emulator running FLEX"
VIAddVersionKey FileVersion     "${PROGVERSION}.0.0"
VIProductVersion "${PROGVERSION}.0.0"

; The name of the installer
Name "${PROGNAME} V${PROGVERSION}"

; The file to write
OutFile "${PROGNAME}-${PROGVERSION}.exe"

; The default installation directory
InstallDir $PROGRAMFILES\${PROGNAME}

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Gnu\${PROGNAME}" "InstDirectory"

;--------------------------------

; Pages

Page license
Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "${PROGNAME} (required)"
  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Check for unsupported Windows versions
  Call GetWindowsVersion
  Pop $R0
  StrCmp $R0 "95" WindowsVersionNotOk WindowsVersionOk
WindowsVersionNotOk:
    Abort "Windows 95 is not supported. \
                   Installation will be aborted"
WindowsVersionOk:

  ; Add files to be extracted to the current $OUTDIR path
  File /a ".\Flexemu\*.*"

  ; Write the installation path into the registry
  ReadRegStr $R1 HKLM SOFTWARE\Gnu\${PROGNAME} "InstDirectory"
  ; Check for a previous Installation
  StrCmp $R1 "" WriteRegKeys DontWriteRegKeys
WriteRegKeys:
  WriteRegStr HKLM SOFTWARE\Gnu\${PROGNAME} "InstDirectory"  "$INSTDIR"
  WriteRegStr HKLM SOFTWARE\Gnu\${PROGNAME} "DocDirectory"   "$INSTDIR\Documentation"
  WriteRegStr HKLM SOFTWARE\Gnu\${PROGNAME} "DiskDirectory"  "$INSTDIR\Data"
  WriteRegStr HKLM SOFTWARE\Gnu\${PROGNAME} "Version"        "${PROGVERSION}"
  WriteRegDWORD HKLM SOFTWARE\Gnu\${PROGNAME} "ScreenHeightFactor" 1
  WriteRegDWORD HKLM SOFTWARE\Gnu\${PROGNAME} "ScreenWidthFactor"  1
DontWriteRegKeys:
  WriteRegStr HKLM SOFTWARE\Gnu\FLEXplorer "BootSectorFile"  "$INSTDIR\boot"

  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGNAME}" "DisplayName" "${PROGNAME} V${PROGVERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGNAME}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGNAME}" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGNAME}" "NoRepair" 1
  WriteUninstaller "uninstall.exe"  
SectionEnd

Section "Monitor Programs (required)"
    SectionIn RO
    SetOutPath $INSTDIR\Data
    File /a ".\Flexemu\Data\*.hex"
    File /a ".\Flexemu\Data\*.s19"
SectionEnd

Section "Disk Files (recommended)"
    SetOutPath $INSTDIR\Data
    File /a ".\Flexemu\Data\*.dsk"
SectionEnd

Section "HTML Documentation (optional)"
    SetOutPath $INSTDIR\Documentation
    File /a ".\Flexemu\Documentation\*.htm"
SectionEnd   

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

; Install shortcuts for all users
; SetShellVarContext all
  SetOutPath $INSTDIR
  CreateDirectory "$SMPROGRAMS\${PROGNAME}"
  CreateShortCut "$SMPROGRAMS\${PROGNAME}\${PROGNAME}.lnk" "$INSTDIR\${PROGNAME}.exe" "" "$INSTDIR\${PROGNAME}.exe" 0 "" "" "A MC6809 Emulator running FLEX"
  CreateShortCut "$SMPROGRAMS\${PROGNAME}\FSetup.lnk" "$INSTDIR\FSetup.exe" "" "$INSTDIR\FSetup.exe" 0 "" "" "Setup for flexemu"
  CreateShortCut "$SMPROGRAMS\${PROGNAME}\Uninstall ${PROGNAME}.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0 "" "" "Uninstall Flexemu Installation"
  SetOutPath "$INSTDIR\Data"
  CreateShortCut "$SMPROGRAMS\${PROGNAME}\FLEXplorer.lnk" "$INSTDIR\FLEXplorer.exe" "" "$INSTDIR\FLEXplorer.exe" 0 "" "" "Explorer for FLEX container files"
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGNAME}"
  DeleteRegKey HKLM SOFTWARE\Gnu\${PROGNAME}

  ; Remove files and uninstaller
  Delete $INSTDIR\Documentation\*.*
  Delete $INSTDIR\Data\*.*
  Delete $INSTDIR\*.*

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\${PROGNAME}\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\${PROGNAME}"
  RMDir "$INSTDIR\Documentation"
  RMDir "$INSTDIR\Data"
  RMDir "$INSTDIR"

SectionEnd

