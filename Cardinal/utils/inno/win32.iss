#include "version.iss"

[Setup]
AppName=Cardinal
AppPublisher=DISTRHO
AppPublisherURL=https://github.com/DISTRHO/Cardinal/
AppSupportURL=https://github.com/DISTRHO/Cardinal/issues/
AppUpdatesURL=https://github.com/DISTRHO/Cardinal/releases/
AppVersion={#VERSION}
DefaultDirName={commonpf32}\Cardinal
DisableDirPage=yes
DisableWelcomePage=no
LicenseFile=..\..\LICENSE
OutputBaseFilename=Cardinal-win32-{#VERSION}-installer
OutputDir=.
UsePreviousAppDir=no

[Types]
Name: "normal"; Description: "Full installation";
Name: "custom"; Description: "Custom installation"; Flags: iscustom;

[Components]
Name: resources; Description: "Resources"; Types: normal custom; Flags: fixed;
Name: carla; Description: "Carla/Ildaeil host tools"; Types: normal;
Name: jack; Description: "Standalone (JACK)"; Types: custom;
Name: native; Description: "Standalone (Native)"; Types: custom;
Name: lv2; Description: "LV2 plugin"; Types: normal;
Name: vst2; Description: "VST2 plugin"; Types: normal;
Name: vst3; Description: "VST3 plugin"; Types: normal;
Name: clap; Description: "CLAP plugin"; Types: normal;

[Files]
#include "resources.iss"
; icon
Source: "..\..\utils\distrho.ico"; DestDir: "{app}"; Components: resources; Flags: ignoreversion;
; carla
Source: "..\..\carla\bin\carla-bridge-*.*"; DestDir: "{commoncf32}\Cardinal\Carla"; Components: carla; Flags: ignoreversion;
Source: "..\..\carla\bin\carla-discovery-*.exe"; DestDir: "{commoncf32}\Cardinal\Carla"; Components: carla; Flags: ignoreversion;
Source: "..\..\carla\bin\libcarla_frontend.dll"; DestDir: "{commoncf32}\Cardinal\Carla"; Components: carla; Flags: ignoreversion;
Source: "..\..\carla\bin\libcarla_utils.dll"; DestDir: "{commoncf32}\Cardinal\Carla"; Components: carla; Flags: ignoreversion;
Source: "..\..\carla\build\Carla\libpython3.8.dll"; DestDir: "{commoncf32}\Cardinal\Carla\resources"; Components: carla; Flags: ignoreversion;
Source: "..\..\carla\build\Carla\Qt5*.dll"; DestDir: "{commoncf32}\Cardinal\Carla\resources"; Components: carla; Flags: ignoreversion;
Source: "..\..\carla\build\Carla\resources\*.*"; DestDir: "{commoncf32}\Cardinal\Carla\resources"; Components: carla; Flags: ignoreversion;
Source: "..\..\carla\build\Carla\iconengines\*.*"; DestDir: "{commoncf32}\Cardinal\Carla\resources\iconengines"; Components: carla; Flags: ignoreversion;
Source: "..\..\carla\build\Carla\imageformats\*.*"; DestDir: "{commoncf32}\Cardinal\Carla\resources\imageformats"; Components: carla; Flags: ignoreversion;
Source: "..\..\carla\build\Carla\platforms\*.*"; DestDir: "{commoncf32}\Cardinal\Carla\resources\platforms"; Components: carla; Flags: ignoreversion;
Source: "..\..\carla\build\Carla\styles\*.*"; DestDir: "{commoncf32}\Cardinal\Carla\resources\styles"; Components: carla; Flags: ignoreversion;
Source: "..\..\carla\build\Carla\resources\lib\*.*"; DestDir: "{commoncf32}\Cardinal\Carla\resources\lib"; Components: carla; Flags: ignoreversion;
Source: "..\..\carla\build\Carla\resources\lib\PyQt5\*.*"; DestDir: "{commoncf32}\Cardinal\Carla\resources\lib\PyQt5"; Components: carla; Flags: ignoreversion;
; jack
Source: "..\..\bin\Cardinal.exe"; DestDir: "{app}"; Components: jack; Flags: ignoreversion;
; native
Source: "..\..\bin\CardinalNative.exe"; DestDir: "{app}"; Components: native; Flags: ignoreversion;
; lv2
Source: "..\..\bin\Cardinal.lv2\*.*"; DestDir: "{commoncf32}\LV2\Cardinal.lv2"; Components: lv2; Flags: ignoreversion;
Source: "..\..\bin\CardinalFX.lv2\*.*"; DestDir: "{commoncf32}\LV2\CardinalFX.lv2"; Components: lv2; Flags: ignoreversion;
Source: "..\..\bin\CardinalSynth.lv2\*.*"; DestDir: "{commoncf32}\LV2\CardinalSynth.lv2"; Components: lv2; Flags: ignoreversion;
; vst2
Source: "..\..\bin\Cardinal.vst\*.*"; DestDir: "{code:GetVST2Dir}\Cardinal.vst"; Components: vst2; Flags: ignoreversion;
; vst3
Source: "..\..\bin\Cardinal.vst3\Contents\x86-win\Cardinal.vst3"; DestDir: "{commoncf32}\VST3\Cardinal.vst3\Contents\x86-win"; Components: vst3; Flags: ignoreversion;
Source: "..\..\bin\CardinalFX.vst3\Contents\x86-win\CardinalFX.vst3"; DestDir: "{commoncf32}\VST3\CardinalFX.vst3\Contents\x86-win"; Components: vst3; Flags: ignoreversion;
Source: "..\..\bin\CardinalSynth.vst3\Contents\x86-win\CardinalSynth.vst3"; DestDir: "{commoncf32}\VST3\CardinalSynth.vst3\Contents\x86-win"; Components: vst3; Flags: ignoreversion;
; clap
Source: "..\..\bin\Cardinal.clap\*.*"; DestDir: "{commoncf32}\CLAP\Cardinal.clap"; Components: clap; Flags: ignoreversion;

[Icons]
Name: "{commonprograms}\Cardinal (JACK)"; Filename: "{app}\Cardinal.exe"; IconFilename: "{app}\distrho.ico"; WorkingDir: "{app}"; Comment: "Virtual modular synthesizer plugin (JACK variant)"; Components: jack;
Name: "{commonprograms}\Cardinal (Native)"; Filename: "{app}\CardinalNative.exe"; IconFilename: "{app}\distrho.ico"; WorkingDir: "{app}"; Comment: "Virtual modular synthesizer plugin (Native variant)"; Components: native;

; based on https://www.kvraudio.com/forum/viewtopic.php?t=501615
[Code]
var
  VST2DirPage: TInputDirWizardPage;
  TypesComboOnChangePrev: TNotifyEvent;
procedure ComponentsListCheckChanges;
begin
  WizardForm.NextButton.Enabled := (WizardSelectedComponents(False) <> '');
end;
procedure ComponentsListClickCheck(Sender: TObject);
begin
  ComponentsListCheckChanges;
end;
procedure TypesComboOnChange(Sender: TObject);
begin
  TypesComboOnChangePrev(Sender);
  ComponentsListCheckChanges;
end;
procedure InitializeWizard;
begin
  WizardForm.ComponentsList.OnClickCheck := @ComponentsListClickCheck;
  TypesComboOnChangePrev := WizardForm.TypesCombo.OnChange;
  WizardForm.TypesCombo.OnChange := @TypesComboOnChange;
  VST2DirPage := CreateInputDirPage(wpSelectComponents,
  'Confirm VST2 Plugin Directory', '',
  'Select the folder in which setup should install the VST2 Plugin, then click Next.',
  False, '');
  VST2DirPage.Add('VST2 Plugin Directory');
  VST2DirPage.Values[0] := ExpandConstant('{reg:HKLM\SOFTWARE\VST,VSTPluginsPath|{commonpf32}\VSTPlugins}');
end;
procedure CurPageChanged(CurPageID: Integer);
begin
  if CurPageID = VST2DirPage.ID then
  begin
    VST2DirPage.Buttons[0].Enabled := WizardIsComponentSelected('vst2');
    VST2DirPage.PromptLabels[0].Enabled := VST2DirPage.Buttons[0].Enabled;
    VST2DirPage.Edits[0].Enabled := VST2DirPage.Buttons[0].Enabled;
  end;
  if CurPageID = wpSelectComponents then
  begin
    ComponentsListCheckChanges;
  end;
end;
function ShouldSkipPage(PageID: Integer): Boolean;
begin
  if PageID = VST2DirPage.ID then
  begin
    If (not WizardIsComponentSelected('vst2'))then
      begin
        Result := True
      end;
  end;
end;
function GetVST2Dir(Param: string): string;
begin
    Result := VST2DirPage.Values[0];
end;
