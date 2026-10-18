param([string]$Compiler = 'gcc', [ValidateSet('all', 'board', 'boot')][string]$Suite = 'all')
$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$out = 'tests/host/artifacts'
Push-Location $root
try {
& (Join-Path $PSScriptRoot 'verify_board_split.ps1')
New-Item -ItemType Directory -Path $out -Force | Out-Null
$includes = @('03_Config', '03_Platform/platform_common', '03_Platform/platform_mcu',
    '03_Platform/platform_bsp', '04_Impl', '04_Impl/impl_board',
    '01_App/app_boot', '01_App/app_system', '02_Service/service_log',
    '02_Service/service_diagnosis', '04_Impl/impl_service_port/log_port') |
    ForEach-Object { '-I' + $_ }
$flags = @('-std=c99', '-Wall', '-Wextra', '-Werror', '-pedantic') + $includes
$board = Join-Path $out 'test_board_control.exe'
if ($Suite -ne 'boot') {
# Relative ASCII arguments avoid MinGW ld's Unicode absolute-output-path issue.
& $Compiler @flags 'tests/host/test_board_control.c' 'tests/host/fake_uart.c' `
    '04_Impl/impl_board/board_init.c' `
    '04_Impl/impl_board/board_control.c' `
    '04_Impl/impl_board/board_resource_config_ecs100.c' -o $board
if ($LASTEXITCODE -ne 0) { throw 'Board test compilation failed.' }
$cases = @('before_init', 'init_order_idempotent', 'polarity_and_delay',
    'invalid_enable', 'init_failure_1', 'init_failure_2', 'init_failure_3',
    'reset_failure_1', 'reset_failure_2', 'action_failure_0',
    'action_failure_1', 'action_failure_2', 'action_failure_3')
foreach ($case in $cases) {
    & $board $case
    if ($LASTEXITCODE -ne 0) { throw "Board test failed: $case" }
}
Write-Output "PASS: $($cases.Count) board scenarios"
foreach ($badPower in 0..3) {
    $validation = Join-Path $out "test_board_validation_$badPower.exe"
    & $Compiler @flags "-DBAD_POWER=$badPower" 'tests/host/test_board_validation.c' `
        'tests/host/fake_uart.c' `
        '04_Impl/impl_board/board_init.c' '04_Impl/impl_board/board_control.c' -o $validation
    if ($LASTEXITCODE -ne 0) { throw 'Mapping validation test compilation failed.' }
    & $validation
    if ($LASTEXITCODE -ne 0) { throw "Mapping validation failed: $badPower" }
}
$mcu = Join-Path $out 'test_mcu_gpio.exe'
& $Compiler @flags '-Itests/host/fakes' '-ICore/Inc' 'tests/host/test_mcu_gpio.c' `
    '04_Impl/impl_mcu/stm32f411_plat_gpio.c' -o $mcu
if ($LASTEXITCODE -ne 0) { throw 'MCU test compilation failed.' }
& $mcu
if ($LASTEXITCODE -ne 0) { throw 'MCU descriptor test failed.' }
Write-Output 'PASS: 4 mapping fixtures and 1 real MCU descriptor suite'
$aht = Join-Path $out 'test_aht21_power.exe'
& $Compiler @flags 'tests/host/test_aht21_power.c' 'tests/host/fake_uart.c' `
    '04_Impl/impl_board/board_init.c' '04_Impl/impl_board/board_control.c' `
    '04_Impl/impl_board/board_resource_config_ecs100.c' -o $aht
if ($LASTEXITCODE -ne 0) { throw 'AHT21 test compilation failed.' }
& $aht
if ($LASTEXITCODE -ne 0) { throw 'AHT21 test failed.' }
foreach ($badUart in 1..6) {
    $validation = Join-Path $out "test_uart_validation_$badUart.exe"
    & $Compiler @flags "-DBAD_UART=$badUart" 'tests/host/test_board_validation.c' `
        'tests/host/fake_uart.c' '04_Impl/impl_board/board_init.c' `
        '04_Impl/impl_board/board_control.c' -o $validation
    if ($LASTEXITCODE -ne 0) { throw 'UART validation compilation failed.' }
    & $validation
    if ($LASTEXITCODE -ne 0) { throw "UART mapping validation failed: $badUart" }
}
Write-Output 'PASS: AHT21 pending request and 6 invalid UART/power mappings'
}
if ($Suite -ne 'board') {
    $boot = Join-Path $out 'test_app_boot.exe'
    $sources = @('tests/host/test_app_boot.c', 'tests/host/fake_uart.c', '01_App/app_boot/app_boot.c',
        '01_App/app_system/app_system.c', '02_Service/service_log/service_log.c',
        '02_Service/service_diagnosis/service_diagnosis.c',
        '03_Platform/platform_common/platform_object.c',
        '03_Platform/platform_common/platform_service.c',
        '03_Platform/platform_common/service_manager.c',
        '03_Platform/platform_common/platform_version.c',
        '04_Impl/impl_board/board_init.c', '04_Impl/impl_board/board_control.c',
        '04_Impl/impl_board/board_resource_config_ecs100.c')
    & $Compiler @flags @sources -o $boot
    if ($LASTEXITCODE -ne 0) { throw 'Boot test compilation failed.' }
    foreach ($stage in 0..3) {
        & $boot $stage
        if ($LASTEXITCODE -ne 0) { throw "Boot test failed: $stage" }
    }
    Write-Output 'PASS: 4 real app_boot integration scenarios'
    $remapped = Join-Path $out 'test_app_boot_remapped.exe'
    $remappedSources = @($sources | Where-Object { $_ -ne '04_Impl/impl_board/board_resource_config_ecs100.c' }) +
        @('tests/host/fixtures/board_resource_config_board_b.c')
    & $Compiler @flags '-DTEST_REMAPPED_KEYS' @remappedSources -o $remapped
    if ($LASTEXITCODE -ne 0) { throw 'Remapped board compilation failed.' }
    & $remapped 0
    if ($LASTEXITCODE -ne 0) { throw 'App did not follow the remapped board key channels.' }
    Write-Output 'PASS: unchanged App follows swapped board key channels'
    $invalidAht = Join-Path $out 'test_app_boot_invalid_aht.exe'
    & $Compiler @flags '-DTEST_REMAPPED_KEYS' '-DTEST_AHT_MAP_INVALID' @remappedSources -o $invalidAht
    if ($LASTEXITCODE -ne 0) { throw 'Invalid AHT fixture compilation failed.' }
    & $invalidAht 4
    if ($LASTEXITCODE -ne 0) { throw 'Invalid AHT mapping must stop startup.' }
    Write-Output 'PASS: AHT21 mapping error stops startup; TBD emits warning without UART TX'
}
}
finally { Pop-Location }
