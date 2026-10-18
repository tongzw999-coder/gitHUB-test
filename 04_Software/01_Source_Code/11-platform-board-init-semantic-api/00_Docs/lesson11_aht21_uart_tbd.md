# 11.2 AHT21 电源控制与 Nordic UART 待接入说明

更新：2026-10-15。依据 WATCH_MOUDLE_V023 原理图第 1、5、6、8 页。

## 两种控制语义

| 对象 | Board 接口 | 配置和控制路径 |
| --- | --- | --- |
| 马达动作 | `board_motor_enable(BOARD_SIGNAL_ACTIVE)` | MOTOR_EN → pinmap → CH_5 → PB3/VIBARTOR_EN |
| AHT21 供电 | `board_aht21_power_enable(BOARD_SIGNAL_ACTIVE)` | AHT21 powermap → NORDIC_UART busmap → CH_1 → USART1 → Nordic 协议 TBD |
| AHT21 测量 | 后续传感器驱动 | AHT21 device map → COMMON_I2C；与电源控制串口分开 |

Nordic 侧 P0.31 对应图纸的 AHT20_PL_CTRL，驱动 U10 SY6280AAC 的 EN，
U10 输出 AHT_VCC。该管脚没有映射为 STM32 本地 GPIO。

## 当前实现

- `board_resource_config_ecs100.c`：AHT21 电源域的 `enable_pin` 为 INVALID，
  `control_bus` 为 NORDIC_UART，默认供电目标为 ON；UART 对应 PLAT_UART_CH_1。
- `board_control.c`：Motor 直接走 pinmap；AHT21 请求校验电源表和 UART 表后返回
  NOT_SUPPORTED。TBD 注释标明后续的帧编码、发送与应答处理位置。
- `board_uart_write()`：现有可编译发送入口，按 busmap 选择 UART，透传缓冲区、
  长度、超时和平台返回值。没有默认测试报文，没有隐含 Nordic 命令。
- `board_init.c`：保留本地三条默认输出；`app_boot` 随后请求 AHT21 上电，
  协议未接入时记录警告并继续当前本地演示，映射错误则停止启动。

当前 AHT21 请求没有发送串口数据，没有确认电源状态，没有运行传感器测量。

## 后续集成事项

Nordic 通信命令、帧格式、应答、超时及串口访问协调仍为 TBD。
双方协议确定后，在标记处调用 `board_uart_write()` 并处理应答；UART 发送成功
与远端命令完成需要分别处理。AHT21 稳定时间和测量初始化依据实际器件资料接入。

## 手动开关的电气边界

SW5 第一组通过 R43 上拉 EN。断开此支路不会必然覆盖 Nordic 的高电平输出。
若 Nordic 为高阻态，还需确认 EN 的确定电平。SY6280 数据手册要求 EN 不悬空。
共享 I2C 上拉接在上游供电，关断 AHT_VCC 后的接口漏电与反向供电需实板验证。

## 验证记录

主机测试覆盖命名状态、Motor 直接控制、AHT 无发送的 TBD 返回、UART 资源校验、
发送参数与错误透传、启动警告与错误分支。Keil 完整构建 0 错误、2 个已有 Service
枚举警告；未烧录、未进行 Nordic 联调或供电测量。
