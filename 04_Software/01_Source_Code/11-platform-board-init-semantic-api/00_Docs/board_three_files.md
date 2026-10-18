# Board三文件与本节讲解范围

当前工程按三个实现文件分工：

| 文件 | 职责 | 讲解安排 |
| --- | --- | --- |
| `04_Impl/impl_board/board_resource_config_ecs100.c` 与共用 `board_resource_config.h` | EC-S100板级名称与平台资源对应关系 | 11.1详细讲解 |
| `04_Impl/impl_board/board_control.c` | 背光、马达、复位操作，AHT21 电源请求与 UART 发送入口 | 11.2 |
| `04_Impl/impl_board/board_init.c` | 本地 GPIO 初始状态 | 11.3 |

公共接口分别声明在 `03_Platform/platform_bsp/board_control.h`、`board_init.h`。`board_internal.h` 供实现文件共享内部声明。

## 按板型选择资源

`board_resource_config.h` 保留共用资源声明。EC-S100 的 pinmap、busmap、powermap、irqmap 和板型名称定义在 `board_resource_config_ecs100.c`，Keil 的 `EC-S100-SmartWatch` 目标编译这一个资源配置源文件。

新增实际板型时，按 `board_resource_config_<板型>.c` 命名，按照硬件设计填写资源表，并在对应构建目标中选择一份配置。当前未添加运行时板型识别或 Board 管理器。

`tests/host/fixtures/board_resource_config_board_b.c` 是按键通道交换的主机测试配置，包含测试所需的部分映射。它未加入 Keil，不作为实际板子的完整硬件配置。

## 本节调用示例

`01_App/app_boot/app_boot.c` 的 `app_boot_mcu_probe()` 使用 `g_board_pinmap[BOARD_PIN_ID_USER_KEY1].gpio_id` 选择通道，再调用 `plat_gpio_read()`。

当前配置的USER_KEY1对应CH_2，MCU实现把CH_2映射到PB1。资源配置定义常量数据，读取资源表与调用平台函数是不同步骤。

## 11.2 的 AHT21 电源请求

`BOARD_POWER_DOMAIN_AHT21` 的 `control_bus` 指向 `BOARD_BUS_ID_NORDIC_UART`，
busmap 将它映射到 `PLAT_UART_CH_1`。MCU 实现将此通道绑定到现有 USART1。
`BOARD_BUS_DEVICE_AHT21` 的测量接口仍为 `COMMON_I2C`，两条链路职责不同。

`board_aht21_power_enable(BOARD_SIGNAL_ACTIVE)` 当前校验配置后返回
`PLATFORM_ERR_NOT_SUPPORTED`，Nordic 帧编码与应答协议标注为 TBD。
`board_uart_write()` 已提供基于 busmap 的发送入口；TBD 路径没有调用它。
启动时 `app_boot` 在本地 `board_init()` 成功后请求 AHT 供电，记录待接入警告，
继续本地课程演示。当前没有启动 AHT 测量服务，也没有确认 AHT 已上电。

Motor 操作通过 `BOARD_PIN_ID_MOTOR_EN` 直接解析 pinmap；powermap 不再含 MOTOR。
详见 `00_Docs/lesson11_aht21_uart_tbd.md`。

## 验证入口

工程根目录运行：

```powershell
& './tests/host/run_tests.ps1'
& './tests/host/build_keil.ps1'
```

`verify_board_split.ps1` 检查三个文件的边界、注释与Keil登记。主机测试分别编译和链接真实生产文件，包含交换按键通道后App跟随资源表变化的场景。历史 `verify_scope.ps1` 依赖旧课件工作区的基线快照，不属于本次通用验证入口。

本次Keil编译0错误、2个原有Service枚举警告，未进行烧录或实板验证。
