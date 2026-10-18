param([string]$Baseline)
$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
if (-not $Baseline) {
    $courseRoot = [IO.Directory]::GetParent($root).Parent.Parent.FullName
    $lesson10 = Join-Path $courseRoot '10-Platform-board-resource-modeling'
    # Discover the unique baseline by its English project name, without relying
    # on a localized directory literal in this reusable ASCII script.
    $matches = @(Get-ChildItem -LiteralPath $lesson10 -Directory -Recurse |
        Where-Object Name -eq '10-platform-board-resource-modeling')
    if ($matches.Count -ne 1) { throw 'Pass -Baseline with the lesson-10 project path.' }
    $Baseline = $matches[0].FullName
}
$before = Import-Csv -LiteralPath (Join-Path $PSScriptRoot 'artifacts/lesson10-before.csv')
$after = @(Get-ChildItem -LiteralPath $Baseline -File -Recurse -Force | ForEach-Object {
    [pscustomobject]@{Path=$_.FullName.Substring($Baseline.Length+1);
                     SHA256=(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash}
})
$after | Export-Csv -LiteralPath (Join-Path $PSScriptRoot 'artifacts/lesson10-after.csv') -NoTypeInformation
$delta = @(Compare-Object $before $after -Property Path,SHA256)
if ($delta.Count -ne 0) { $delta | Format-Table; throw 'Lesson 10 changed.' }
Write-Output "PASS lesson10 SHA256: $($before.Count) files, 0 changes"

$expectedChanges = @('01_App\app_boot\app_boot.c', '01_App\app_boot\app_boot.h',
    '03_Config\platform_mcu_config.h', '04_Impl\impl_board\board_resource_config.c',
    '04_Impl\impl_mcu\stm32f411_plat_gpio.c', 'MDK-ARM\EC-S100-SmartWatch.uvprojx')
$changed = @()
$preserved = 0
foreach ($entry in $before) {
    # Exclusions used when copying the baseline. Fresh lesson-11 compiler output
    # is not source churn and must not be compared with lesson-10 build output.
    if ($entry.Path -match '(^|\\)(\.git|Objects|Listings)(\\|$)' -or
        $entry.Path.StartsWith('MDK-ARM\EC-S100-SmartWatch\') -or
        $entry.Path -match '\.(uvoptx|o|obj|d|crf|axf|elf|hex|bin|map|lst|lnp|sct|htm|html|log|bak)$' -or
        $entry.Path -match '\.uvguix\.' -or
        $entry.Path -match '(^|\\)(JLinkLog.txt|JLinkSettings.ini)$') { continue }
    $path = Join-Path $root $entry.Path
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "Missing baseline source: $($entry.Path)" }
    if ((Get-FileHash -LiteralPath $path).Hash -ne $entry.SHA256) { $changed += $entry.Path }
    else { ++$preserved }
}
if (@(Compare-Object $expectedChanges $changed).Count -ne 0) {
    $changed
    throw 'Unexpected baseline code/config changes.'
}
Write-Output "PASS baseline copy: $preserved unchanged retained files; exactly 6 modified files"
$changed | ForEach-Object { Write-Output "  MODIFIED $_" }

$old = [xml](Get-Content -LiteralPath (Join-Path $Baseline 'MDK-ARM/EC-S100-SmartWatch.uvprojx') -Raw)
$new = [xml](Get-Content -LiteralPath (Join-Path $root 'MDK-ARM/EC-S100-SmartWatch.uvprojx') -Raw)
$oldGroups = @($old.Project.Targets.Target.Groups.Group.GroupName)
$newGroups = @($new.Project.Targets.Target.Groups.Group.GroupName | Where-Object { $_ -ne '03_Platform/platform_bsp' })
if ([string]::Join('|', $oldGroups) -ne [string]::Join('|', $newGroups)) { throw 'Existing group order changed.' }
if ($old.Project.Targets.Target.TargetOption.TargetArmAds.Cads.VariousControls.IncludePath -ne
    $new.Project.Targets.Target.TargetOption.TargetArmAds.Cads.VariousControls.IncludePath) { throw 'Include list changed.' }
$projectFiles = @($new.Project.Targets.Target.Groups.Group.Files.File)
foreach ($file in $projectFiles) {
    if (-not (Test-Path -LiteralPath (Join-Path (Join-Path $root 'MDK-ARM') $file.FilePath))) {
        throw "Missing project file: $($file.FilePath)"
    }
}
foreach ($name in @('board_control.c', 'board_control.h')) {
    if (@($projectFiles | Where-Object FileName -eq $name).Count -ne 1) { throw "Incorrect project entry: $name" }
}
$boardFiles = @((Join-Path $root '04_Impl/impl_board/board_control.c'),
                (Join-Path $root '03_Platform/platform_bsp/board_control.h'))
if (Select-String -LiteralPath $boardFiles -Pattern 'HAL_|GPIO_PIN_|GPIO[ABC]\b|#include "main.h"') {
    throw 'Physical MCU implementation leaked into Board API/code.'
}
$stateFiles = @(Get-ChildItem -LiteralPath $root -Force -Recurse | Where-Object {
    $_.Name -eq '.git' -or $_.Name -like '*.uvoptx' -or $_.Name -like '*.uvguix.*'
})
if ($stateFiles.Count -ne 0) { throw 'Copied VCS or user Keil state found.' }
Write-Output 'PASS Keil XML: existing group/include order preserved, all files exist, Board entries unique'
Write-Output 'PASS Board boundary: no HAL/physical pins; no .git or Keil user option state'
