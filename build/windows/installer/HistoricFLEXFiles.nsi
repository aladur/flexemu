;--------------------------------
; HistoricFLEXFiles.nsi
;--------------------------------

!include LogicLib.nsh
!include x64.nsh
!include FileFunc.nsh
!include SplitFirstStrPart.nsh
!include "MUI2.nsh"
  
!define APPNAME    "HistoricFLEXFiles"
!define APPVERSION "1.00"
; Refreshing Windows Defines
!define SHCNE_ASSOCCHANGED 0x8000000
!define SHCNF_IDLIST 0
!define ERROR_ALREADY_EXISTS 183
!define ARP "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
!define BASEDIR    "..\..\fug_dsk_files"

CRCCheck on
SetDateSave on
SetDatablockOptimize on
BGGradient 000000 008000 00FF00
SetCompressor /SOLID lzma
LicenseBkColor /windows

; file info
VIAddVersionKey ProductName     "${APPNAME}"
VIAddVersionKey LegalCopyright  "(C) 2019 W. Schwotzer"
VIAddVersionKey Comment         "FLEX User Group Historic FLEX Files"
VIAddVersionKey ProductVersion  "${APPVERSION}"
VIAddVersionKey FileDescription "FLEX User Group Historic FLEX Files"
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
  ; Set the installation folder to the documents directory.
  ; It depends on the SetShellVarContext if the context of this variable
  ; is All Users or the current user.
  StrCpy $INSTDIR $DOCUMENTS\${APPNAME}

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
  SetRegView Default

FunctionEnd

;-------------------------------
; Pages
!define MUI_COMPONENTSPAGE_SMALLDESC  
!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
  
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

;--------------------------------
; The install sections
Section "FLEX DSK files" FlexDskFiles

  SectionIn RO
  SetOutPath $INSTDIR
  ; Install all FLEX DSK files
  SetOutPath $INSTDIR\Bjarne_Baeckstroem
  File /a "${BASEDIR}\Bjarne_Baeckstroem\*.dsk"
  SetOutPath $INSTDIR\Dan_Farnsworth
  File /a "${BASEDIR}\Dan_Farnsworth\*.dsk"
  SetOutPath $INSTDIR\Frank_Hogg_Labs
  File /a "${BASEDIR}\Frank_Hogg_Labs\*.dsk"
  SetOutPath $INSTDIR\FLEX2
  File /a "${BASEDIR}\FLEX2\*.dsk"
  SetOutPath $INSTDIR\Joe_Lang
  File /a "${BASEDIR}\Joe_Lang\*.dsk"
  SetOutPath $INSTDIR\Leo_Taylor
  File /a "${BASEDIR}\Leo_Taylor\*.dsk"
  SetOutPath $INSTDIR\UK_68_Micro_Group
  File /a "${BASEDIR}\UK_68_Micro_Group\*.dsk"
  SetOutPath $INSTDIR\Windrush
  File /a "${BASEDIR}\Windrush\*.dsk"

SectionEnd

Section "Documentation files" DocumentationFiles

  SectionIn RO
  SetOutPath $INSTDIR
  ; Install all *.html and image files
  File /a "${BASEDIR}\*.html"
  SetOutPath $INSTDIR\images
  File /a "${BASEDIR}\images\*.*"

SectionEnd

Section "Desktop Icons" DesktopIcons

  SetOutPath $INSTDIR ; for working directory
  CreateShortCut "$DESKTOP\Historic FLEX Files.lnk" "$INSTDIR\index.html" "" "$INSTDIR\images\fug.ico" 0 "" "" "Historic FLEX Files from FLEX User Group"

SectionEnd

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${FlexDskFiles} "All FLEX DSK files."
  !insertmacro MUI_DESCRIPTION_TEXT ${DocumentationFiles} "HTML Documencd tation files."
  !insertmacro MUI_DESCRIPTION_TEXT ${DesktopIcons} "Add a Desktop Icon."
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
  StrCpy $DisplayName "Historic FLEX Files ${APPVERSION}"
  
  WriteRegStr   HKLM "${ARP}" "DisplayName"     "$DisplayName"
  WriteRegStr   HKLM "${ARP}" "DisplayVersion"  "${APPVERSION}"
  WriteRegStr   HKLM "${ARP}" "DisplayIcon"     "$INSTDIR\images\fug.ico"
  WriteRegStr   HKLM "${ARP}" "HelpLink"        "http://flexemu.neocities.org"
  WriteRegStr   HKLM "${ARP}" "InstallLocation" "$INSTDIR"
  WriteRegStr   HKLM "${ARP}" "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegStr   HKLM "${ARP}" "Publisher"       "Wolfgang Schwotzer"
  WriteRegDWORD HKLM "${ARP}" "EstimatedSize"   "$EstimatedSize"
  WriteRegDWORD HKLM "${ARP}" "VersionMajor"    "$VersionMajor"
  WriteRegDWORD HKLM "${ARP}" "VersionMinor"    "$VersionMinor"
  WriteRegDWORD HKLM "${ARP}" "NoModify"        1
  WriteRegDWORD HKLM "${ARP}" "NoRepair"        1
  WriteUninstaller "uninstall.exe"  

  System::Call 'Shell32::SHChangeNotify(i ${SHCNE_ASSOCCHANGED}, i ${SHCNF_IDLIST}, p0, p0)'

SectionEnd

;--------------------------------
; Uninstaller

Function un.onInit

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
	Abort ; Immediately Quit uninstallation
uninit.done:
  StrCpy $INSTDIR $R0

FunctionEnd

Section "Uninstall" Uninstall

  ; Remove files and uninstaller
  Delete $INSTDIR\Windrush\*.*
  Delete $INSTDIR\UK_68_Micro_Group\*.*
  Delete $INSTDIR\Leo_Taylor\*.*
  Delete $INSTDIR\Joe_Lang\*.*
  Delete $INSTDIR\FLEX2\*.*
  Delete $INSTDIR\Frank_Hogg_Labs\*.*
  Delete $INSTDIR\Dan_Farnsworth\*.*
  Delete $INSTDIR\Bjarne_Baeckstroem\*.*
  Delete $INSTDIR\images\*.*
  Delete $INSTDIR\*.*

  ; Remove directories used
  RMDir "$INSTDIR\Windrush"
  RMDir "$INSTDIR\UK_68_Micro_Group"
  RMDir "$INSTDIR\Leo_Taylor"
  RMDir "$INSTDIR\Joe_Lang"
  RMDir "$INSTDIR\FLEX2"
  RMDir "$INSTDIR\Frank_Hogg_Labs"
  RMDir "$INSTDIR\Dan_Farnsworth"
  RMDir "$INSTDIR\Bjarne_Baeckstroem"
  RMDir "$INSTDIR\images"
  RMDir "$INSTDIR"

  ; Remove shortcuts, if they have been installed.
  Delete "$DESKTOP\Historic FLEX Files.lnk"

  ; Remove registry keys
  DeleteRegKey HKLM "${ARP}"
  
  System::Call 'Shell32::SHChangeNotify(i ${SHCNE_ASSOCCHANGED}, i ${SHCNF_IDLIST}, p0, p0)'

SectionEnd

