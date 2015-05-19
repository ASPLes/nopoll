; Script generated by the HM NIS Edit Script Wizard.

!include "nopoll_product_version.nsh"

; HM NIS Edit Wizard helper definesdot
!define PRODUCT_NAME "noPoll Toolkit for win${PLATFORM_BITS} (${PLATFORM_BITS} bits)"
!define PRODUCT_PUBLISHER "ASPL"
!define PRODUCT_WEB_SITE "http://www.aspl.es/nopoll"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "doc\nopoll-logo-48x48.ico"
!define MUI_UNICON "doc\nopoll-logo-48x48.ico"
; !define MUI_WELCOMEFINISHPAGE_BITMAP "libnopoll\logo\nopoll-lateral-image.bmp"
; !define MUI_UNWELCOMEFINISHPAGE_BITMAP "libnopoll\logo\nopoll-lateral-image.bmp"

; Welcome page
!define MUI_WELLCOMEPAGE_TITLE "$(PRODUCT_NAME): Open Source Professional WebSocket toolkit $(PRODUCT_VERSION) installer"
!define MUI_WELCOMEPAGE_TITLE_3LINES
!insertmacro MUI_PAGE_WELCOME

!insertmacro MUI_PAGE_LICENSE "COPYING"

!insertmacro MUI_PAGE_COMPONENTS

; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR/README-NOPOLL-WIN32.txt"
!define MUI_FINISHPAGE_TITLE_3LINES
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!define MUI_WELCOMEPAGE_TITLE_3LINES
!insertmacro MUI_UNPAGE_WELCOME

!insertmacro MUI_UNPAGE_INSTFILES

!define MUI_FINISHPAGE_TITLE_3LINES
!insertmacro MUI_UNPAGE_FINISH

; Language files
!insertmacro MUI_LANGUAGE "English"

; MUI end ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "nopoll-installer-${PRODUCT_VERSION}-w${PLATFORM_BITS}.exe"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

Section "Core Binaries" SEC01

  SectionIn RO
  SetOutPath "$INSTDIR"
  File "README-NOPOLL-WIN32.txt"

  SetOutPath "$INSTDIR\bin"
  SetOverwrite ifnewer
  File "release\libnopoll0\libnopoll.dll"
  File "libgcc_s_dw2.1.dll"
SectionEnd

Section "Library with debug support enabled" SEC0112

  SetOutPath "$INSTDIR"
  SetOutPath "$INSTDIR\debug"
  SetOverwrite ifnewer

  File "debug\libnopoll0\libnopoll-debug.dll"
SectionEnd

Section "OpenSSL installation" SEC021
  SetOutPath "$INSTDIR\bin"
  SetOverwrite ifnewer

  File "test\ssleay32.dll"
  File "test\libeay32.dll"
SectionEnd

Section "Test examples" SEC08
  SetOutPath "$INSTDIR\bin"
  SetOverwrite ifnewer

  File "test\nopoll-regression-client.exe"
  File "test\nopoll-regression-listener.exe"
  File "test\test-private.key"
  File "test\test-certificate.crt"
SectionEnd

Section /o "Development Headers" SEC10
  SetOutPath "$INSTDIR\include"
  SetOverwrite ifnewer

  SetOutPath "$INSTDIR\include\openssl"
  File /r "c:\OpenSSL\include\openssl"

  SetOutPath "$INSTDIR\include\nopoll"
  File "src\nopoll.h"
  File "src\nopoll_config.h"
  File "src\nopoll_handlers.h"
  File "src\nopoll_decl.h"
  File "src\nopoll_private.h"
  File "src\nopoll_ctx.h"
  File "src\nopoll_conn.h"
  File "src\nopoll_conn_opts.h"
  File "src\nopoll_log.h"
  File "src\nopoll_listener.h"
  File "src\nopoll_loop.h"
  File "src\nopoll_io.h"
  File "src\nopoll_msg.h"
  File "src\nopoll_win32.h"
SectionEnd

Section /o "Developement libs" SEC11
  SetOutPath "$INSTDIR\lib"
  SetOverwrite ifnewer
  
  ; base library libnopoll.dll
  File "release\libnopoll0\libnopoll.dll.a"
  File "src\libnopoll.def"
  File "release\libnopoll0\libnopoll.exp"
  File "release\libnopoll0\libnopoll.lib"

  SetOutPath "$INSTDIR\lib\debug"
  SetOverwrite ifnewer

  ; (DEBUG) base library libnopoll.dll
  File "debug\libnopoll0\libnopoll-debug.dll.a"
  File "debug\libnopoll0\libnopoll-debug.exp"
  File "debug\libnopoll0\libnopoll-debug.lib"

SectionEnd


Section -AdditionalIcons
  ReadRegStr $R9 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders" "Common Programs"
  StrCmp $R9 "" 0 ContinueInstallation
  MessageBox MB_OK "No se pudo obtener el directorio de instalación general, saliendo..."
  Abort ; or anything else...
  ContinueInstallation:
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  WriteIniStr "$INSTDIR\ASPL.url" "InternetShortcut" "URL" "http://www.aspl.es"

  CreateDirectory "$R9\noPoll 1.0"
  CreateShortCut "$R9\noPoll 1.0\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$R9\noPoll 1.0\ASPL.lnk" "$INSTDIR\ASPL.url"
  CreateShortCut "$R9\noPoll 1.0\README.lnk" "$INSTDIR\README-NOPOLL-WIN32.txt"
  CreateShortCut "$R9\noPoll 1.0\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"

  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"

;  Delete "$INSTDIR\imagenLateral.bmp"
SectionEnd


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) uninstall process have finished properly."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to uninstall $(^Name)?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  ReadRegStr $R9 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders" "Common Programs"
  StrCmp $R9 "" 0 ContinueInstallation
 ContinueInstallation:

  Delete "$R9\noPoll 1.0\Website.lnk"
  Delete "$R9\noPoll 1.0\ASPL.lnk"
  Delete "$R9\noPoll 1.0\Uninstall.lnk"
  Delete "$R9\noPoll 1.0\README.lnk"

  RMDir "$R9\noPoll 1.0"
  RMDir /r "$INSTDIR"
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  SetAutoClose true
SectionEnd

LangString DESC_SEC01 ${LANG_ENGLISH} "Includes minimal core DLL binary installation for noPoll 1.0 (libnopoll.dll). No extension library included."

LangString DESC_SEC0112 ${LANG_ENGLISH} "Includes all noPoll 1.0 DLLs with log support enabled. The default release comes without log support enabled."

LangString DESC_SEC021 ${LANG_ENGLISH} "OpenSSL binaries. noPoll 1.0 depends on OpenSSL. However, if you have an already working OpenSSL installation you can skip it."

LangString DESC_SEC08 ${LANG_ENGLISH} "Test files, regression tests and other examples."

LangString DESC_SEC10 ${LANG_ENGLISH} "Devel headers (.h files) and additional includes (OpenSSL headers too)."

LangString DESC_SEC11 ${LANG_ENGLISH} "Devel libs: import libraries (.lib) definition files (exports .def) and other additional develment files."

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC01} $(DESC_SEC01)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC0112} $(DESC_SEC0112)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC021} $(DESC_SEC021)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC08} $(DESC_SEC08)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC10} $(DESC_SEC10)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC11} $(DESC_SEC11)
!insertmacro MUI_FUNCTION_DESCRIPTION_END
