param([string]$Run='cp31-full-probe',[string]$ImageBase='0x10000000')
$ErrorActionPreference='Stop'
if($Run -notmatch '^cp31-[a-z0-9-]+$') {throw 'Invalid run'}
$Root=Split-Path $PSScriptRoot -Parent
$Out=Join-Path $Root "build\$Run"
$Work="C:\dev\samp-r5-matching\$Run"
$Vs='C:\Program Files (x86)\Microsoft Visual Studio .NET 2003'
$env:PATH="$Vs\Common7\IDE;$Vs\Vc7\bin;"+$env:PATH
$env:INCLUDE="$Vs\Vc7\include;$Vs\Vc7\PlatformSDK\include;$Work\vendor\upstream\saco\d3d9\include"
$env:LIB="$Vs\Vc7\lib;$Vs\Vc7\PlatformSDK\lib;$(Join-Path $Root 'private\checkpoint31')"
$Sdk=@{}
$LinkLibraries=@()
foreach($Name in @('kernel32.lib','user32.lib','gdi32.lib','advapi32.lib','shell32.lib','comctl32.lib','winmm.lib','wsock32.lib','d3dx9.lib','d3d9.lib','bass.lib','dxguid.lib','dsound.lib','libcmt.lib','libcpmt.lib','oldnames.lib')) {
 $Found=$null
 if($Name -eq 'dxguid.lib') {$Found=Join-Path $Root 'private\checkpoint31\dxguid.lib'}
 else {foreach($Directory in $env:LIB.Split(';')) { $Candidate=Join-Path $Directory $Name; if(Test-Path $Candidate) {$Found=$Candidate;break} }}
 if(!$Found) {throw "SDK library absent: $Name"}
 $Sdk[$Name]=(Get-FileHash $Found).Hash.ToLowerInvariant()
 $LinkLibraries+=$Found
}
Copy-Item (Join-Path $Root 'tests') $Work -Recurse -Force
Copy-Item (Join-Path $Root 'private\checkpoint31\d3dx9_25.dll') $Work -Force
Copy-Item (Join-Path $Root 'private\checkpoint31\bass.dll') $Work -Force
Copy-Item (Join-Path $Root 'tests') $Out -Recurse -Force
Copy-Item (Join-Path $Root 'src') $Work -Recurse -Force
Copy-Item (Join-Path $Root 'src') $Out -Recurse -Force
Copy-Item (Join-Path $Root 'config\checkpoint31\exports.def') (Join-Path $Work 'exports.def') -Force
Push-Location $Work
try {
 & cl.exe /nologo /c /Ox /MT /Zp1 /EHsc /Gy /DNDEBUG /Fofoundation.obj tests\checkpoint31\foundation.cpp
 if($LASTEXITCODE -ne 0) {throw 'Foundation fixture compile failed'}
 & cl.exe /nologo /c /MT /Zp1 /EHsc /Foboundary.obj tests\checkpoint31\game_boundary.cpp
 if($LASTEXITCODE -ne 0) {throw 'Foundation fixture compile failed'}
 & cl.exe /nologo /c /MT /Zp1 /EHsc /Folayouts.obj tests\checkpoint31\layouts.cpp
 if($LASTEXITCODE -ne 0) {throw 'ABI layout compile failed'}
 & link.exe /nologo /dll /incremental:no /opt:ref /opt:noicf "/base:$ImageBase" /map:foundation.map /out:foundation.dll /def:exports.def foundation.obj boundary.obj layouts.obj raknet.lib dxut.lib utilities.lib @LinkLibraries
 if($LASTEXITCODE -ne 0) {throw 'Foundation link failed'}
 Copy-Item foundation.dll,foundation.map,foundation.lib,foundation.obj,boundary.obj,layouts.obj $Out -Force
 & cl.exe /nologo /MT /EHsc /Fenative.exe tests\checkpoint31\native.cpp foundation.lib
 if($LASTEXITCODE -ne 0) {throw 'Native fixture compile failed'}
 $Output=& .\native.exe
 $Code=$LASTEXITCODE
 $Output | Write-Output
 $Output | Set-Content -Encoding UTF8 (Join-Path $Out 'native.log')
 Copy-Item native.exe $Out -Force
 if($Code -ne 0) {throw "Native fixture failed: $Code"}
} finally {Pop-Location}
$Artifacts=@{}
foreach($Name in @('foundation.dll','foundation.map','foundation.lib','foundation.obj','boundary.obj','layouts.obj','native.exe','native.log','raknet.lib','dxut.lib','utilities.lib')) {
 $Artifacts[$Name]=(Get-FileHash (Join-Path $Out $Name)).Hash.ToLowerInvariant()
}
$Inputs=@{}
foreach($Folder in @('src','tests\checkpoint31')) {
 Get-ChildItem (Join-Path $Work $Folder) -Recurse -File | Where-Object {$_.FullName -notmatch '__pycache__'} | ForEach-Object {
  $Inputs[$_.FullName.Substring($Work.Length+1).Replace('\','/')]=(Get-FileHash $_.FullName).Hash.ToLowerInvariant()
 }
}
foreach($Folder in @('tools','config')) {
 Copy-Item (Join-Path $Root $Folder) $Out -Recurse -Force
 Get-ChildItem (Join-Path $Root $Folder) -Recurse -File | Where-Object {$_.FullName -notmatch '__pycache__'} | ForEach-Object {
  $Inputs[$_.FullName.Substring($Root.Length+1).Replace('\','/')]=(Get-FileHash $_.FullName).Hash.ToLowerInvariant()
 }
}
[ordered]@{run=$Run;image_base=$ImageBase;native_exit=$Code;inputs=$Inputs;artifacts=$Artifacts;sdk=$Sdk;
 runtimes=@{
 'd3dx9_25.dll'=(Get-FileHash (Join-Path $Work 'd3dx9_25.dll')).Hash.ToLowerInvariant();
 'bass.dll'=(Get-FileHash (Join-Path $Work 'bass.dll')).Hash.ToLowerInvariant()
 }
} | ConvertTo-Json -Depth 8 | Set-Content -Encoding UTF8 (Join-Path $Out 'link.json')
