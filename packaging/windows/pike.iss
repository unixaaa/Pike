; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
AppName=Pike 7.6 *BETA*
AppVerName=Pike 7.6.53, SDL, OpenGL, MySQL, Freetype, Gz and GTK+
AppVersion=7.6.53
VersionInfoVersion=7.6.53.0
AppPublisher=The Pike Team
AppPublisherURL=http://pike.ida.liu.se/
AppSupportURL=http://pike.ida.liu.se/
AppUpdatesURL=http://pike.ida.liu.se/
AppCopyright=Copyright 2004 Link�ping University
DefaultDirName={pf}\Pike
DefaultGroupName=Pike
AllowNoIcons=true
AlwaysShowComponentsList=false
LicenseFile=X:\win32-pike\pikeinstaller\Copying.txt
; uncomment the following line if you want your installation to run on NT 3.51 too.
; MinVersion=4,3.51

;FIXME: pike.exe failed on win98 if Pike already is installed. Fix automatic
;uninstall. Late note: Seems to fail on XP too. Applies to 7.4 as well?

Compression=lzma/ultra
SetupIconFile=X:\win32-pike\icons\pikeinstall.ico
UninstallIconFile=X:\win32-pike\icons\pike_red.ico
InternalCompressionLevel=ultra

[Files]
Source: X:\win32-pike\dists\Pike-v7.6.13-Win32-Windows-NT-5.1.2600-i86pc.exe; DestDir: {tmp}; Flags: deleteafterinstall ignoreversion
Source: "X:\win32-pike\icons\pike_black.ico"; DestDir: "{app}/icons"; Flags: ignoreversion; Attribs: hidden
Source: "X:\win32-pike\icons\pike_blue.ico"; DestDir: "{app}/icons"; Flags: ignoreversion; Attribs: hidden
Source: "X:\win32-pike\icons\pike_green.ico"; DestDir: "{app}/icons"; Flags: ignoreversion; Attribs: hidden
Source: "X:\win32-pike\icons\pike_magenta.ico"; DestDir: "{app}/icons"; Flags: ignoreversion; Attribs: hidden
Source: "X:\win32-pike\icons\pike_orange.ico"; DestDir: "{app}/icons"; Flags: ignoreversion; Attribs: hidden
Source: "X:\win32-pike\icons\pike_red.ico"; DestDir: "{app}/icons"; Flags: ignoreversion; Attribs: hidden


;;;Begin SDL files
Source: "X:\win32-pike\dlls\SDL.dll"; DestDir: "{app}/bin"; Flags: ignoreversion restartreplace sharedfile; Components: sdl
;;;End SDL files

;;;Begin GLU files
Source: "X:\win32-pike\dlls\glu32.dll"; DestDir: "{app}/bin"; Flags: ignoreversion restartreplace sharedfile; Components: GLU
;;;End GLU files

;;;Begin GTK+ files
Source: "X:\win32-pike\dlls\gdk-1.3.dll"; DestDir: "{app}\bin"; Flags: ignoreversion restartreplace sharedfile; Components: gtk
Source: "X:\win32-pike\dlls\gdk_imlib.dll"; DestDir: "{app}\bin"; Flags: ignoreversion restartreplace sharedfile; Components: gtk
Source: "X:\win32-pike\dlls\glib-1.3.dll"; DestDir: "{app}\bin"; Flags: ignoreversion restartreplace sharedfile; Components: gtk
Source: "X:\win32-pike\dlls\gmodule-1.3.dll"; DestDir: "{app}\bin"; Flags: ignoreversion restartreplace sharedfile; Components: gtk
Source: "X:\win32-pike\dlls\gnu-intl.dll"; DestDir: "{app}\bin"; Flags: ignoreversion restartreplace sharedfile; Components: gtk
Source: "X:\win32-pike\dlls\gobject-1.3.dll"; DestDir: "{app}\bin"; Flags: ignoreversion restartreplace sharedfile; Components: gtk
Source: "X:\win32-pike\dlls\gthread-1.3.dll"; DestDir: "{app}\bin"; Flags: ignoreversion restartreplace sharedfile; Components: gtk
Source: "X:\win32-pike\dlls\gtk-1.3.dll"; DestDir: "{app}\bin"; Flags: ignoreversion restartreplace sharedfile; Components: gtk
Source: "X:\win32-pike\dlls\iconv-1.3.dll"; DestDir: "{app}\bin"; Flags: ignoreversion restartreplace sharedfile; Components: gtk
Source: "X:\win32-pike\dlls\iconv.dll"; DestDir: "{app}\bin"; Flags: ignoreversion restartreplace sharedfile; Components: gtk
Source: "X:\win32-pike\dlls\imlib-jpeg.dll"; DestDir: "{app}\bin"; Flags: ignoreversion restartreplace sharedfile; Components: gtk
Source: "X:\win32-pike\dlls\imlib-png.dll"; DestDir: "{app}\bin"; Flags: ignoreversion restartreplace sharedfile; Components: gtk
Source: "X:\win32-pike\dlls\imlib-tiff.dll"; DestDir: "{app}\bin"; Flags: ignoreversion restartreplace sharedfile; Components: gtk
Source: "X:\win32-pike\dlls\libgdk-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion restartreplace sharedfile; Components: gtk
Source: "X:\win32-pike\dlls\libglib-2.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion restartreplace sharedfile; Components: gtk
Source: "X:\win32-pike\dlls\libgmodule-2.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion restartreplace sharedfile; Components: gtk
Source: "X:\win32-pike\dlls\libgobject-2.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion restartreplace sharedfile; Components: gtk
Source: "X:\win32-pike\dlls\libgthread-2.0-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion restartreplace sharedfile; Components: gtk
Source: "X:\win32-pike\dlls\libgtk-0.dll"; DestDir: "{app}\bin"; Flags: ignoreversion restartreplace sharedfile; Components: gtk
Source: "X:\win32-pike\dlls\libintl-1.dll"; DestDir: "{app}\bin"; Flags: ignoreversion restartreplace sharedfile; Components: gtk
Source: "X:\win32-pike\dlls\localcharset.dll"; DestDir: "{app}\bin"; Flags: ignoreversion restartreplace sharedfile; Components: gtk
;;;End GTK+ files


[Run]
Filename: {tmp}\Pike-v7.6.13-Win32-Windows-NT-5.1.2600-i86pc.exe; Parameters: "--no-gui --traditional ""prefix={app}"""

[Types]
;Name: "custom"; Description: "Pike only"; Flags: iscustom

[Components]
Name: gtk; Description: "GTK+"; Types: full custom;
Name: sdl; Description: "SDL"; Types: full custom;
Name: glu; Description: "GLU"; Types: full custom;
Name: pike; Description: Pike; Types: full custom; Flags: fixed; ExtraDiskSpaceRequired: 40000000

[Tasks]
;;Name: gtk; Description: "Install GTK+ support"
;Name: mysql; Description: "Install MySQL client support"
Name: associate; Description: Associate .pike and .pmod extensions with Pike

[Dirs]
;Name: "{app}\apps"; Flags: uninsneveruninstall

[Registry]
Root: HKCR; Subkey: .pike; ValueType: string; ValueData: pike_file; Tasks: associate
Root: HKCR; Subkey: .pike; ValueType: string; ValueName: ContentType; ValueData: text/x-pike-code; Tasks: associate
Root: HKCR; Subkey: .pmod; ValueType: string; ValueData: pike_module; Tasks: associate
Root: HKCR; Subkey: .pmod; ValueType: string; ValueName: ContentType; ValueData: text/x-pike-code; Tasks: associate
Root: HKCR; Subkey: pike_file; ValueType: string; ValueData: Pike program file; Tasks: associate
Root: HKCR; Subkey: pike_file\DefaultIcon; ValueType: string; ValueData: {app}\pike.ico,0; Tasks: associate
Root: HKCR; Subkey: pike_file\Shell\Open\Command; ValueType: string; ValueData: """{app}\bin\pike.exe"" ""%1"""; Flags: uninsdeletevalue; Tasks: associate
Root: HKCR; Subkey: pike_file\Shell\Edit\Command; ValueType: string; ValueData: """notepad.exe"" ""%1"""; Flags: createvalueifdoesntexist; Tasks: associate
Root: HKCR; Subkey: pike_module; ValueType: string; ValueData: Pike module file; Tasks: associate
;Root: HKCR; Subkey: pike_module\DefaultIcon; ValueType: string; ValueData: {app}\pike.ico,0; Tasks: associate
Root: HKCR; Subkey: pike_module\Shell\Edit\Command; ValueType: string; ValueData: """notepad.exe"" ""%1"""; Flags: createvalueifdoesntexist; Tasks: associate

[Icons]
Name: {group}\Pike; Filename: {app}\bin\pike.exe

[UninstallDelete]
Type: filesandordirs; Name: {app}\bin
Type: filesandordirs; Name: {app}\include
Type: filesandordirs; Name: {app}\lib
Type: filesandordirs; Name: {app}\man
