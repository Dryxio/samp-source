# Incremental client compiler. This produces candidates, never acceptance.
param([string]$Run,[string]$Units,[ValidateSet("Ob1","Ob2")][string]$Inline="Ob1",[switch]$PoolStrings)
$ErrorActionPreference='Stop'
if($Run -notmatch '^cp32-[a-z0-9-]+$') {throw 'Invalid run'}
$Root=Split-Path $PSScriptRoot -Parent
$Out=Join-Path $Root "build\$Run"
if(Test-Path $Out) {throw 'Preserve existing run'}
$Work='C:\dev\samp-r5-matching\client-work'
New-Item -ItemType Directory -Force $Out,$Work | Out-Null
$Vs='C:\Program Files (x86)\Microsoft Visual Studio .NET 2003'
$env:PATH="$Vs\Common7\IDE;$Vs\Vc7\bin;"+$env:PATH
$env:INCLUDE="$Vs\Vc7\include;$Vs\Vc7\PlatformSDK\include;$Work\vendor\upstream\saco\d3d9\include"
$Opts=@('/nologo','/c','/Ox','/Og',"/$Inline",'/Oi','/Ot','/Oy','/MT','/Zp1','/EHsc','/Gy','/DNDEBUG','/DWIN32','/D_WINDOWS')
if($PoolStrings) {$Opts+= '/GF'}
$Inputs=@{}
foreach($Folder in @('client','vendor')) {
 Copy-Item (Join-Path $Root $Folder) $Work -Recurse -Force
 Copy-Item (Join-Path $Root $Folder) $Out -Recurse -Force
 Get-ChildItem (Join-Path $Root $Folder) -Recurse -File | ForEach-Object {$Inputs[$_.FullName.Substring($Root.Length+1).Replace('\','/')]=(Get-FileHash $_.FullName).Hash.ToLowerInvariant()}
}
if(!(Test-Path "$Work\client\raknet")) {New-Item -ItemType Junction -Path "$Work\client\raknet" -Target "$Work\vendor\upstream\raknet" | Out-Null}
if(!(Test-Path "$Work\client\saco\d3d9")) {New-Item -ItemType Junction -Path "$Work\client\saco\d3d9" -Target "$Work\vendor\upstream\saco\d3d9" | Out-Null}
$ToolHashes=@{}
foreach($Name in @('cl.exe','c1xx.dll','c2.dll')) {$ToolHashes[$Name]=(Get-FileHash "$Vs\Vc7\bin\$Name").Hash.ToLowerInvariant()}
$Common=($Opts -join '|')+($ToolHashes.Keys | Sort-Object | ForEach-Object {$_+$ToolHashes[$_]})
$Common+=($Inputs.Keys | Where-Object {$_ -match '\.(h|hpp|inl)$'} | Sort-Object | ForEach-Object {$_+$Inputs[$_]})
$Records=@()
Push-Location $Work
try {
 foreach($Unit in $Units.Split(',')) {
  if($Unit -notmatch '^[a-zA-Z0-9_/-]+$') {throw 'Invalid unit'}
  $File="client/saco/$Unit.cpp"; $Name=$Unit.Replace('/','_')
  if(!$Inputs.ContainsKey($File)) {throw "Missing source $File"}
  $Hash=[System.Security.Cryptography.SHA256]::Create()
  $Key=([BitConverter]::ToString($Hash.ComputeHash([Text.Encoding]::UTF8.GetBytes(($Common -join '|')+$File+$Inputs[$File])))).Replace('-','').ToLowerInvariant()
  $Cache="$Work\cache-$Key.obj";$Cached=Test-Path $Cache
  if(!$Cached) {
   & cl.exe @Opts "/Fo$Cache" $File
   if($LASTEXITCODE -ne 0) {throw "Compile failed $Unit"}
  }
  Copy-Item $Cache (Join-Path $Out "$Name.obj")
  $Records += [ordered]@{unit=$Unit;object="$Name.obj";sha256=(Get-FileHash $Cache).Hash.ToLowerInvariant();cache_key=$Key;reused=$Cached}
 }
} finally {Pop-Location}
foreach($File in $Inputs.Keys) {if((Get-FileHash (Join-Path $Root $File)).Hash.ToLowerInvariant() -ne $Inputs[$File]) {throw "Input changed during compilation $File"}}
[ordered]@{status='CANDIDATES_NOT_ACCEPTED';sources=$Inputs;options=$Opts;tools=$ToolHashes;units=$Records} | ConvertTo-Json -Depth 7 | Set-Content -Encoding UTF8 (Join-Path $Out 'probe.json')
