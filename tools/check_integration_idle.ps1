# Read-only preflight. Refuse to start over a compiler, linker or another probe.
$ErrorActionPreference='Stop'
$Busy=@(Get-CimInstance Win32_Process | Where-Object {
 $_.ProcessId -ne $PID -and (
  $_.Name -in @('cl.exe','link.exe') -or
  ($_.Name -in @('powershell.exe','pwsh.exe') -and $_.CommandLine -match '(probe_client|link_actor_closure)\.ps1')
 )
})
if($Busy.Count) {
 $Busy | Select-Object ProcessId,Name | Format-Table | Out-String | Write-Output
 throw 'Guest compilation/integration process active; inspect before retrying'
}
Write-Output 'GUEST_COMPILER_AND_LINKER_IDLE'
