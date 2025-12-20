# Simple Log (slog)

一个轻量级、高性能的 C++ 日志库，提供简洁的 API 和丰富的功能特性。

## 特性

### ✨ 核心特性

- **多日志等级支持**：Trace, Debug, Info, Warning, Error
- **格式化日志**：基于 [fmt](https://github.com/fmtlib/fmt) 库，支持 `{}` 风格的格式化字符串
- **编译时格式检查**：使用 `FMT_STRING` 宏在编译时检查格式字符串
- **多 Logger 管理**：全局注册表管理多个 logger，支持按名称查找和管理
- **线程安全**：所有操作都是线程安全的，支持多线程并发使用
- **灵活的 Sink 架构**：可扩展的 Sink 接口，支持自定义输出目标

### 🎯 高级特性

- **全局日志等级规则**：支持精确匹配和正则表达式匹配，可在 logger 创建前设置规则
- **日志抑制功能**：限制特定 tag 的日志输出次数，避免日志泛滥
- **十六进制数据转储**：支持以十六进制格式输出二进制数据
- **Logger 克隆**：可以从一个 sink fork 出多个 logger 对象
- **默认 Logger**：支持设置全局默认 logger，简化日志调用

### 📦 内置 Sink

- **Stdout Sink**：标准输出，支持彩色输出（可选），自动添加时间戳和日志等级
- **File Sink**：文件输出，线程安全，支持文件轮转，多logger写入同一文件
- **None Sink**：静默 sink，不输出任何日志，用于关闭日志输出

## 快速开始

### 基本使用

```cpp
#include <slog/slog.hpp>

int main() {
    // 创建 logger
    auto logger = slog::make_stdout_logger("my_app", slog::LogLevel::Info);
    
    // 输出日志
    logger->info("Application started");
    logger->debug("Debug information: {}", 42);
    logger->warning("Warning: {}", "something happened");
    logger->error("Error occurred: {}", "file not found");
    
    return 0;
}
```

### 格式化日志

```cpp
auto logger = slog::make_stdout_logger("app", slog::LogLevel::Debug);

int value = 42;
double pi = 3.14159;
std::string name = "slog";

logger->info("Integer: {}, Double: {}, String: {}", value, pi, name);
logger->debug("Combined: {} = {}, pi = {}", name, value, pi);
```

### 全局日志函数

```cpp
// 使用默认 logger
slog::info("This is an info message");
slog::debug("Debug value: {}", 42);
slog::error("Error: {}", "something wrong");

// 使用宏（编译时格式检查）
SLOG_INFO("User {} logged in", username);
SLOG_ERROR("Failed to connect: {}", error_code);
```

### 文件日志

```cpp
#include "../src/sink_file.hpp"

// 创建文件logger
auto logger = slog::sink::make_file_logger(
    "my_app",                  // logger名称
    "/tmp/my_app.log",         // 日志文件路径
    slog::LogLevel::Info,      // 日志等级
    10 * 1024 * 1024,         // 最大文件大小：10MB
    5,                         // 保留5个旧日志文件
    true                       // 每次写入后立即刷新
);

logger->info("Application started");
logger->error("An error occurred");

// 多线程安全使用
std::vector<std::thread> threads;
for (int i = 0; i < 10; ++i) {
    threads.emplace_back([&logger, i]() {
        logger->info("Thread {} is logging", i);
    });
}
for (auto& t : threads) {
    t.join();
}
```

详细的文件Sink使用说明请参见 [docs/file_sink_usage.md](docs/file_sink_usage.md)。

## 详细功能

### 日志等级

日志库支持以下日志等级（从低到高）：

- `Trace`：最详细的调试信息
- `Debug`：调试信息
- `Info`：一般信息
- `Warning`：警告信息
- `Error`：错误信息

```cpp
auto logger = slog::make_stdout_logger("app", slog::LogLevel::Warning);

logger->trace("不会输出");
logger->debug("不会输出");
logger->info("不会输出");
logger->warning("会输出");
logger->error("会输出");
```

### Logger 注册表

所有 logger 都会自动注册到全局注册表中，可以通过名称查找和管理：

```cpp
// 创建并注册 logger
auto logger1 = slog::make_stdout_logger("logger1", slog::LogLevel::Info);
auto logger2 = slog::make_stdout_logger("logger2", slog::LogLevel::Debug);

// 检查 logger 是否存在
if (slog::has_logger("logger1")) {
    std::cout << "logger1 exists" << std::endl;
}

// 获取 logger
auto logger = slog::get_logger("logger1");
if (logger) {
    logger->info("Retrieved logger");
}

// 设置默认 logger
slog::set_default_logger("logger1");
auto default_log = slog::default_logger();

// 移除 logger
slog::drop_logger("logger2");
```

### 全局日志等级规则

支持在 logger 创建前设置全局规则，规则会在 logger 创建时自动应用：

#### 精确匹配

```cpp
// 在 logger 创建前设置规则
slog::set_logger_level("my_logger", slog::LogLevel::Debug);

// 创建 logger 时会自动应用规则
auto logger = slog::make_stdout_logger("my_logger", slog::LogLevel::Error);
// logger 的实际等级是 Debug（规则覆盖了默认的 Error）
```

#### 正则表达式匹配

```cpp
// 匹配所有以 "_debug" 结尾的 logger
slog::set_logger_level(".*_debug", slog::LogLevel::Trace);

// 匹配所有以 "camera_" 开头的 logger
slog::set_logger_level("^camera_.*", slog::LogLevel::Info);

// 创建 logger
auto logger1 = slog::make_stdout_logger("test_debug", slog::LogLevel::Error);
// logger1 的实际等级是 Trace（匹配了 ".*_debug" 规则）

auto logger2 = slog::make_stdout_logger("camera_main", slog::LogLevel::Error);
// logger2 的实际等级是 Info（匹配了 "^camera_.*" 规则）
```

#### 优先级

- 精确匹配优先于正则匹配
- 多个正则规则按添加顺序，第一个匹配的规则生效

```cpp
// 设置正则规则
slog::set_logger_level(".*_special", slog::LogLevel::Warning);

// 设置精确匹配规则（优先级更高）
slog::set_logger_level("test_special", slog::LogLevel::Debug);

auto logger = slog::make_stdout_logger("test_special", slog::LogLevel::Error);
// logger 的实际等级是 Debug（精确匹配优先）
```

### 日志抑制功能

限制特定 tag 的日志输出次数，避免日志泛滥：

```cpp
auto logger = slog::make_stdout_logger("app", slog::LogLevel::Debug);

// 限制 "network" tag 最多输出 3 条日志
for (int i = 0; i < 10; ++i) {
    logger->info_limited("network", 3, "Network message {}", i);
    // 只有前 3 条会输出，第 3 条会显示抑制提示
}

// 重置计数器
logger->reset_limited("network");

// 重置后可以再次输出
logger->info_limited("network", 3, "Network message after reset");
```

### 十六进制数据转储

支持以十六进制格式输出二进制数据：

```cpp
auto logger = slog::make_stdout_logger("app", slog::LogLevel::Debug);

// 转储原始数据
uint8_t data[] = {0x00, 0x01, 0x02, 0x03, 0xFF, 0xFE};
logger->dump(slog::LogLevel::Debug, data, sizeof(data), "Raw data: ");

// 转储 vector 数据
std::vector<uint8_t> vec = {0xAA, 0xBB, 0xCC, 0xDD};
logger->dump(slog::LogLevel::Debug, vec, "Vector data: ");
```

### Logger 克隆

可以从一个 sink fork 出多个 logger 对象：

```cpp
auto logger1 = slog::make_stdout_logger("parent", slog::LogLevel::Info);

// 克隆 logger
auto logger2 = logger1->clone("child1");
auto logger3 = logger1->clone("child2");

// logger2 和 logger3 有独立的名称，但共享相同的 sink 配置
logger2->info("Message from child1");
logger3->info("Message from child2");
```

### 动态修改日志等级

```cpp
auto logger = slog::make_stdout_logger("app", slog::LogLevel::Error);

logger->info("不会输出");

// 动态修改等级
logger->set_level(slog::LogLevel::Debug);

logger->debug("现在会输出");
logger->info("现在会输出");
```

### 静默 Logger

使用 None sink 创建不输出任何日志的 logger：

```cpp
auto silent_logger = slog::make_none_logger("silent");

// 这些日志不会输出
silent_logger->info("不会输出");
silent_logger->error("不会输出");
```

## 编译和安装

### 要求

- C++17 或更高版本
- CMake 3.10 或更高版本

### 编译

```bash
mkdir build
cd build
cmake ..
make
```

### 安装

```bash
make install
```

## 线程安全

所有操作都是线程安全的：

- Logger 注册表使用 mutex 保护
- Stdout sink 使用全局 mutex 确保输出不会交错
- 支持多线程并发创建和使用 logger

## 性能特性

- **零开销抽象**：使用模板和内联函数，运行时开销最小
- **编译时格式检查**：使用 `FMT_STRING` 宏在编译时检查格式字符串
- **优化的正则表达式**：使用 `std::regex::optimize` 标志优化正则表达式性能
- **避免不必要的字符串拷贝**：提供 `const char*` 版本的日志函数

## API 参考

### 创建 Logger

```cpp
// 创建标准输出 logger
auto logger = slog::make_stdout_logger("name", slog::LogLevel::Info);

// 创建文件 logger
auto file_logger = slog::sink::make_file_logger(
    "name", 
    "/path/to/log.txt",
    slog::LogLevel::Info,
    10 * 1024 * 1024,  // 10MB
    5,                 // 保留5个旧文件
    true               // 立即刷新
);

// 创建静默 logger
auto silent = slog::make_none_logger("name");

// 创建自定义 sink 的 logger
auto custom = slog::make_logger("name", custom_sink);
```

### Logger 方法

```cpp
// 日志输出
logger->trace(fmt, ...);
logger->debug(fmt, ...);
logger->info(fmt, ...);
logger->warning(fmt, ...);
logger->error(fmt, ...);

// 带限制的日志输出
logger->trace_limited(tag, allowed_num, fmt, ...);
logger->debug_limited(tag, allowed_num, fmt, ...);
logger->info_limited(tag, allowed_num, fmt, ...);
logger->warning_limited(tag, allowed_num, fmt, ...);
logger->error_limited(tag, allowed_num, fmt, ...);

// 数据转储
logger->dump(level, data, size, fmt, ...);
logger->dump(level, vector, fmt, ...);

// 等级管理
logger->set_level(level);
LogLevel level = logger->get_level();
bool allowed = logger->is_allowed(level);

// 其他
std::string name = logger->name();
auto cloned = logger->clone("new_name");
logger->reset_limited(tag);
```

### 全局函数

```cpp
// 日志输出（使用默认 logger）
slog::trace(fmt, ...);
slog::debug(fmt, ...);
slog::info(fmt, ...);
slog::warning(fmt, ...);
slog::error(fmt, ...);

// Logger 管理
slog::register_logger(logger);
slog::get_logger("name");
slog::has_logger("name");
slog::drop_logger("name");
slog::set_default_logger("name");
auto default_log = slog::default_logger();

// 全局日志等级规则
slog::set_logger_level("name", level);
slog::set_logger_level(".*pattern", level);  // 正则表达式

// 工具函数
const char* name = slog::log_level_name(level);
char short_name = slog::log_level_short_name(level);
LogLevel level = slog::log_level_from_name("debug");
```

### 宏

```cpp
// 编译时格式检查
SLOG_TRACE(fmt, ...);
SLOG_DEBUG(fmt, ...);
SLOG_INFO(fmt, ...);
SLOG_WARNING(fmt, ...);
SLOG_ERROR(fmt, ...);

// Logger 专用宏
LOGGER_TRACE(logger, fmt, ...);
LOGGER_DEBUG(logger, fmt, ...);
LOGGER_INFO(logger, fmt, ...);
LOGGER_WARNING(logger, fmt, ...);
LOGGER_ERROR(logger, fmt, ...);
```

## 扩展 Sink

实现自定义 Sink 需要继承 `LoggerSink` 接口：

```cpp
class MySink : public slog::LoggerSink {
public:
    std::shared_ptr<LoggerSink> clone(std::string const& logger_name) const override {
        // 实现克隆逻辑
    }
    
    bool setup(std::string const& logger_name) override {
        // 初始化逻辑
        return true;
    }
    
    void log(LogLevel level, std::string const& msg) override {
        // 输出日志
    }
    
    void set_level(LogLevel level) override {
        level_ = level;
    }
    
    LogLevel get_level() const override {
        return level_;
    }
    
    const char* name() const override {
        return "MySink";
    }
    
private:
    LogLevel level_;
};
```

## 示例

完整示例请参考：
- `test/test_all.cpp` - 所有功能的测试示例
- `examples/example_file_sink.cpp` - 文件Sink使用示例

## 许可证

Copyright (c) 2023-2025 Liu Chuansen

## 作者

Liu Chuansen (179712066@qq.com)

