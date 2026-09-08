# Avoid transporting the full unit list through the guest command line.
param([string]$Run,[string]$UnitsFile)
$ErrorActionPreference='Stop'
$Root=Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$Units=(Get-Content $UnitsFile -Raw).Trim()
if(!$Units) {throw 'Empty unit proposal'}
& (Join-Path $Root 'tools\probe_client.ps1') -Run $Run -Units $Units -PoolStrings
