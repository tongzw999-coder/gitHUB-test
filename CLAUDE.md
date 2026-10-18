# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 仓库用途

本仓库不是代码工程，而是一个可复用的**嵌入式项目目录结构模板**。这里没有构建、检查或测试命令。使用模板时，应整体复制该目录结构，再填入具体嵌入式产品项目的内容。不要臆造工具链或构建系统。

## 顶层目录结构及用途

每个编号目录对应一个工程领域：

- `00_Project_Management/` —— 开发前的项目管理文档，按方法论细分：
  需求导入 QFD、需求约束 Pugh、需求转化 Basic Statics、功能图谱 Function Map、功能风险管控 DFMEA、法规认证（国内/国外）、知识产权（国内/国外）、敏捷开发 Scrum、持续集成与测试 DevOps、产品生产管理 Six Sigma、缺陷管理追踪 Jira。
- `00_Reference/` —— 公共参考文档（`01_Docs`）。
- `01_Function_Map/` —— 功能具体需求清单、流程图等。
- `02_Hardware/` —— `00_Ref/00_HDK` 存放 Hardware Development Kit；`01_Project` 存放原理图、PCB 等硬件资料。
- `03_Firmware/` —— 只存放发布版本固件（`.hex`/`.bin`），不放固件源码。
- `04_Software/` —— `00_Ref/00_SDK` 存放厂商 SDK；`01_Source_Code` 存放应用源码。
- `05_Mechanical/` —— 机械结构、外壳相关文件。
- `06_FCT/` —— 产品发布后配套的上位机工具（功能电路测试）。
- `07_Tools/` —— 开发过程中使用的工具：自动测试、固件加密等。

固件/应用源码应放在 `04_Software/01_Source_Code` 下，而不是 `03_Firmware`（后者只放构建产物）。

## `.gitkeep` 约定

模板通过占位文件 `.gitkeep` 让所有空目录纳入版本控制。目录开始放入真实内容时，删除其 `.gitkeep`；新建空目录时，补一个 `.gitkeep`，否则 git 不会跟踪该目录。

README 中给出的 Git Bash 命令：

```bash
# 删除所有 .gitkeep 占位文件（例如向模板填入实际内容时）
find . -type d -not -path "*/.git*" -exec rm -f {}/.gitkeep \;

# 为所有空目录添加 .gitkeep
find ./ -type d -empty -not -path "./.git/*" -exec touch {}/.gitkeep \;
```

## 编码约定

- 缩进统一使用 4 个空格，不使用 Tab。
- 文档与目录名采用“中文 + 英文方法论后缀”的命名方式；新增管理类目录时保持该模式。

## 参考

项目背景：<https://www.bilibili.com/video/BV1E1421i7wJ/>
