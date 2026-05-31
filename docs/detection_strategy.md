# ESL 检测策略

这个仓库的目标不是把检测逻辑埋进业务代码里，而是把可运行示例、构建选项和脚本固定下来，让检查可以重复执行。

## 1. `vector<int>` 只有 0/1 时如何判断和替换

判断方式在 `BinaryVectorStorage::from_vector`：

```cpp
std::all_of(values.begin(), values.end(), [](int value) {
    return value == 0 || value == 1;
});
```

如果全量元素都是 0/1，就把 `vector<int>` 转成动态位图存储：按原长度创建 `DynamicBitset`，逐位 `set(i, values[i] == 1)`。之后访问通过 `value_at()`，调用方不用知道底层是 `vector<int>` 还是 bitset。

本例的 `mask_bits` 来自 XML，运行时会打印：

```text
VECTOR_AUDIT name=mask_bits size=6 storage=dynamic_bitset bits=101101 ones=4
```

工程化替换时不能只看声明，必须扫描全部使用点：如果代码依赖 `int&`、负数、非 0/1 值、连续内存地址或频繁写入，不能直接换。适合替换的是布尔语义、按位查询、计数、mask、enable/disable 表这类数据。

## 2. `vector` 越界如何静态和动态定位

静态检测入口：

```powershell
.\tools\static_check.ps1
```

它会执行 MSVC `/analyze`，并列出 `[]` 下标访问候选点；如果本机安装了 `clang-tidy` 或 `cppcheck`，脚本会自动附加运行。静态工具能发现常量越界、明显循环边界错误、部分空容器路径，但对运行时 XML/外部输入决定的下标无法完整证明。

动态检测入口：

```powershell
.\tools\dynamic_check.ps1
.\tools\dynamic_check.ps1 -RunBoundsProbe
```

Debug 构建启用 `/RTC1`、`/Zi` 和 `_ITERATOR_DEBUG_LEVEL=2`。`-RunBoundsProbe` 会编译一个故意越界的小探针，输出精确文件、行号、函数、vector 名称、index 和 size，例如：

```text
BOUNDS_ERROR vector=probe_values index=5 size=3 at ...\tools\bounds_probe.cpp:10 in main
```

对已有代码，不需要用户手动理解业务再插桩：先跑静态脚本收敛候选点，再跑 Debug/ASan/debug-iterator 配置。如果必须拿到业务语义级名称，可以用统一封装或编译期强制包含方式集中处理，而不是在每个调用点手写日志。

## 3. XML 如何确保和 C++ 一一对应

配置读取在 `load_config_xml()` 中按固定字段表执行：

- 未知字段：直接报错。
- 重复字段：直接报错。
- 缺失字段：直接报错。
- 类型错误或越界：直接报错。
- `input_sequence` 与 `mask_bits` 长度不一致：直接报错。
- `mask_bits` 非 0/1：直接报错。

运行时会打印每个字段的 XML 原始文本、C++ 解析值和 XML 行号：

```text
CONFIG_AUDIT field=threshold line=6 xml="18" cpp="18" status=loaded
```

自动校验入口：

```powershell
.\tools\xml_audit.ps1
```

这个脚本会构建并运行程序，检查所有必需字段都出现 `CONFIG_AUDIT`，并检查 `mask_bits` 确实转成了 dynamic bitset。
