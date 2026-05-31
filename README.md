# ESL 建模 (Electronic System Level Modeling)

> 电子系统级（ESL）建模、仿真与验证的实践仓库。

## 简介

ESL（Electronic System Level，电子系统级）是一种在高于 RTL（寄存器传输级）的抽象层次上对
电子系统（SoC / 芯片 / 软硬件系统）进行**建模、仿真和验证**的设计方法学。其核心目标是：

- 在芯片流片之前进行**架构探索**与性能评估；
- 构建**虚拟原型（Virtual Prototype）**，让软件团队提前开始驱动、固件、应用开发；
- 支持**软硬件协同设计（HW/SW Co-design）**；
- 以远高于 RTL 的**仿真速度**完成早期功能验证。

## 为什么需要 ESL

| 维度 | RTL 仿真 | ESL / TLM 仿真 |
| --- | --- | --- |
| 抽象层次 | 信号 / 时钟周期 | 事务 / 函数调用 |
| 仿真速度 | 慢（KHz 级） | 快（MHz~GHz 级，可快数百倍） |
| 可用时间点 | 设计中后期 | 设计早期 |
| 主要用途 | 实现与签核 | 架构探索、虚拟原型、软件早开发 |

## 抽象层次

```
算法级 / 功能级   ←  最抽象
     │
事务级 (TLM)
   ├─ LT  (Loosely Timed,   松散定时)
   └─ AT  (Approximately Timed, 近似定时)
     │
周期精确级 (Cycle Accurate)
     │
RTL              ←  最具体
```

## 涉及的语言与工具

本仓库涉及的实现语言与工具**不限于**以下内容，会随主题扩展持续增补：

- **SystemC** —— 基于 C++ 的系统级建模库（IEEE 1666 标准）
- **C++** —— 核心建模与仿真语言
- **TLM-2.0** —— 事务级建模标准（loosely-timed / approximately-timed）
- 其他可能涉及：SystemVerilog、Python（脚本与验证）、Matlab/Simulink（算法建模）等

## 目录结构（规划）

```
.
├── README.md          # 项目说明（当前文件）
├── src/               # 模型源码（SystemC / C++ 等）
├── tlm/               # TLM 事务级模型
├── tests/             # 测试用例与验证平台
└── docs/              # 设计文档与笔记
```

> 目录会随着内容补充逐步建立，当前仓库以 README 为起点。

## 构建与运行

待模型代码加入后补充具体的构建说明（如 SystemC 环境配置、CMake/Makefile、仿真命令等）。

## 参考资料

- IEEE Std 1666™ — SystemC Language Reference Manual
- Accellera Systems Initiative — TLM-2.0 标准
- 《SystemC: From the Ground Up》
- 《Transaction-Level Modeling with SystemC》

---

*本仓库聚焦 ESL 建模实践，持续更新中。*
