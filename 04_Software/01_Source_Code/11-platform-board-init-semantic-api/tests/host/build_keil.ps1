param([string]$Uv4 = 'C:\Keil_v5\UV4\UV4.exe')
$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$mdk = Join-Path $root 'MDK-ARM'
$log = Join-Path $mdk 'lesson11_build.log'
$started = Get-Date
# -r rebuilds, -j0 hides the IDE; there is deliberately no -f (flash) or -d.
# Start-Process -Wait gives a real exit status rather than launching and returning.
$proc = Start-Process -FilePath $Uv4 -ArgumentList `
    '-r "EC-S100-SmartWatch.uvprojx" -t "EC-S100-SmartWatch" -j0 -o "lesson11_build.log"' `
    -WorkingDirectory $mdk -WindowStyle Hidden -PassThru -Wait
Write-Output "KEIL_EXIT_CODE=$($proc.ExitCode)"
if (-not (Test-Path -LiteralPath $log)) { throw 'Keil did not create its build log.' }
if ((Get-Item -LiteralPath $log).LastWriteTime -lt $started) {
    throw "Keil did not update its build log (exit $($proc.ExitCode))."
}
$text = Get-Content -LiteralPath $log -Raw
Write-Output $text
# uVision exit 1 means warnings. Require fresh output and explicit zero errors
# as well as exit status 0/1; do not mistake a stale AXF/log for a successful build.
if ($proc.ExitCode -notin @(0, 1) -or $text -notmatch '0 Error\(s\), \d+ Warning\(s\)' -or
    (Get-Item -LiteralPath $log).LastWriteTime -lt $started) { throw 'Keil build failed.' }
if ($text -notmatch 'compiling board_resource_config_ecs100\.c') {
    throw 'The EC-S100 resource source was not rebuilt.'
}
foreach ($extension in @('axf', 'hex')) {
    $file = Get-Item -LiteralPath (Join-Path $mdk "EC-S100-SmartWatch/EC-S100-SmartWatch.$extension")
    if ($file.Length -eq 0 -or $file.LastWriteTime -lt $started) { throw "No fresh $extension artifact." }
    Write-Output "FRESH $($file.Name) bytes=$($file.Length)"
}
Write-Output 'PASS real Keil rebuild/link, no flashing or interactive IDE'
