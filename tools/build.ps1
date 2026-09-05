param([string]$Run = 'a')
$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'
if ($Run -notmatch '^[a-zA-Z0-9_-]+$') { throw 'Invalid run identifier' }
$Root = Split-Path $PSScriptRoot -Parent
$VmBuild = "C:\dev\samp-r5-matching\$Run"
$Out = Join-Path $Root "build\$Run"
New-Item -ItemType Directory -Force $VmBuild,$Out,(Join-Path $Out 'src') | Out-Null
$Vs = 'C:\Program Files (x86)\Microsoft Visual Studio .NET 2003'
$env:PATH = "$Vs\Common7\IDE;$Vs\Vc7\bin;" + $env:PATH
$Compiler = "$Vs\Vc7\bin\cl.exe"
$Options = @('/nologo','/c','/Ox','/Og','/Ob1','/Oi','/Ot','/Oy','/MT','/Zp1','/EHsc','/Gy')
$Units = @('pools','actors','camera','game')
$Records = @()
foreach ($Unit in $Units) {
    $Source = Join-Path $Root "src\$Unit.cpp"
    $Copy = Join-Path $VmBuild "$Unit.cpp"
    $Object = Join-Path $VmBuild "$Unit.obj"
    Copy-Item $Source $Copy -Force
    Copy-Item $Source (Join-Path $Out "src\$Unit.cpp") -Force
    Remove-Item $Object -ErrorAction SilentlyContinue
    Push-Location $VmBuild
    try {
        & $Compiler @Options "/Fo$Object" $Copy
        if ($LASTEXITCODE -ne 0) { throw "Compile failed: $Unit ($LASTEXITCODE)" }
    } finally { Pop-Location }
    if (-not (Test-Path $Object)) { throw "Missing fresh object: $Unit" }
    Copy-Item $Object (Join-Path $Out "$Unit.obj") -Force
    $Records += [ordered]@{
        unit=$Unit; source_sha256=(Get-FileHash $Source -Algorithm SHA256).Hash.ToLowerInvariant()
        object_sha256=(Get-FileHash $Object -Algorithm SHA256).Hash.ToLowerInvariant()
    }
}
$Toolchain = @()
foreach ($Name in @('cl.exe','c1xx.dll','c2.dll','link.exe')) {
    $Path = "$Vs\Vc7\bin\$Name"
    $Toolchain += [ordered]@{name=$Name;version=(Get-Item $Path).VersionInfo.FileVersion;
        sha256=(Get-FileHash $Path -Algorithm SHA256).Hash.ToLowerInvariant()}
}
[ordered]@{run=$Run;utc=[DateTime]::UtcNow.ToString('o');options=$Options;
    toolchain=$Toolchain;units=$Records} | ConvertTo-Json -Depth 6 |
    Set-Content -Encoding UTF8 (Join-Path $Out 'build.json')
Write-Output "PASS: compiled $($Units.Count) units into $Out"
