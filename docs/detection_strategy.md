# ESL 检测策略

这个仓库演示的是一套“外部 skill + 项目少量审计点”的检测闭环。任意 C++ 程序的所有越界路径无法被静态分析完全证明，动态检测也只能覆盖实际运行到的路径；因此本工程把结果分为：

- 已静态证明。
- 已动态证明。
- 已超时捕获。
- 有风险但当前外部证据不足，需要 sanitizer、debug iterator、调试器、覆盖更多用例或内部审计点。

## 1. vector 越界如何检测

### 静态阶段

入口：

```powershell
.\tools\static_check.ps1
```

它做三件事：

- MSVC `/analyze`。
- XML 静态 schema 比对。
- 扫描 C/C++ 里的下标访问并输出 `STATIC_BOUNDS_CANDIDATE`。

示例风险点：

```cpp
(void)input_sequence_[hazard_index];
```

这是故意保留的裸 `operator[]` 越界风险。静态阶段不能知道所有运行时 index，但可以把这种候选点收敛出来。

### 动态阶段

入口：

```powershell
.\tools\dynamic_check.ps1
```

它会构建 Debug 版本，然后运行四个 case：

- `safe`：正常 XML，必须成功。
- `checked_oob`：`checked_at()` 故意越界，必须输出 `BOUNDS_ERROR`。
- `unchecked_oob`：裸 `operator[]` 越界，Debug runtime 可能报错，也可能弹调试器/卡住；脚本必须用 timeout 捕获。
- `hang`：故意不结束，脚本必须输出 `TIMEOUT_DETECTED`。

确定性越界输出示例：

```text
ERROR BOUNDS_ERROR vector=input_sequence/hazard_checked index=101 size=6 at src\filter_core.cpp:36 in ThresholdAccumulatorCore::tick
```

这个结果包含 vector 名、index、size、文件、行号和函数。

### 覆盖边界

如果某个越界路径没有被测试跑到，动态检测无法证明它不存在。  
如果裸 `operator[]` 在 Release 下读到了未定义内存但没有崩，运行结束后也可能查不出来。  
工程上必须组合：

- 静态候选扫描。
- Debug iterator / sanitizer。
- timeout watchdog。
- 覆盖危险 XML 和测试用例。
- 对关键访问使用 `checked_at()` 或由 skill 临时插桩。

## 2. XML 如何确保和 C++ 一一对应

### 静态对应

C++ 侧 schema 在：

```text
include/esl/config_schema.hpp
```

字段清单由 `ESL_CONFIG_FIELD_LIST` 定义。静态检查入口：

```powershell
.\tools\xml_static_check.ps1
```

它会把 `config/*.xml` 的 tag 和 C++ schema 比较：

- XML 缺 C++ 字段：报 `XML_SCHEMA_MISMATCH missing_cpp_field=...`
- XML 多字段：报 `XML_SCHEMA_MISMATCH extra_xml_field=...`
- XML 重复字段：报 `XML_SCHEMA_MISMATCH duplicate_field=...`
- 全部匹配：输出 `XML_STATIC_OK`

### 动态赋值证明

动态入口：

```powershell
.\tools\xml_audit.ps1
```

程序读取 XML 后会打印：

```text
CONFIG_AUDIT field=hazard_read_offset line=13 xml="100" cpp="100" status=loaded
```

这证明 XML 原始文本、C++ 解析值、XML 行号是一致的。skill 会解析 `CONFIG_AUDIT`，确认“字段确实被 C++ 得到”。

## 3. vector<int> 只有 0/1 时如何替换 dynamic_bitset

判断方式在 `BinaryVectorStorage::from_vector`：

```cpp
std::all_of(values.begin(), values.end(), [](int value) {
    return value == 0 || value == 1;
});
```

如果全量元素都是 0/1，就把 `vector<int>` 转成动态位图存储：按原长度创建 `DynamicBitset`，逐位 `set(i, values[i] == 1)`。运行时输出：

```text
VECTOR_AUDIT name=mask_bits size=6 storage=dynamic_bitset bits=101101 ones=4
```

这证明 `mask_bits` 来自 XML，且确实在运行时被转换成 bitset 存储。

## 4. Skill 何时运行

skill 是外部检查员，不能被 C++ 直接调用。推荐运行时机：

- 改了 XML 后。
- 改了配置结构后。
- 改了 vector / index / loop / clocked process 后。
- 每次提交前。
- 夜间回归时。
- 仿真崩溃、卡住或结果异常后。

运行方式：

```powershell
powershell -ExecutionPolicy Bypass -File C:\Users\xuche\.codex\skills\esl-runtime-check\scripts\run_esl_runtime_check.ps1 -Project D:\AI\ai_git_by_codex
```

它会调用项目脚本，收集 `STATIC_BOUNDS_CANDIDATE`、`BOUNDS_ERROR`、`TIMEOUT_DETECTED`、`XML_STATIC_OK`、`CONFIG_AUDIT` 等证据并生成报告。
