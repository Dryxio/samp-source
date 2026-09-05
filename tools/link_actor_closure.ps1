param([string]$Run='cp32-closure-linked1',[string]$Ob1='cp32-closure-link-ob1',[string]$Ob2='cp32-closure-link-ob2')
$ErrorActionPreference='Stop'
$Root=Split-Path $PSScriptRoot -Parent
foreach($Value in @($Run,$Ob1,$Ob2)) {if($Value -notmatch '^cp32-[a-z0-9-]+$') {throw 'Invalid run'}}
$Out=Join-Path $Root "build\$Run"; if(Test-Path $Out) {throw 'Preserve existing run'}
New-Item -ItemType Directory $Out | Out-Null
$Vs='C:\Program Files (x86)\Microsoft Visual Studio .NET 2003'
$env:PATH="$Vs\Common7\IDE;$Vs\Vc7\bin;"+$env:PATH
$env:LIB="$Root\private\checkpoint31;$Vs\Vc7\lib;$Vs\Vc7\PlatformSDK\lib"
$Objects=@();$Probes=@()
foreach($Source in @($Ob1,$Ob2)) {
 $Directory=Join-Path $Root "build\$Source";$Probe=Get-Content (Join-Path $Directory 'probe.json') -Raw | ConvertFrom-Json
 $Probes+=$Source
 foreach($Unit in $Probe.units) {Copy-Item (Join-Path $Directory $Unit.object) $Out; $Objects+=$Unit.object}
 Copy-Item (Join-Path $Directory 'probe.json') (Join-Path $Out "$Source.json")
}
Copy-Item (Join-Path $Root 'config\checkpoint32\closure-exports.def') $Out
Push-Location $Out
try {
 & link.exe /nologo /dll /incremental:no /opt:ref /opt:noicf /base:0x10000000 /map:closure.map /out:closure.dll /def:closure-exports.def @Objects kernel32.lib user32.lib wsock32.lib d3dx9.lib comctl32.lib gdi32.lib shell32.lib advapi32.lib
 if($LASTEXITCODE -ne 0) {throw 'Link failed'}
} finally {Pop-Location}
$Hashes=@{}
Get-ChildItem $Out -File | ForEach-Object {$Hashes[$_.Name]=(Get-FileHash $_.FullName).Hash.ToLowerInvariant()}
$Sdk=@{}
foreach($Name in @('libcmt.lib','libcpmt.lib','oldnames.lib','kernel32.lib','user32.lib','uuid.lib','wsock32.lib','d3dx9.lib','comctl32.lib','gdi32.lib','shell32.lib','advapi32.lib')) {
 foreach($Directory in $env:LIB.Split(';')) {
  $Path=Join-Path $Directory $Name
  if(Test-Path $Path) {$Sdk[$Name]=(Get-FileHash $Path).Hash.ToLowerInvariant();break}
 }
}
[ordered]@{status='LINKED_CANDIDATE_NOT_ACCEPTED';probes=$Probes;artifacts=$Hashes;sdk=$Sdk;linker_sha256=(Get-FileHash "$Vs\Vc7\bin\link.exe").Hash.ToLowerInvariant()} | ConvertTo-Json -Depth 6 | Set-Content -Encoding UTF8 (Join-Path $Out 'link.json')
