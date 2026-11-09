; Windows installer for the Mini Knobz bundle.
; Each plugin is a checkbox; VST3s go to the VST3 folder (Common Files\VST3 by default),
; the GASP standalone app goes to Program Files.
#ifndef AppVersion
  #define AppVersion "1.0.0"
#endif
#ifndef DistDir
  #define DistDir "..\dist"
#endif

[Setup]
AppId={{B8012ED2-C87F-46C7-86E0-93531F88076B}
AppName=Mini Knobz
AppVersion={#AppVersion}
AppPublisher=Audio Makers
DefaultDirName={autopf}\Audio Makers\Mini Knobz
DefaultGroupName=Mini Knobz
DisableProgramGroupPage=yes
DisableDirPage=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
OutputBaseFilename=Mini Knobz {#AppVersion} Instalador
OutputDir=..\dist
SetupIconFile=..\build\KnobzGasp_artefacts\JuceLibraryCode\icon.ico
UninstallDisplayIcon={app}\Knobz Gasp.exe
Compression=lzma2
SolidCompression=yes
WizardStyle=modern

[Languages]
Name: "pt"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"

[Types]
Name: "full"; Description: "Todos os plugins"
Name: "custom"; Description: "Escolher os plugins"; Flags: iscustom

[Components]
Name: "bite"; Description: "Knobz Bite (saturação)"; Types: full custom
Name: "phat"; Description: "Knobz Phat (fattener + exciter)"; Types: full custom
Name: "kickside"; Description: "Knobz Kickside (sidechain pump)"; Types: full custom
Name: "wide"; Description: "Knobz Wide (estéreo)"; Types: full custom
Name: "gasp"; Description: "Knobz Gasp (reverse reverb)"; Types: full custom

[Tasks]
Name: "desktopicon"; Description: "Criar atalho do Knobz Gasp na área de trabalho"; Components: gasp; Flags: unchecked

[Files]
Source: "{#DistDir}\Knobz Bite.vst3\*"; DestDir: "{code:Vst3Dir}\Knobz Bite.vst3"; Components: bite; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#DistDir}\Knobz Phat.vst3\*"; DestDir: "{code:Vst3Dir}\Knobz Phat.vst3"; Components: phat; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#DistDir}\Knobz Kickside.vst3\*"; DestDir: "{code:Vst3Dir}\Knobz Kickside.vst3"; Components: kickside; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#DistDir}\Knobz Wide.vst3\*"; DestDir: "{code:Vst3Dir}\Knobz Wide.vst3"; Components: wide; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#DistDir}\Knobz Gasp.vst3\*"; DestDir: "{code:Vst3Dir}\Knobz Gasp.vst3"; Components: gasp; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#DistDir}\Knobz Gasp.exe"; DestDir: "{app}"; Components: gasp; Flags: ignoreversion
Source: "..\Assets\Fonts\OFL-SpaceGrotesk.txt"; DestDir: "{app}"; DestName: "Font licenses.txt"; Flags: ignoreversion

[Icons]
Name: "{group}\Knobz Gasp"; Filename: "{app}\Knobz Gasp.exe"; Components: gasp
Name: "{autodesktop}\Knobz Gasp"; Filename: "{app}\Knobz Gasp.exe"; Tasks: desktopicon

[Registry]
Root: HKLM; Subkey: "Software\Audio Makers\Mini Knobz"; ValueType: string; ValueName: "VST3Path"; ValueData: "{code:Vst3Dir}"; Flags: uninsdeletekey

[UninstallDelete]
Type: filesandordirs; Name: "{reg:HKLM\Software\Audio Makers\Mini Knobz,VST3Path|{commoncf64}\VST3}\Knobz Bite.vst3"
Type: filesandordirs; Name: "{reg:HKLM\Software\Audio Makers\Mini Knobz,VST3Path|{commoncf64}\VST3}\Knobz Phat.vst3"
Type: filesandordirs; Name: "{reg:HKLM\Software\Audio Makers\Mini Knobz,VST3Path|{commoncf64}\VST3}\Knobz Kickside.vst3"
Type: filesandordirs; Name: "{reg:HKLM\Software\Audio Makers\Mini Knobz,VST3Path|{commoncf64}\VST3}\Knobz Wide.vst3"
Type: filesandordirs; Name: "{reg:HKLM\Software\Audio Makers\Mini Knobz,VST3Path|{commoncf64}\VST3}\Knobz Gasp.vst3"

[Code]
var
  Vst3Page: TInputDirWizardPage;

procedure InitializeWizard;
begin
  Vst3Page := CreateInputDirPage(wpSelectComponents, 'Pasta do VST3',
    'Onde os plugins VST3 v' + #227 + 'o ser instalados?',
    'Os plugins v' + #227 + 'o para a pasta abaixo. A padr' + #227 + 'o ' + #233 + ' a pasta VST3 que todas as DAWs procuram. Clique em Procurar para escolher outra.',
    False, '');
  Vst3Page.Add('');
  Vst3Page.Values[0] := GetPreviousData('VST3Path', ExpandConstant('{commoncf64}\VST3'));
end;

procedure RegisterPreviousData(PreviousDataKey: Integer);
begin
  SetPreviousData(PreviousDataKey, 'VST3Path', Vst3Page.Values[0]);
end;

function Vst3Dir(Param: String): String;
begin
  Result := Vst3Page.Values[0];
end;
