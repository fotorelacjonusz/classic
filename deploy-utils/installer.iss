[Setup]
WizardStyle=modern

AppName=Fotorelacjonusz
AppVersion="{#APP_VERSION}"
AppCopyright=Copyright (C) 2012-2013 Kamil Ostaszewski, 2019-2020 Sebastian Skałacki
AppPublisher=Sebastian Skałacki
AppPublisherURL=https://github.com/fotorelacjonusz/original/
LicenseFile=Fotorelacjonusz\COPYING.txt

OutputDir=.

DefaultDirName={autopf}\Fotorelacjonusz
ArchitecturesAllowed="{#APP_ARCH}"

[Files]
Source: "Fotorelacjonusz\*"; DestDir: "{app}"; Flags: recursesubdirs

[Icons]
Name: "{commonprograms}\Fotorelacjonusz"; Filename: "{app}\Fotorelacjonusz.exe"; WorkingDir: "{app}"; IconFilename: "{app}\icon\rounded.ico"
Name: "{commondesktop}\Fotorelacjonusz"; Filename: "{app}\Fotorelacjonusz.exe"; WorkingDir: "{app}"; IconFilename: "{app}\icon\rounded.ico"
