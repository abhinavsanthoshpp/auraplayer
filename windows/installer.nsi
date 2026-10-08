; Orion Player Windows NSIS Installer Script
; Copyright (C) 2026 Abhinav Santhosh (GitHub: @abhinavsanthoshpp)
; All Rights Reserved.

!define PRODUCT_NAME "Orion Player"
!define PRODUCT_VERSION "1.0.0"
!define PRODUCT_PUBLISHER "Abhinav Santhosh"
!define PRODUCT_WEB_SITE "https://abhinavsanthoshpp.github.io/orionplayer"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\orionplayer.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"

!include "MUI2.nsh"

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "OrionPlayer-Setup-v1.0.0.exe"
InstallDir "$PROGRAMFILES64\Orion Player"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show
RequestExecutionLevel admin

!define MUI_ABORTWARNING
!define MUI_ICON "..\resources\icons\orionplayer.ico"
!define MUI_UNICON "..\resources\icons\orionplayer.ico"

; Installer Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN "$INSTDIR\orionplayer.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Launch Orion Player now"
!insertmacro MUI_PAGE_FINISH

; Uninstaller Pages
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File /r "dist\*.*"

  ; Shortcuts
  CreateDirectory "$SMPROGRAMS\Orion Player"
  CreateShortcut "$SMPROGRAMS\Orion Player\Orion Player.lnk" "$INSTDIR\orionplayer.exe" "" "$INSTDIR\orionplayer.exe" 0
  CreateShortcut "$SMPROGRAMS\Orion Player\Uninstall.lnk" "$INSTDIR\uninstall.exe"
  CreateShortcut "$DESKTOP\Orion Player.lnk" "$INSTDIR\orionplayer.exe" "" "$INSTDIR\orionplayer.exe" 0
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninstall.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\orionplayer.exe"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\orionplayer.exe"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd

Section Uninstall
  Delete "$DESKTOP\Orion Player.lnk"
  Delete "$SMPROGRAMS\Orion Player\Orion Player.lnk"
  Delete "$SMPROGRAMS\Orion Player\Uninstall.lnk"
  RMDir "$SMPROGRAMS\Orion Player"

  RMDir /r "$INSTDIR"

  DeleteRegKey HKLM "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
SectionEnd
