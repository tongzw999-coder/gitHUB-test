$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$required = @('04_Impl/impl_board/board_init.c',
    '04_Impl/impl_board/board_control.c',
    '04_Impl/impl_board/board_resource_config_ecs100.c',
    '04_Impl/impl_board/board_internal.h',
    '03_Platform/platform_bsp/board_init.h',
    '03_Platform/platform_bsp/board_control.h',
    '04_Impl/impl_board/board_resource_config.h')
foreach ($file in $required) {
    if (-not (Test-Path -LiteralPath (Join-Path $root $file))) {
        throw "Missing separate Board source/interface: $file"
    }
}
$init = Get-Content -LiteralPath (Join-Path $root $required[0]) -Raw
$control = Get-Content -LiteralPath (Join-Path $root $required[1]) -Raw
if ($init -notmatch 'platform_err_t\s+board_init\s*\(void\)\s*\{') {
    throw 'board_init.c must own the board_init implementation.'
}
if ($control -match 'platform_err_t\s+board_init\s*\(void\)\s*\{') {
    throw 'board_control.c still owns board_init.'
}
foreach ($file in $required) {
    $text = Get-Content -LiteralPath (Join-Path $root $file) -Raw
    if ($text -match '#include\s+"[^"\r\n]+\.c"') {
        throw "Board source must compile independently: $file"
    }
    if ($text -match '\bHAL_|\bGPIO_PIN_|\bGPIO[ABC]\b|#include "main.h"') {
        throw "Physical MCU details leaked into Board source: $file"
    }
    if ($text -notmatch '@file' -or $text -notmatch '@brief' -or
        $text -notmatch '@version' -or $text -notmatch '@author') {
        throw "Missing standard file header: $file"
    }
}
$projectPath = Join-Path $root 'MDK-ARM/EC-S100-SmartWatch.uvprojx'
$bytes = [IO.File]::ReadAllBytes($projectPath)
if ($bytes.Length -ge 3 -and $bytes[0] -eq 0xEF -and
    $bytes[1] -eq 0xBB -and $bytes[2] -eq 0xBF) {
    throw 'Keep Keil project XML in UTF-8 without a BOM for uVision 5.38.'
}
$project = [xml](Get-Content -LiteralPath $projectPath -Raw)
if (Test-Path -LiteralPath (Join-Path $root '04_Impl/impl_board/board_resource_config.c')) {
    throw 'The generic resource implementation must be replaced by the board-specific source.'
}
foreach ($target in @($project.Project.Targets.Target)) {
    $resourceFiles = @($target.Groups.Group.Files.File | Where-Object {
        $_.FileType -eq '1' -and $_.FileName -like 'board_resource_config*.c'
    })
    if ($resourceFiles.Count -ne 1 -or
        $resourceFiles[0].FileName -ne 'board_resource_config_ecs100.c') {
        throw "Target $($target.TargetName) must compile exactly one EC-S100 resource source."
    }
}
$files = @($project.Project.Targets.Target.Groups.Group.Files.File)
foreach ($file in $required) {
    $name = [IO.Path]::GetFileName($file)
    if (@($files | Where-Object FileName -eq $name).Count -ne 1) {
        throw "Keil must register exactly one entry for $name"
    }
}
foreach ($file in $files) {
    if (-not (Test-Path -LiteralPath (Join-Path $root ('MDK-ARM/' + $file.FilePath)))) {
        throw "Keil references a missing file: $($file.FilePath)"
    }
}
$shared = Get-Content -LiteralPath (Join-Path $root '04_Impl/impl_board/board_resource_config.h') -Raw
if ($shared -match '#define\s+BOARD_CFG_NAME\s+"') {
    throw 'The common resource header must not hard-code a board identity.'
}
Write-Output 'PASS Board split: one board-specific configuration per Keil target, common interface, and separate init/control'
