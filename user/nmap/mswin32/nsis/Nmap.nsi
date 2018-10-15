;Nmap Installer
;Started by Bo Jiang @ 08/26/2005 06:07PM

;--------------------------------
;Include Modern UI

  !include "MUI.nsh"
  !include "AddToPath.nsh"

;--------------------------------
;General

  ;Name and file
  Name "Nmap"
  OutFile "NmapInstaller.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\Nmap"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\Nmap" ""

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

;  !insertmacro MUI_PAGE_LICENSE "${NSISDIR}\Docs\Modern UI\License.txt"
  !insertmacro MUI_PAGE_LICENSE "COPYING"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "Nmap Core Files" SecCore

  SetOutPath "$INSTDIR"
  RMDir /r $PROGRAMFILES\Nmap
  
  SetOverwrite on
  File CHANGELOG
  File COPYING
  File nmap-mac-prefixes
  File nmap-os-fingerprints
  File nmap-os-db
  File nmap-protocols
  File nmap-rpc
  File nmap-service-probes
  File nmap-services
  File nmap.exe
  File nmap.xsl
  File nmap_performance.reg
  File README-WIN32
  
  ;Store installation folder
  WriteRegStr HKCU "Software\Nmap" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

Section "Register Nmap Path" SecRegisterPath
  PUSH $INSTDIR
  Call AddToPath
SectionEnd


Section "WinPcap 3.1" SecWinPcap
  File winpcap-nmap-3.1.B.exe
  Exec '"$INSTDIR\winpcap-nmap-3.1.B.exe"'
  Delete "$INSTDIR\winpcap-nmap-3.1.B.exe"
SectionEnd

Section "Improve Performance" SecPerfRegistryMods
  File nmap_performance.reg
  Exec 'regedt32 /S "$INSTDIR\nmap_performance.reg"'
SectionEnd

;--------------------------------
;Descriptions

  ;Component strings
  LangString DESC_SecCore ${LANG_ENGLISH} "Installs Nmap executables and script files"
  LangString DESC_SecRegisterPath ${LANG_ENGLISH} "Registers Nmap path to System path so you can execute it from any directory"
  LangString DESC_SecWinPcap ${LANG_ENGLISH} "Installs WinPcap 3.1 (required for most Nmap scans unless it is already installed)"
  LangString DESC_SecPerfRegistryMods ${LANG_ENGLISH} "Modifies Windows registry values to improve TCP connect scan performance.  Recommended."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecCore} $(DESC_SecCore)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecWinPcap} $(DESC_SecWinPcap)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecRegisterPath} $(DESC_SecRegisterPath)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPerfRegistryMods} $(DESC_SecPerfRegistryMods)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END
;--------------------------------
;Uninstaller Section

Section "Uninstall"

  SetDetailsPrint textonly
  DetailPrint "Uninstalling Files..."
  SetDetailsPrint listonly

  IfFileExists $INSTDIR\nmap.exe nmap_installed
    MessageBox MB_YESNO "It does not appear that Nmap is installed in the directory '$INSTDIR'.$\r$\nContinue anyway (not recommended)?" IDYES nmap_installed
    Abort "Uninstall aborted by user"
  
  nmap_installed:
  Delete "$INSTDIR\CHANGELOG"
  Delete "$INSTDIR\COPYING"
  Delete "$INSTDIR\nmap-mac-prefixes"
  Delete "$INSTDIR\nmap-os-fingerprints"
  Delete "$INSTDIR\nmap-os-db"
  Delete "$INSTDIR\nmap-protocols"
  Delete "$INSTDIR\nmap-rpc"
  Delete "$INSTDIR\nmap-service-probes"
  Delete "$INSTDIR\nmap-services"
  Delete "$INSTDIR\nmap.exe"
  Delete "$INSTDIR\nmap.xsl"
  Delete "$INSTDIR\nmap_performance.reg"
  Delete "$INSTDIR\README-WIN32"

  Delete "$INSTDIR\Uninstall.exe"

  RMDir /r $PROGRAMFILES\Nmap

  SetDetailsPrint textonly
  DetailPrint "Deleting Registry Keys..."
  SetDetailsPrint listonly
  DeleteRegKey /ifempty HKCU "Software\Nmap"

  SetDetailsPrint textonly
  DetailPrint "Unregistering Nmap Path..."
  Push $INSTDIR
  Call un.RemoveFromPath

  SetDetailsPrint both
SectionEnd

