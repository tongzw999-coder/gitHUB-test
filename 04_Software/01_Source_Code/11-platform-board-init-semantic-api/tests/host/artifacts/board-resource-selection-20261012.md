# 第11课板型资源文件验证

日期：2026-10-12。

## 改动范围

- EC-S100 资源源文件更名为 `04_Impl/impl_board/board_resource_config_ecs100.c`。
- 共用声明保留在 `board_resource_config.h`，板型名称定义在选中的资源源文件中。
- Keil 的项目文件、项目选项和窗口状态中的资源文件引用已同步。
- 原有换板主机测试数据独立为 `tests/host/fixtures/board_resource_config_board_b.c`。该文件是部分测试配置，未加入 Keil 固件目标。

## 验证结果

| 项目 | 结果 |
| --- | --- |
| Keil 目标资源文件登记 | 仅编译 `board_resource_config_ecs100.c` |
| Keil 文件引用 | 所有登记路径存在 |
| 生产资源表 | 与修改前逐字比较一致 |
| App、board_control、board_init、MCU GPIO 实现 | SHA256 与修改前一致 |
| 主机测试 | 13 个 Board 场景、4 个非法映射场景、1 组 MCU 描述符测试、4 个 App 启动场景和1个换板场景通过 |
| Keil ARMCC 重编译 | 0 Error(s), 2 Warning(s) |
| 链接结果 | App 和 Board control 引用 `board_resource_config_ecs100.o` 中的资源表 |
| 构建产物 | 本次生成 AXF 1381576 字节、HEX 67551 字节 |
| Keil 界面 | 第11课工程已打开，项目树显示带板型后缀的资源源文件 |

编译日志：`MDK-ARM/lesson11_build.log`。

两个警告均来自既有 Service 文件的枚举初始化：`service_log.c:75`、`service_diagnosis.c:82`。本次未修改这些模块。

未进行烧录或实板验证。SPI、I2C、PWM 等资源描述的驱动接入范围保持原状，本次改动仅涉及资源配置的板型归属与构建选择。

## 复现命令

在工程根目录执行：

```powershell
& './tests/host/run_tests.ps1'
& './tests/host/build_keil.ps1'
```

Keil 项目 XML 保留 UTF-8 无 BOM 格式。本机 uVision 5.38 在带 BOM 时返回15，恢复原有无 BOM 编码后重新构建通过。
