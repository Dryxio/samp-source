param([string]$Run='cp2-a',[string]$ImageBase='0x10000000')
$ErrorActionPreference='Stop'
$ProgressPreference='SilentlyContinue'
if ($Run -notmatch '^cp2-[a-z0-9-]+$') { throw 'Invalid run name' }
$Root=Split-Path $PSScriptRoot -Parent
$Out=Join-Path $Root "build\$Run"
$Work="C:\dev\samp-r5-matching\$Run"
New-Item -ItemType Directory -Force $Out,$Work | Out-Null
Copy-Item (Join-Path $Root 'src') $Work -Recurse -Force
Copy-Item (Join-Path $Root 'src') $Out -Recurse -Force
$Vs='C:\Program Files (x86)\Microsoft Visual Studio .NET 2003'
$env:PATH="$Vs\Common7\IDE;$Vs\Vc7\bin;"+$env:PATH
$env:INCLUDE="$Vs\Vc7\include;$Vs\Vc7\PlatformSDK\include"
$env:LIB="$Vs\Vc7\lib;$Vs\Vc7\PlatformSDK\lib"
$Opts=@('/nologo','/c','/Ox','/Og','/Ob1','/Oi','/Ot','/Oy','/MT','/Zp1','/EHsc','/Gy')
$Units=@('checkpoint2\capsule','checkpoint2\platform','checkpoint2\storage','checkpoint2\buffer','checkpoint2\layer','checkpoint2\switch')
$Records=@()
Push-Location $Work
try {
 foreach($Unit in $Units) {
  $Name=Split-Path $Unit -Leaf
  $Source=Join-Path $Work "src\$Unit.cpp"
  Remove-Item "$Name.obj" -ErrorAction SilentlyContinue
  & cl.exe @Opts "/Fo$Name.obj" $Source
  if($LASTEXITCODE -ne 0) { throw "Compile failed $Unit" }
  Copy-Item "$Name.obj" $Out -Force
  $Records += [ordered]@{unit=$Unit;object="$Name.obj";source_sha256=(Get-FileHash $Source).Hash.ToLowerInvariant();object_sha256=(Get-FileHash "$Name.obj").Hash.ToLowerInvariant()}
 }
 Copy-Item (Join-Path $Root 'config\checkpoint2\exports.def') exports.def -Force
 & link.exe /nologo /dll /incremental:no /opt:ref /opt:noicf "/base:$ImageBase" /map:capsule.map /out:capsule.dll /def:exports.def capsule.obj platform.obj storage.obj buffer.obj layer.obj switch.obj kernel32.lib
 if($LASTEXITCODE -ne 0) { throw 'Link failed' }
 Copy-Item capsule.dll,capsule.map,capsule.lib $Out -Force
 Copy-Item (Join-Path $Root 'tests') $Work -Recurse -Force
 & cl.exe /nologo /MT /EHsc /Zp1 /Fenative.exe tests\checkpoint2\native.cpp capsule.lib
 if($LASTEXITCODE -ne 0) { throw 'Native test compilation failed' }
 $TestOutput = & .\native.exe
 $TestExit=$LASTEXITCODE
 $TestOutput | Write-Output
 $TestOutput | Set-Content -Encoding UTF8 (Join-Path $Out 'native.log')
 Copy-Item native.exe $Out -Force
 Copy-Item tests\checkpoint2\native.cpp (Join-Path $Out "native.cpp") -Force
 if($TestExit -ne 0) { throw "Native test failed: $TestExit" }
} finally {Pop-Location}
$Files=@{}
Get-ChildItem (Join-Path $Work 'src') -File -Recurse | ForEach-Object {
 $Relative=$_.FullName.Substring($Work.Length+1).Replace('\','/')
 $Files[$Relative]=(Get-FileHash $_.FullName).Hash.ToLowerInvariant()
}
$Tools=@()
foreach($Name in @('cl.exe','c1xx.dll','c2.dll','link.exe')) {
 $Path="$Vs\Vc7\bin\$Name"
 $Tools += [ordered]@{name=$Name;version=(Get-Item $Path).VersionInfo.FileVersion;sha256=(Get-FileHash $Path).Hash.ToLowerInvariant()}
}
$Libraries=@{}
foreach($Name in @('libcmt.lib','libcpmt.lib')) {$Libraries[$Name]=(Get-FileHash "$Vs\Vc7\lib\$Name").Hash.ToLowerInvariant()}
[ordered]@{run=$Run;image_base=$ImageBase;options=$Opts;units=$Records;sources=$Files;toolchain=$Tools;libraries=$Libraries;
 dll_sha256=(Get-FileHash (Join-Path $Out 'capsule.dll')).Hash.ToLowerInvariant();map_sha256=(Get-FileHash (Join-Path $Out 'capsule.map')).Hash.ToLowerInvariant();native_exit=$TestExit;
 native_sha256=(Get-FileHash (Join-Path $Out 'native.exe')).Hash.ToLowerInvariant();native_source_sha256=(Get-FileHash (Join-Path $Root 'tests\checkpoint2\native.cpp')).Hash.ToLowerInvariant();
 native_log_sha256=(Get-FileHash (Join-Path $Out 'native.log')).Hash.ToLowerInvariant();exports_sha256=(Get-FileHash (Join-Path $Root 'config\checkpoint2\exports.def')).Hash.ToLowerInvariant()
} | ConvertTo-Json -Depth 6 | Set-Content -Encoding UTF8 (Join-Path $Out 'build.json')
Write-Output "PASS compile $Run"
