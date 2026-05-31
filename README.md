# ESL SystemC Clock Demo

这是一个小型但完整的 ESL 建模示例：

- 用 SystemC 风格时钟驱动模块每拍推进。
- 用 C++ 类实现内部阈值累加逻辑。
- 用 XML 提供模型配置。
- 运行时检测 `vector<int>` 是否只有 0/1，并把适合的向量替换为 dynamic bitset 存储。
- 提供静态/动态检测脚本，覆盖 vector 越界候选点、Debug 运行时检查、XML 到 C++ 的字段审计。

默认构建使用仓库内的最小 SystemC 风格时钟内核 `include/esl/mini_systemc.hpp`，这样当前机器没有外部 SystemC 库也能直接运行。业务代码仍按 `sc_core::sc_clock`、`sc_module`、posedge process 的方式组织，后续接入 Accellera SystemC 时可以替换时钟内核适配层。

## 模块逻辑

示例模块是 `ThresholdAccumulatorCore`：

1. XML 给出 `input_sequence`、`mask_bits`、`gain`、`threshold`、`cycles` 和时钟周期。
2. 每个时钟上升沿读取一个输入样本。
3. `mask_bits[index] == 1` 时，累加 `sample * gain`。
4. 累加值达到阈值后输出一次 `fired=true`，并扣除阈值。

## 目录结构

```text
.
├── config/esl_config.xml          # XML 配置
├── docs/detection_strategy.md     # 三个核心问题的检测策略
├── include/esl/                   # 头文件
├── src/                           # 模型源码
└── tools/                         # 构建、运行、检测脚本
```

## 构建和运行

在 PowerShell 中执行：

```powershell
.\tools\run.ps1
```

也可以分开构建：

```powershell
.\tools\build.ps1 -Config Release
.\build\esl_demo.exe .\config\esl_config.xml
```

当前脚本使用本机 Visual Studio MSVC：

```text
D:\Software\Visual Studio\anzhuang\VC\Auxiliary\Build\vcvars64.bat
```

如果你的 Visual Studio 路径不同，脚本会尝试用 `vswhere` 自动查找。

## 检测入口

静态检测：

```powershell
.\tools\static_check.ps1
```

动态检测：

```powershell
.\tools\dynamic_check.ps1
.\tools\dynamic_check.ps1 -RunBoundsProbe
```

XML 到 C++ 一一对应审计：

```powershell
.\tools\xml_audit.ps1
```

更详细的判断和替换策略见 [docs/detection_strategy.md](docs/detection_strategy.md)。
