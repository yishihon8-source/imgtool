[Setup]
AppName=ImageBatchTool
AppVersion=1.0.0
DefaultDirName={autopf}\ImageBatchTool
DefaultGroupName=ImageBatchTool
OutputDir=.
OutputBaseFilename=ImageBatchTool_Setup_v1.0.0
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64
; 如果你有图标文件可以加这一行
; SetupIconFile=ImageBatchTool\assets\icon.ico

[Files]
Source: "ImageBatchTool\ImageBatchTool.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "ImageBatchTool\msvcp140.dll"; DestDir: "{app}"
Source: "ImageBatchTool\vcruntime140.dll"; DestDir: "{app}"
Source: "ImageBatchTool\vcruntime140_1.dll"; DestDir: "{app}"
Source: "ImageBatchTool\使用说明.txt"; DestDir: "{app}"
Source: "ImageBatchTool\README.md"; DestDir: "{app}"
; assets 目录（如果存在且不为空）
Source: "ImageBatchTool\assets\*"; DestDir: "{app}\assets"; Flags: recursesubdirs createallsubdirs skipifsourcedoesntexist

[Icons]
Name: "{group}\ImageBatchTool"; Filename: "{app}\ImageBatchTool.exe"
Name: "{commondesktop}\ImageBatchTool"; Filename: "{app}\ImageBatchTool.exe"

[Run]
Filename: "{app}\ImageBatchTool.exe"; Description: "启动 ImageBatchTool"; Flags: nowait postinstall skipifsilent
