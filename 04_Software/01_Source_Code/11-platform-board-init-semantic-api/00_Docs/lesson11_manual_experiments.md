# 第11课手动调用实验

本文件是主动实验说明，不是启动 demo。工程没有自动点亮背光、启动马达或触发屏幕复位的代码。以下实验未上板执行。

## 前提和边界

- 完成 Core 的 GPIO/tick 初始化，并确认 `app_boot_init()` 返回 `PLATFORM_ERR_OK`。
- 在任务上下文手动调用；本课按单线程启动、串行调用设计。多任务必须在外部串行化所有 Board 调用，包括两段延时和复位操作全程。不能在 ISR 中调用 `board_display_reset()`，不能停住 tick 再等待它。
- `board_init()` 不是 MCU GPIO 模式初始化，也不是整机电源管理。它仅发送背光关闭、马达关闭、复位释放三条默认命令，全部成功才 ready。
- Core 旧上电值未改：PA1 背光 HIGH，PB3 马达 LOW，PB10 复位 LOW。背光在 Board 接管完成后才关闭，不能说从上电开始绝对安全，也不能保证没有短暂亮屏。初始复位保持 LOW 后由 init 释放，不等同于 init 主动产生 assert/deassert 脉冲。
- GPIO 二值背光只有全开/全关，没有 PWM 调光。旧 busmap 的 TIMER instance 1、BACKLIGHT_PWM 以及 device channel 1 都是未接入的描述，不能当作真实 TIM1/channel。
- DISPLAY powermap 继承了“RESET 作为 enable_pin”的旧描述，本课只注释限制而不改数据；禁止把它当显示供电开关。没有通用 display power on API。

## 接口与预期

| 手动调用 | 预期命令 | 说明 |
| --- | --- | --- |
| `board_init()` | CH6 LOW → CH5 LOW → CH4 HIGH | 背光关、马达关、释放复位；无延时；重复成功 init 无写入 |
| `board_backlight_on()` | CH6 HIGH | 经 BACKLIGHT powermap → pinmap，高有效 |
| `board_backlight_off()` | CH6 LOW | 二值关闭，不是设置 PWM 占空比 |
| `board_motor_enable(BOARD_SIGNAL_ACTIVE)` | CH5 HIGH | 经 MOTOR_EN pinmap，高有效，控制振动动作 |
| `board_motor_enable(BOARD_SIGNAL_INACTIVE)` | CH5 LOW | 完成实验后显式关闭 |
| `board_display_reset()` | CH4 LOW → delay 10ms → HIGH → delay 10ms | ACTIVE 断言，INACTIVE 释放，低有效由 pinmap 转换 |
| `board_motor_enable(BOARD_SIGNAL_STATE_RESERVED)` | 无写入，返回 PARAM | 仅 ACTIVE、INACTIVE 为有效请求 |
| `board_aht21_power_enable(BOARD_SIGNAL_ACTIVE)` | 校验 powermap → UART busmap，返回 NOT_SUPPORTED | Nordic 协议 TBD，当前不发送字节，不确认供电状态 |

电气观察点只用于接线/测量，不是 Board API 参数：CH4 对应 PB10（`PB10_LCD_RESET_*`），CH5 对应 PB3（`PB3_MOTOR_EN_*`），CH6 对应 PA1（`PA1_LCD_BLACK_*`，保留 BLACK 拼写）。这些实际宏只用于 MCU 实现层。

两个 10ms 沿用 refactor 教学配置，不是器件时序保证。更换屏幕必须按数据手册验证复位有效时间、释放后的就绪时间及电源条件。本课不初始化显示控制器、不发送绘图数据，因此背光亮不代表画面驱动已就绪。

## 单步实验方法

1. 首先记录 `app_boot_init()` 返回值，只在 OK 后继续。失败时停在错误状态，不执行后续业务。
2. 主动调用 `board_backlight_on()`，记录返回值和背光/PA1 状态；然后单独调用 `board_backlight_off()` 并检查返回值。
3. 确认马达供电、安装和工作环境适合测试，再分别调用 `board_motor_enable(BOARD_SIGNAL_ACTIVE)` 与 `board_motor_enable(BOARD_SIGNAL_INACTIVE)`。持续时间由操作者在任务侧按硬件要求控制，不在启动路径添加循环演示。若关闭失败，停止实验并由操作者执行外部安全措施，不把软件返回错误当作马达已经关闭。
4. 在正常运行的任务中单次调用 `board_display_reset()`，用逻辑分析仪观察 PB10 先 LOW、后 HIGH 的次序。不得暂停 CPU 后把墙钟时间当作延时精度证据。
5. 调用非法 enable，确认 PARAM；再次调用合法 off，确认非法参数未破坏 ready。

可用于手动任务侧的最小单动作分发函数如下，仅作为文档示例，不在当前工程启动路径调用，也未加入 Keil 编译：

```c
#include "board_control.h"

typedef enum
{
    LESSON_ACTION_BACKLIGHT_ON,
    LESSON_ACTION_BACKLIGHT_OFF,
    LESSON_ACTION_MOTOR_ON,
    LESSON_ACTION_MOTOR_OFF,
    LESSON_ACTION_DISPLAY_RESET
} lesson_action_t;

platform_err_t lesson11_manual_action(lesson_action_t action)
{
    /* Call from an externally serialized task only, after successful app boot. */
    switch (action)
    {
        case LESSON_ACTION_BACKLIGHT_ON: return board_backlight_on();
        case LESSON_ACTION_BACKLIGHT_OFF: return board_backlight_off();
        case LESSON_ACTION_MOTOR_ON: return board_motor_enable(BOARD_SIGNAL_ACTIVE);
        case LESSON_ACTION_MOTOR_OFF: return board_motor_enable(BOARD_SIGNAL_INACTIVE);
        case LESSON_ACTION_DISPLAY_RESET: return board_display_reset();
        default: return PLATFORM_ERR_PARAM;
    }
}
```

## 错误处理

启动流程在本地 GPIO 默认状态完成后请求 AHT21 供电。由于 Nordic 协议尚未接入，
当前会记录 `AHT21 power control pending: Nordic protocol TBD` 并继续本地演示。
资源映射错误仍会终止启动。`app_boot_init()` 成功不代表 AHT21 已上电或可以开始测量。

所有接口都返回既有 `platform_err_t`，不新增错误码，不额外提供布尔查询接口。

- 未完成初始化或发生 GPIO 写失败后，语义动作返回 `PLATFORM_ERR_NOT_INITIALIZED`。该工程没有 `STATE` 常量，这是统一状态错误。
- GPIO 写失败原样透传，清除 ready。初始化第1/2/3步失败时，之前的写可能已生效、之后的写未执行，不能宣称整体安全。重试 `board_init()` 会从第一步重新执行。
- 复位 assert 失败不延时；deassert 失败不执行第二段延时，复位可能仍保持有效。不要吞掉原错误或宣称屏幕复位成功。
- 参数/映射边界错误使用 `PLATFORM_ERR_PARAM`；保留 GPIO/非 GPIO 信号或未支持的电源域使用 `PLATFORM_ERR_NOT_SUPPORTED`。当前有效配置的三类动作均可执行，不以 unsupported 替代背光。
- `plat_delay_ms()` 返回 void，无法提供可透传的延时失败码；需要正常推进的平台 tick。宿主测试只核验调用顺序和参数，不证明实际物理时间或电气反馈。

## 本机复跑

在本工程根目录 PowerShell 执行：

```powershell
.\tests\host\run_tests.ps1
.\tests\host\verify_board_split.ps1
.\tests\host\build_keil.ps1
```

编译脚本只有 rebuild/link，隐藏 IDE，不下载固件、不启动调试会话。fake GPIO/tick 测试不访问真实硬件。日志、哈希和测试产物位于 `tests/host/artifacts/`，Keil 原始日志位于 `MDK-ARM/lesson11_build.log`。
