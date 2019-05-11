;--------------------------------
; flexemu.nsi
;--------------------------------

!include LogicLib.nsh
!include x64.nsh
!include StrStr.nsh
!include GetParameters.nsh
!include GetParameterValue.nsh
!include FileFunc.nsh
!include SplitFirstStrPart.nsh
!include "MUI2.nsh"
  
!define APPNAME    "Flexemu"
!define APPVERSION "2.24"
; Refreshing Windows Defines
!define SHCNE_ASSOCCHANGED 0x8000000
!define SHCNF_IDLIST 0
!define ERROR_ALREADY_EXISTS 183
!define ARP "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
!define BASEDIR    "..\..\.."

CRCCheck on
SetDateSave on
SetDatablockOptimize on
BGGradient 000000 008000 00FF00
SetCompressor /SOLID lzma
LicenseBkColor /windows

; file info
VIAddVersionKey ProductName     "${APPNAME}"
VIAddVersionKey LegalCopyright  "(C) 1997-2019 W. Schwotzer"
VIAddVersionKey Comment         "an MC6809 emulator running FLEX"
VIAddVersionKey ProductVersion  "${APPVERSION}"
VIAddVersionKey FileDescription "an MC6809 emulator running FLEX"
VIAddVersionKey FileVersion     "${APPVERSION}.0.0"
VIProductVersion "${APPVERSION}.0.0"

; The name of the installer
Name "${APPNAME} ${APPVERSION}"

; The installation file to create
OutFile "..\${APPNAME}-Setup-${APPVERSION}.exe"

; Have Windows Vista, 2008, 7, etc. trust us to not be a "legacy" installer
;RequestExecutionLevel admin

!macro VerifyUserIsAdmin
  UserInfo::GetAccountType
  Pop $0
  ${If} $0 != "admin" ;Require admin rights on NT4+
    MessageBox MB_ICONSTOP "Administrator rights required!"
    SetErrorLevel 740 ;ERROR_ELEVATION_REQUIRED
    Quit
  ${EndIf}
!macroend

Function .onInit

  ; Check if installer is already running. If open a user dialog and abort installation.
  System::Call 'kernel32::CreateMutex(i 0, i 0, t "${APPNAME}_Mutex") ?e'
  Pop $R0
  StrCmp $R0 ${ERROR_ALREADY_EXISTS} 0 init.verifyadmin
  MessageBox MB_OK "${APPNAME} installer is already running."
  Abort
init.verifyadmin:
  ; Depending on admin status install application for all or current user.
  UserInfo::GetAccountType
  Pop $0
  ${If} $0 == "admin"
    SetShellVarContext all
  ${Else}
    SetShellVarContext current
  ${EndIf}
  ; Estimate default installation directory for 32-/64-bit installation
  ; On 64-bit OS a command line parameter can be used to force a
  ; 32-bit installation: 
  ; Flexemu-Setup-x.yy.exe /ARCH=x86
  ; Result:
  ;   32-bit OS:
  ;      Always: $ARCH == "x86"
  ;   64-bit OS:
  ;      If command line parameter is set: $ARCH == "x86"
  ;      If no command line parameter set: $ARCH == "x64"
  Var /GLOBAL Arch
  StrCpy $Arch "x86"
  ${If} ${RunningX64}
    Push "ARCH"  ; push the parameter name onto the stack
    Push "x64"   ; push the default value for this parameter
    Call GetParameterValue
    Pop $2
	StrCpy $Arch "$2"
    ${If} $Arch == "x86"
      StrCpy $INSTDIR $PROGRAMFILES32\${APPNAME}
    ${Else}
      StrCpy $INSTDIR $PROGRAMFILES64\${APPNAME}
    ${EndIf}
  ${Else}
    StrCpy $INSTDIR $PROGRAMFILES\${APPNAME}
  ${EndIf}

  ; Estimate by Registry access if Application is already installed
  ; If so open a user dialog and evtl. uninstall it or abort.
  ${If} ${RunningX64}
    SetRegView 64
    ReadRegStr $R0 HKLM "${ARP}" "UninstallString"
    IfFileExists "$R0" init.askuninst init.try32bit
init.try32bit:  
    SetRegView 32
  ${Else}
    SetRegView Default
  ${EndIf}
  ReadRegStr $R0 HKLM "${ARP}" "UninstallString"
  IfFileExists "$R0" init.askuninst init.done
init.askuninst:
  MessageBox MB_YESNO|MB_ICONQUESTION "${APPNAME} is already installed. Uninstall the existing version?" IDYES init.uninstall IDNO init.quit
init.quit:
  Quit
init.uninstall:
  ReadRegStr $R1 HKLM "${ARP}" "InstallLocation"
  ClearErrors
  ExecWait '"$R0" /S _?=$R1'
  IfErrors +1 init.done  
  MessageBox MB_OK "Error during uninstallation"
  Quit  
init.done:
; Set registry mode according to architecture to be installed
  ${If} $Arch == "x64"
    SetRegView 64
  ${Else}
    SetRegView Default
  ${EndIf}

FunctionEnd

;-------------------------------
; Pages
!define MUI_COMPONENTSPAGE_SMALLDESC  
!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_LICENSE "${BASEDIR}\COPYING"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
  
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

;--------------------------------
; The install sections
Section "${APPNAME}" BinaryFiles

  SectionIn RO  
  SetOutPath $INSTDIR ; Set output path to the installation directory.
  ; Add files to be extracted to the current $OUTDIR path
${If} $Arch == "x64"
  File /a "${BASEDIR}\bin\x64\Release\flexemu.exe"
  File /a "${BASEDIR}\bin\x64\Release\flexplorer.exe"
  File /a "${BASEDIR}\bin\x64\Release\fsetup.exe"
  File /a "${BASEDIR}\bin\x64\Release\mdcrtool.exe"
  File /a "${BASEDIR}\bin\x64\Release\wxbase312u_vc_x64_custom.dll"
  File /a "${BASEDIR}\bin\x64\Release\wxmsw312u_core_vc_x64_custom.dll"
${Else}
  File /a "${BASEDIR}\bin\Win32\Release\flexemu.exe"
  File /a "${BASEDIR}\bin\Win32\Release\flexplorer.exe"
  File /a "${BASEDIR}\bin\Win32\Release\fsetup.exe"
  File /a "${BASEDIR}\bin\Win32\Release\mdcrtool.exe"
  File /a "${BASEDIR}\bin\Win32\Release\wxbase312u_vc_custom.dll"
  File /a "${BASEDIR}\bin\Win32\Release\wxmsw312u_core_vc_custom.dll"
${EndIf}
  File /a "${BASEDIR}\src\boot"
  File /a "${BASEDIR}\src\flexemu.conf"
  File /a /oname=Changes.txt "${BASEDIR}\ChangeLog"
  File /a /oname=Copying.txt "${BASEDIR}\COPYING"
  File /a /oname=Readme.txt "${BASEDIR}\README"
SectionEnd

Section "Monitor programs and disk files" MonitorDiskFiles

  SectionIn RO
  SetOutPath $INSTDIR\Data
  ; Install Monitor programs
  File /a "${BASEDIR}\monitor\coltab.hex"
  File /a "${BASEDIR}\monitor\mon24.s19"
  File /a "${BASEDIR}\monitor\mon53.s19"
  File /a "${BASEDIR}\monitor\mon54.s19"
  File /a "${BASEDIR}\monitor\monu54-6.s19"
  File /a "${BASEDIR}\monitor\neumon54.hex"
  ; Install cassette/disk files
  File /a "${BASEDIR}\disks\btx.dsk"
  File /a "${BASEDIR}\disks\colors.dsk"
  File /a "${BASEDIR}\disks\games.dsk"
  File /a "${BASEDIR}\disks\laycad.dsk"
  File /a "${BASEDIR}\disks\layout.dsk"
  File /a "${BASEDIR}\disks\pictures.dsk"
  File /a "${BASEDIR}\disks\source.dsk"
  File /a "${BASEDIR}\disks\system.dsk"
  File /a "${BASEDIR}\disks\test.dsk"
  File /a "${BASEDIR}\disks\system54.dsk"
  File /a "${BASEDIR}\disks\system.mdcr"

SectionEnd

Section "Documentation" Documentation

  SectionIn RO
  SetOutPath $INSTDIR\Documentation
  File /a "${BASEDIR}\doc\flexdos.htm"
  File /a "${BASEDIR}\doc\flexemu.htm"
  File /a "${BASEDIR}\doc\flexerr.htm"
  File /a "${BASEDIR}\doc\flexfcb.htm"
  File /a "${BASEDIR}\doc\flexfms.htm"
  File /a "${BASEDIR}\doc\flexfs.htm"
  File /a "${BASEDIR}\doc\flexmem.htm"
  File /a "${BASEDIR}\doc\flexuser.htm"
  File /a "${BASEDIR}\doc\flexutil.htm"
  File /a "${BASEDIR}\doc\neumon54.htm"
  File /a "${BASEDIR}\doc\mon24.htm"
  File /a "${BASEDIR}\doc\e2ramext.htm"
  File /a "${BASEDIR}\doc\mc6809.htm"
  File /a "${BASEDIR}\doc\6809diag.pdf"
  File /a "${BASEDIR}\doc\6809fadg.pdf"
  File /a "${BASEDIR}\doc\asmb.pdf"
  File /a "${BASEDIR}\doc\ba2bqs.pdf"
  File /a "${BASEDIR}\doc\basic_um.pdf"
  File /a "${BASEDIR}\doc\basprec.pdf"
  File /a "${BASEDIR}\doc\crasmb.pdf"
  File /a "${BASEDIR}\doc\debug.pdf"
  File /a "${BASEDIR}\doc\dynamite.pdf"
  File /a "${BASEDIR}\doc\f77.pdf"
  File /a "${BASEDIR}\doc\flex2um.pdf"
  File /a "${BASEDIR}\doc\flexapg.pdf"
  File /a "${BASEDIR}\doc\linkload.pdf"
  File /a "${BASEDIR}\doc\relasmb.pdf"
  File /a "${BASEDIR}\doc\swflexum.pdf"
  File /a "${BASEDIR}\doc\tedit.pdf"
  File /a "${BASEDIR}\doc\util_man.pdf"
  File /a "${BASEDIR}\doc\6x09_Instruction_Sets.pdf"
  SetOutPath $INSTDIR\Documentation\images
  File /a "${BASEDIR}\doc\images\e2v5m.png"
  File /a "${BASEDIR}\doc\images\e2v7m.png"
  File /a "${BASEDIR}\doc\images\2x96k.png"
  File /a "${BASEDIR}\doc\images\2x384k.png"
  File /a "${BASEDIR}\doc\images\2x384flx.png"
  File /a "${BASEDIR}\doc\images\e2scn.png"

SectionEnd   

Section "Start Menu Shortcuts" StartMenu

  SectionIn RO
  SetOutPath $INSTDIR ; for working directory
  CreateDirectory "$SMPROGRAMS\${APPNAME}"
  CreateShortCut "$SMPROGRAMS\${APPNAME}\${APPNAME}.lnk" "$INSTDIR\${APPNAME}.exe" "" "$INSTDIR\${APPNAME}.exe" 0 "" "" "A MC6809 Emulator running FLEX"
  CreateShortCut "$SMPROGRAMS\${APPNAME}\FSetup.lnk" "$INSTDIR\FSetup.exe" "" "$INSTDIR\FSetup.exe" 0 "" "" "Setup for flexemu"
  CreateShortCut "$SMPROGRAMS\${APPNAME}\FLEXplorer.lnk" "$INSTDIR\FLEXplorer.exe" "" "$INSTDIR\FLEXplorer.exe" 0 "" "" "Explorer for FLEX container files"
  CreateShortCut "$SMPROGRAMS\${APPNAME}\Uninstall ${APPNAME}.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0 "" "" "Uninstall Flexemu Installation"  

SectionEnd

Section "Desktop Icons" DesktopIcons

  SetOutPath $INSTDIR ; for working directory
  CreateShortCut "$DESKTOP\${APPNAME}.lnk" "$INSTDIR\${APPNAME}.exe" "" "$INSTDIR\${APPNAME}.exe" 0 "" "" "A MC6809 Emulator running FLEX"
  CreateShortCut "$DESKTOP\FSetup.lnk" "$INSTDIR\FSetup.exe" "" "$INSTDIR\FSetup.exe" 0 "" "" "Setup for flexemu"
  CreateShortCut "$DESKTOP\FLEXplorer.lnk" "$INSTDIR\FLEXplorer.exe" "" "$INSTDIR\FLEXplorer.exe" 0 "" "" "Explorer for FLEX container files"

SectionEnd

Section "Microsoft Visual C++ Redistributables" VC_Redist

  SetOutPath $TEMP
${If} $Arch == "x64"
  ReadRegDword $R1 HKLM "SOFTWARE\Wow6432Node\Microsoft\VisualStudio\14.0\VC\Runtimes\x64" "Installed"
  ; Only istall VC_Redist if it is not already installed.
  ; Do not check for version, only for VS2015 (14.0).
  ${If} $R1 != "1"
    File ..\vc_redist.x64.exe  
    ExecWait '"$TEMP\vc_redist.x64.exe" /install /passive /norestart' 
    Delete $TEMP\vc_redist.x64.exe     
  ${EndIf}
${Else}
  ReadRegDword $R1 HKLM "SOFTWARE\Wow6432Node\Microsoft\VisualStudio\14.0\VC\Runtimes\x86" "Installed"
  ${If} $R1 != "1"
    File ..\vc_redist.x86.exe  
    ExecWait '"$TEMP\vc_redist.x86.exe" /install /passive /norestart' 
    Delete $TEMP\vc_redist.x86.exe     
  ${EndIf}
${EndIf}

SectionEnd   

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${BinaryFiles} "${APPNAME} binary files."
  !insertmacro MUI_DESCRIPTION_TEXT ${MonitorDiskFiles} "Monitor programs to boot MC6809. Disk files to boot FLEX operating system from emulated disk drive. Containing some sample programs."
  !insertmacro MUI_DESCRIPTION_TEXT ${Documentation} "Flexemu and FLEX operating system documentation."
  !insertmacro MUI_DESCRIPTION_TEXT ${StartMenu} "Create start menu."
  !insertmacro MUI_DESCRIPTION_TEXT ${DesktopIcons} "Create desktop icons."
  !insertmacro MUI_DESCRIPTION_TEXT ${VC_Redist} "Install Microsoft Visual C++ Redistributable package 2015 (vc_redist). If unsure install it."
  
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Section "-Registry update"

  Var /GLOBAL EstimatedSize
  Var /GLOBAL VersionMajor
  Var /GLOBAL VersionMinor
  Var /GLOBAL DisplayName

  ; Write the uninstall keys for Windows
  ; Calculate estimate size of installation directory
  ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
  IntFmt $0 "0x%08X" $0
  StrCpy $EstimatedSize "$0"
  ; Extract major and minor version
  Push "."
  Push "${APPVERSION}"
  Call SplitFirstStrPart
  Pop $VersionMajor
  Pop $VersionMinor
  StrCpy $DisplayName "${APPNAME} ${APPVERSION}"
  ${If} $Arch == "x86"
    ${If} ${RunningX64}
      StrCpy $DisplayName "$DisplayName (32-Bit)"
    ${EndIf}
  ${EndIf}
  
  WriteRegStr   HKLM "${ARP}" "DisplayName"     "$DisplayName"
  WriteRegStr   HKLM "${ARP}" "DisplayVersion"  "${APPVERSION}"
  WriteRegStr   HKLM "${ARP}" "DisplayIcon"     "$INSTDIR\flexemu.exe"
  WriteRegStr   HKLM "${ARP}" "HelpLink"        "https://aladur.neocities.org/flexemu/"
  WriteRegStr   HKLM "${ARP}" "InstallLocation" "$INSTDIR"
  WriteRegStr   HKLM "${ARP}" "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegStr   HKLM "${ARP}" "Publisher"       "Wolfgang Schwotzer"
  WriteRegDWORD HKLM "${ARP}" "EstimatedSize"   "$EstimatedSize"
  WriteRegDWORD HKLM "${ARP}" "VersionMajor"    "$VersionMajor"
  WriteRegDWORD HKLM "${ARP}" "VersionMinor"    "$VersionMinor"
  WriteRegDWORD HKLM "${ARP}" "NoModify"        1
  WriteRegDWORD HKLM "${ARP}" "NoRepair"        1
  WriteUninstaller "uninstall.exe"

  ; Add File associations supported by FLEXplorer
  WriteRegStr   HKCR "Applications\FLEXplorer.exe" "" ""
  WriteRegStr   HKCR "Applications\FLEXplorer.exe\SupportedTypes" ".dsk" ""
  WriteRegStr   HKCR "Applications\FLEXplorer.exe\SupportedTypes" ".flx" ""
  WriteRegStr   HKCR "Applications\FLEXplorer.exe\SupportedTypes" ".wta" ""
  WriteRegStr   HKCR "Applications\FLEXplorer.exe\shell\open\command" "" "$\"$INSTDIR\FLEXplorer.exe$\" $\"%1$\""

  ; Add Application paths
  WriteRegStr   HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\${APPNAME}.exe" "" "$INSTDIR\${APPNAME}.exe"
  WriteRegStr   HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\FLEXplorer.exe" "" "$INSTDIR\FLEXplorer.exe"
  WriteRegStr   HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\FSetup.exe" "" "$INSTDIR\FSetup.exe"

  System::Call 'Shell32::SHChangeNotify(i ${SHCNE_ASSOCCHANGED}, i ${SHCNF_IDLIST}, p0, p0)'

SectionEnd

;--------------------------------
; Uninstaller

Function un.onInit

  ; Check if any application installed by this installer is running. If so open a user dialog and retry or abort installation.
uninit.checkrun1:
  FindWindow $0 "${APPNAME}" ""
  StrCmp $0 0 uninit.checkrun2
    MessageBox MB_RETRYCANCEL|MB_ICONEXCLAMATION "${APPNAME} is running. Please close it first" /SD IDRETRY IDRETRY uninit.checkrun1 IDABORT uninit.abort
uninit.checkrun2:
  FindWindow $0 "" "${APPNAME} Options Dialog"
  StrCmp $0 0 uninit.checkrun3
    MessageBox MB_RETRYCANCEL|MB_ICONEXCLAMATION "FSetup is running. Please close it first" /SD IDRETRY IDRETRY uninit.checkrun1 IDABORT uninit.abort
uninit.checkrun3:
  FindWindow $0 "" "FLEXplorer"
  StrCmp $0 0 uninit.notrunning
    MessageBox MB_RETRYCANCEL|MB_ICONEXCLAMATION "FLEXplorer is running. Please close it first" /SD IDRETRY IDRETRY uninit.checkrun1 IDABORT uninit.abort
uninit.notrunning:
  ; Depending on admin status uninstall application for all or current user.
  UserInfo::GetAccountType
  Pop $0
  ${If} $0 == "admin"
    SetShellVarContext all
  ${Else}
    SetShellVarContext current
  ${EndIf}
  ; Estimate by Registry access if this is a 32- or 64-bit installation
  ${If} ${RunningX64}
    SetRegView 64
    ReadRegStr $R0 HKLM "${ARP}" "InstallLocation"
    IfErrors uninit.try32bit uninit.done
uninit.try32bit:  
    SetRegView 32
  ${Else}
    SetRegView Default
  ${EndIf}
  ReadRegStr $R0 HKLM "${ARP}" "InstallLocation"
  IfErrors uninit.dlgabort uninit.done
uninit.dlgabort:
    MessageBox MB_OK "Installation is inconsistent. Aborting Uninstallation."
uninit.abort:
	Abort ; Immediately Quit uninstallation
uninit.done:
  StrCpy $INSTDIR $R0

FunctionEnd

Section "Uninstall" Uninstall
  
  ; Remove registry keys
  DeleteRegKey HKLM "${ARP}"
  DeleteRegKey HKLM SOFTWARE\Gnu\${APPNAME}
  DeleteRegKey HKLM SOFTWARE\Gnu\FLEXplorer
  DeleteRegKey HKCR "Applications\FLEXplorer.exe"
  DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\${APPNAME}.exe"
  DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\FLEXplorer.exe"
  DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\FSetup.exe"

  ; Remove files and uninstaller
  Delete $INSTDIR\Documentation\*.*
  Delete $INSTDIR\Data\*.*
  Delete $INSTDIR\*.*

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\${APPNAME}\*.*"
  Delete "$DESKTOP\${APPNAME}.lnk"
  Delete "$DESKTOP\FSetup.lnk"
  Delete "$DESKTOP\FLEXplorer.lnk"
  

  ; Remove directories used
  RMDir "$SMPROGRAMS\${APPNAME}"
  RMDir "$INSTDIR\Documentation"
  RMDir "$INSTDIR\Data"
  RMDir "$INSTDIR"

  System::Call 'Shell32::SHChangeNotify(i ${SHCNE_ASSOCCHANGED}, i ${SHCNF_IDLIST}, p0, p0)'

SectionEnd

