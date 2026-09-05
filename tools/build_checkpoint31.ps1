param([string]$Run='cp31-probe',[string]$OnlyGroup='all')
$ErrorActionPreference='Stop'
$ProgressPreference='SilentlyContinue'
if($Run -notmatch '^cp31-[a-z0-9-]+$') {throw 'Invalid run'}
$Root=Split-Path $PSScriptRoot -Parent
$Out=Join-Path $Root "build\$Run"
$Work="C:\dev\samp-r5-matching\$Run"
New-Item -ItemType Directory -Force $Out,$Work | Out-Null
Copy-Item (Join-Path $Root 'vendor') $Work -Recurse -Force
Copy-Item (Join-Path $Root 'vendor') $Out -Recurse -Force
$Vs='C:\Program Files (x86)\Microsoft Visual Studio .NET 2003'
$env:PATH="$Vs\Common7\IDE;$Vs\Vc7\bin;"+$env:PATH
$env:INCLUDE="$Vs\Vc7\include;$Vs\Vc7\PlatformSDK\include;$Work\vendor\upstream\saco\d3d9\include"
$env:LIB="$Vs\Vc7\lib;$Vs\Vc7\PlatformSDK\lib;$(Join-Path $Root 'private\checkpoint31')"
$Opts=@('/nologo','/c','/Ox','/Og','/Ob1','/Oi','/Ot','/Oy','/MT','/Zp1','/EHsc','/Gy','/DNDEBUG','/DWIN32','/D_WINDOWS')
$Groups=Get-Content (Join-Path $Root 'config\checkpoint31\build.json') -Raw | ConvertFrom-Json
$Records=@()
Push-Location $Work
try {
 foreach($Group in $Groups.groups) {
  if($OnlyGroup -ne "all" -and $Group.name -ne $OnlyGroup) {continue}
  $Objects=@()
  foreach($Unit in $Group.units) {
   $Name=$Group.name+'_'+[IO.Path]::GetFileNameWithoutExtension($Unit)
   $Source=Join-Path $Work $Unit
   & cl.exe @Opts "/Fo$Name.obj" $Unit
   if($LASTEXITCODE -ne 0) {throw "Compile failed: $Unit"}
   Copy-Item "$Name.obj" $Out -Force
   $Objects+="$Name.obj"
   $Records += [ordered]@{group=$Group.name;unit=$Unit;object="$Name.obj";object_sha256=(Get-FileHash "$Name.obj").Hash.ToLowerInvariant()}
  }
  & lib.exe /nologo "/out:$($Group.name).lib" @Objects
  if($LASTEXITCODE -ne 0) {throw 'Archive failed'}
  Copy-Item "$($Group.name).lib" $Out -Force
 }
} finally {Pop-Location}
$Files=@{}
Get-ChildItem (Join-Path $Work 'vendor') -File -Recurse | ForEach-Object {
 $Files[$_.FullName.Substring($Work.Length+1).Replace('\','/')]=(Get-FileHash $_.FullName).Hash.ToLowerInvariant()
}
$Tools=@()
foreach($Name in @('cl.exe','c1xx.dll','c2.dll','link.exe','lib.exe')) {
 $Path="$Vs\Vc7\bin\$Name"
 $Tools += [ordered]@{name=$Name;version=(Get-Item $Path).VersionInfo.FileVersion;sha256=(Get-FileHash $Path).Hash.ToLowerInvariant()}
}
[ordered]@{run=$Run;options=$Opts;units=$Records;sources=$Files;toolchain=$Tools} | ConvertTo-Json -Depth 8 | Set-Content -Encoding UTF8 (Join-Path $Out 'build.json')
Write-Output "PASS libraries $Run"
