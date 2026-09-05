param([string]$Run='cp32-actor-probe',[string]$ImageBase='0x10000000',[string]$Units='game_actorped,game_entity,game_util,net_actorpool')
$ErrorActionPreference='Stop'
if($Run -notmatch '^cp32-[a-z0-9-]+$') {throw 'Invalid run'}
$Root=Split-Path $PSScriptRoot -Parent
$Out=Join-Path $Root "build\$Run"
$Work="C:\dev\samp-r5-matching\$Run"
if((Test-Path $Out) -or (Test-Path $Work)) {throw 'Run already exists; preserve evidence and choose a new name'}
New-Item -ItemType Directory -Force $Out,$Work | Out-Null
Copy-Item (Join-Path $Root 'client') $Work -Recurse -Force
Copy-Item (Join-Path $Root 'client') $Out -Recurse -Force
Copy-Item (Join-Path $Root 'vendor') $Work -Recurse -Force
Copy-Item (Join-Path $Root 'vendor') $Out -Recurse -Force
Copy-Item (Join-Path $Root 'tests') $Work -Recurse -Force
if(!(Test-Path "$Work\client\raknet")) {New-Item -ItemType Junction -Path "$Work\client\raknet" -Target "$Work\vendor\upstream\raknet" | Out-Null}
if(!(Test-Path "$Work\client\saco\d3d9")) {New-Item -ItemType Junction -Path "$Work\client\saco\d3d9" -Target "$Work\vendor\upstream\saco\d3d9" | Out-Null}
$Vs='C:\Program Files (x86)\Microsoft Visual Studio .NET 2003'
$env:PATH="$Vs\Common7\IDE;$Vs\Vc7\bin;"+$env:PATH
$env:INCLUDE="$Vs\Vc7\include;$Vs\Vc7\PlatformSDK\include;$Work\vendor\upstream\saco\d3d9\include"
$env:LIB="$Vs\Vc7\lib;$Vs\Vc7\PlatformSDK\lib"
$Opts=@('/nologo','/c','/Ox','/Og','/Ob1','/Oi','/Ot','/Oy','/MT','/Zp1','/EHsc','/Gy','/DNDEBUG','/DWIN32','/D_WINDOWS')
$Records=@()
Push-Location $Work
try {
 foreach($Unit in $Units.Split(',')) {
  if($Unit -notmatch '^[a-zA-Z0-9_/-]+$') {throw 'Invalid unit'}
  $Name=$Unit.Replace('/','_')
  & cl.exe @Opts "/Fo$Name.obj" "client/matching/$Unit.cpp"
  if($LASTEXITCODE -ne 0) {throw "Compile failed: $Unit"}
  Copy-Item "$Name.obj" $Out -Force
  $Records += [ordered]@{unit=$Unit;object="$Name.obj";object_sha256=(Get-FileHash "$Name.obj").Hash.ToLowerInvariant()}
 }
 Copy-Item (Join-Path $Root 'config\checkpoint32\exports.def') exports.def -Force
 & link.exe /nologo /dll /incremental:no /opt:ref /opt:noicf "/base:$ImageBase" /map:actor.map /out:actor.dll /def:exports.def game_actorped.obj game_entity.obj game_util.obj net_actorpool.obj libcmt.lib kernel32.lib
 if($LASTEXITCODE -ne 0) {throw 'Link failed'}
 Copy-Item actor.dll,actor.map,actor.lib $Out -Force
 & cl.exe /nologo /MT /EHsc /Zp1 /Fenative.exe tests\checkpoint32\native.cpp actor.lib
 if($LASTEXITCODE -ne 0) {throw 'Native compilation failed'}
 $Output=& .\native.exe
 $Code=$LASTEXITCODE
 $Output | Write-Output
 $Output | Set-Content -Encoding UTF8 (Join-Path $Out 'native.log')
 Copy-Item native.exe $Out -Force
 if($Code -ne 0) {throw 'Native tests failed'}
} finally {Pop-Location}
$Sources=@{}
foreach($Folder in @('client','vendor','config/checkpoint32','tests/checkpoint32','tools')) {
 if($Folder -notin @('client','vendor')) { $Dest=Join-Path $Out $Folder; New-Item -ItemType Directory -Force $Dest | Out-Null; Copy-Item (Join-Path (Join-Path $Root $Folder) '*') $Dest -Recurse -Force }
 Get-ChildItem (Join-Path $Root $Folder) -Recurse -File | Where-Object {$_.FullName -notmatch '__pycache__'} | ForEach-Object {
 $Sources[$_.FullName.Substring($Root.Length+1).Replace('\','/')]=(Get-FileHash $_.FullName).Hash.ToLowerInvariant()
}
}
$Sdk=@{}
foreach($Name in @('libcmt.lib','kernel32.lib')) {
 foreach($Directory in $env:LIB.Split(';')) {
  $Candidate=Join-Path $Directory $Name
  if(Test-Path $Candidate) {$Sdk[$Name]=(Get-FileHash $Candidate).Hash.ToLowerInvariant();break}
 }
}
$Artifacts=@{}
foreach($Name in @('actor.dll','actor.map','actor.lib','native.exe','native.log')) {$Artifacts[$Name]=(Get-FileHash (Join-Path $Out $Name)).Hash.ToLowerInvariant()}
$Tools=@()
foreach($Name in @('cl.exe','c1xx.dll','c2.dll','link.exe')) {
 $Path="$Vs\Vc7\bin\$Name"
 $Tools += [ordered]@{name=$Name;version=(Get-Item $Path).VersionInfo.FileVersion;sha256=(Get-FileHash $Path).Hash.ToLowerInvariant()}
}
[ordered]@{run=$Run;image_base=$ImageBase;artifacts=$Artifacts;sdk=$Sdk;native_exit=$Code;options=$Opts;units=$Records;sources=$Sources;toolchain=$Tools} | ConvertTo-Json -Depth 7 | Set-Content -Encoding UTF8 (Join-Path $Out 'build.json')
