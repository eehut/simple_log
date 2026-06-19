# Simple Log (slog)

一个轻量级、高性能的 C++ 日志库，提供简洁的 API 和丰富的功能特性。

## 构建说明

### CMake 选项

项目当前暴露 4 个主要构建选项：

- `SLOG_SINK_SPDLOG=ON|OFF`
  - 是否尝试启用基于 `spdlog` 的 sink，默认 `ON`
  - 这是“尝试启用”而不是“强制启用”：如果系统里找不到 `spdlog`，配置阶段会自动降级为不启用 `spdlog`
- `SLOG_EXTERNAL_LIBFMT=ON|OFF`
  - 是否优先使用系统 `fmt`，默认 `ON`
  - 如果系统里找不到 `fmt`，会自动回退到仓库内置的 `fmt`
- `SLOG_BUILD_EXAMPLES=ON|OFF`
  - 是否编译 examples，默认 `OFF`
- `SLOG_BUILD_TEST=ON|OFF`
  - 是否编译 tests，默认 `OFF`

推荐始终使用 out-of-tree 构建，例如：

```bash
cmake -S . -B build
cmake --build build -j
```

### 依赖行为

当前工程支持以下依赖策略：

- `fmt`
  - 有系统 `fmt` 时，默认优先使用外部 `fmt`
  - 没有系统 `fmt` 时，自动回退到仓库内置 `fmt`
- `spdlog`
  - 只有在检测到系统 `spdlog` 时才会真正编译 `spdlog` sink
  - 未检测到时，`slog` 主体功能仍可正常构建，只是没有 `spdlog` 相关 API

如果同时启用了外部 `fmt` 和 `spdlog`，构建系统会自动为 `spdlog` 传递 `SPDLOG_FMT_EXTERNAL=1`。
如果启用了 `spdlog`，工程会链接 `spdlog::spdlog_header_only`，以避免把非 PIC 的 `libspdlog.a` 链接进 `libslog.so` 时出现兼容性问题。

### 推荐构建组合

#### 1. 最推荐：系统 `fmt` + 系统 `spdlog`

适合开发机、CI、Ubuntu/Debian 等有完整包管理器的环境。

```bash
sudo apt-get install -y libfmt-dev libspdlog-dev
cmake -S . -B build \
  -DSLOG_SINK_SPDLOG=ON \
  -DSLOG_EXTERNAL_LIBFMT=ON \
  -DSLOG_BUILD_EXAMPLES=ON \
  -DSLOG_BUILD_TEST=ON
cmake --build build -j
```

特点：

- 使用系统 `fmt`
- 启用 `spdlog` sink
- 安装后下游 `find_package(slog)` 也会自动解析 `fmt` 和 `spdlog` 依赖

#### 2. 最稳妥的最小依赖版本：禁用 `spdlog`，使用内置 `fmt`

适合离线环境、裁剪环境、最少三方依赖场景。

```bash
cmake -S . -B build \
  -DSLOG_SINK_SPDLOG=OFF \
  -DSLOG_EXTERNAL_LIBFMT=OFF
cmake --build build -j
```

特点：

- 不依赖系统 `fmt`
- 不依赖系统 `spdlog`
- 只提供 `stdout/file/none` sink

#### 3. 系统 `fmt`，禁用 `spdlog`

适合系统里已有 `fmt`，但不希望引入 `spdlog` 的场景。

```bash
sudo apt-get install -y libfmt-dev
cmake -S . -B build \
  -DSLOG_SINK_SPDLOG=OFF \
  -DSLOG_EXTERNAL_LIBFMT=ON
cmake --build build -j
```

#### 4. 启用 `spdlog`，但 `slog` 自身使用内置 `fmt`

```bash
sudo apt-get install -y libspdlog-dev
cmake -S . -B build \
  -DSLOG_SINK_SPDLOG=ON \
  -DSLOG_EXTERNAL_LIBFMT=OFF \
  -DSLOG_BUILD_EXAMPLES=ON \
  -DSLOG_BUILD_TEST=ON
cmake --build build -j
```

说明：

- 该组合在当前 Ubuntu 24.04 + `spdlog 1.12.0` 环境下已验证可编译通过
- 这里的“内置 `fmt`”仅指 `slog` 自身使用内置 `fmt`
- 系统 `spdlog` 包本身通常仍然依赖系统 `fmt`

### 自动降级行为

以下行为是当前工程刻意支持的：

- `SLOG_EXTERNAL_LIBFMT=ON` 但系统没有 `fmt`
  - 自动回退到内置 `fmt`
- `SLOG_SINK_SPDLOG=ON` 但系统没有 `spdlog`
  - 自动禁用 `spdlog` sink，不影响主库构建

建议关注 CMake 配置输出中的这两行：

```text
-- Build with libfmt: 9.1.0
-- Build with spdlog: 1.12.0
```

或者：

```text
-- Build with bundled fmt
-- Build without spdlog supported
```

### 头文件中的条件编译

`spdlog` 相关 API 只会在构建出的 `slog` target 带有 `BUILD_WITH_SPDLOG` 编译定义时暴露出来：

```cpp
#ifdef BUILD_WITH_SPDLOG
auto logger = slog::make_spdlog_logger("app", slog::LogLevel::Info);
#endif
```

如果你是通过 CMake 的 `target_link_libraries(your_app PRIVATE slog::slog)` 或 `slog::slog_static` 来链接，相关编译定义会从 `slog` target 传播到你的目标。

### 安装与下游使用

安装：

```bash
cmake -S . -B build \
  -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build -j
cmake --install build
```

下游项目使用：

```cmake
find_package(slog REQUIRED CONFIG)
add_executable(your_app main.cpp)
target_link_libraries(your_app PRIVATE slog::slog)
```

如果安装版本是带外部 `fmt` 或 `spdlog` 构建出来的，`slogConfig.cmake` 会自动继续查找这些依赖。

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
- **Spdlog Sink**（可选）：基于 spdlog 的 console 和 file logger，支持同步/异步模式，多线程安全，无缓存

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

文件日志功能支持将日志输出到文件，并可选择同时输出到标准输出（stdout）。内置的 stdout 和 file sink 默认启用。

#### 基本文件日志

```cpp
// 创建文件logger（仅输出到文件）
auto logger = slog::make_file_logger(
    "my_app",                  // logger名称
    "/tmp/my_app.log",         // 日志文件路径
    slog::LogLevel::Info,      // 日志等级
    false,                     // 不输出到stdout（默认false）
    true                       // 每次写入后立即刷新（默认true）
);

// 创建文件logger（同时输出到文件和stdout）
auto logger_with_stdout = slog::make_file_logger(
    "my_app",
    "/tmp/my_app.log",
    slog::LogLevel::Info,
    true,                      // 同时输出到stdout
    true
);

logger->info("Application started");
logger->error("An error occurred");
```

#### 自动轮转的文件日志

```cpp
// 创建自动轮转的文件logger（同时输出到文件和stdout）
auto rotating_logger = slog::make_rotating_file_logger(
    "my_app",                  // logger名称
    "/tmp/my_app.log",         // 日志文件路径
    slog::LogLevel::Info,      // 日志等级
    10 * 1024 * 1024,         // 最大文件大小：10MB
    5,                         // 保留5个旧日志文件
    true,                      // 同时输出到stdout
    true                       // 每次写入后立即刷新
);

// 当文件达到10MB时，会自动轮转：
// my_app.log -> my_app.log.1
// my_app.log.1 -> my_app.log.2
// ... 最多保留5个旧文件
```

#### 多线程安全使用

```cpp
auto logger = slog::make_file_logger("my_app", "/tmp/my_app.log", 
                                     slog::LogLevel::Info, false, true);

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

### Spdlog 集成（可选）

如果编译时启用了 spdlog 支持（`-DSLOG_SINK_SPDLOG=ON`，默认开启），可以使用基于 spdlog 的 logger，提供更高的性能和更丰富的功能。

> **注意**：`SLOG_SINK_SPDLOG` 是用户配置选项。如果系统未安装 spdlog 库，即使设置了 `-DSLOG_SINK_SPDLOG=ON`，编译时也会自动禁用 spdlog 支持。代码中应使用 `#ifdef BUILD_WITH_SPDLOG` 来判断是否启用了 spdlog 功能。

#### 基本使用

```cpp
#ifdef BUILD_WITH_SPDLOG
    // 创建同步 console logger
    auto console_logger = slog::make_spdlog_console_logger("app", slog::LogLevel::Info);
    
    // 创建异步 file logger（性能更好，适合高并发场景）
    auto file_logger = slog::make_spdlog_file_logger("app", "/tmp/app.log", 
                                                      slog::LogLevel::Debug, true);
    
    // 使用方式与标准 slog API 完全一致
    console_logger->info("Hello from spdlog console");
    file_logger->debug("Hello from spdlog file");
#endif
```

#### 同步 vs 异步模式

- **同步模式**（`async=false`，默认）：
  - 日志立即写入，适合调试和低并发场景
  - 线程安全，但每次写入都会阻塞调用线程
  
- **异步模式**（`async=true`）：
  - 日志先放入队列，后台线程异步写入
  - 性能更好，适合高并发场景
  - 队列大小：16384，线程池：1 线程
  - 队列满时阻塞等待，确保不丢失日志

```cpp
#ifdef BUILD_WITH_SPDLOG
    // 同步模式（适合调试）
    auto sync_logger = slog::make_spdlog_console_logger("sync", slog::LogLevel::Info, false);
    
    // 异步模式（适合生产环境）
    auto async_logger = slog::make_spdlog_file_logger("async", "/tmp/async.log", 
                                                       slog::LogLevel::Info, true);
#endif
```

#### 特性

- **多线程安全**：默认使用 spdlog 的 `_mt` 版本（多线程安全）
- **无缓存**：关闭 spdlog 的缓存功能，立即刷新，确保日志及时写入
- **格式统一**：日志格式与 simple_log 保持一致
- **无需依赖**：用户代码无需包含 spdlog 头文件，完全封装在 simple_log 内部

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

#### 从默认 Logger 克隆

更常见的实践是从默认 logger 中克隆一个 logger 对象。特别适用于软件子模块开发时使用，让模块能够在实际运行时，根据主进程的日志设定运行。

```cpp
// 主进程设置默认logger
auto main_logger = slog::make_file_logger("main", "/tmp/main.log", 
                                          slog::LogLevel::Info, true, true);
slog::set_default_logger("main");

// 子模块从默认logger克隆，继承相同的sink配置
auto driver_logger = slog::default_logger()->clone("driver");
auto camera_logger = slog::default_logger()->clone("camera");

// 这些logger会输出到相同的文件，但日志中会显示不同的logger名称
driver_logger->info("Driver initialized");
camera_logger->info("Camera opened");
```

#### 克隆并设置新的日志等级

克隆时可以指定新的日志等级，适用于需要不同日志详细程度的子模块：

```cpp
auto parent_logger = slog::make_stdout_logger("parent", slog::LogLevel::Info);

// 克隆并设置更详细的日志等级
auto debug_logger = parent_logger->clone("debug_module", slog::LogLevel::Debug);

// 克隆并设置更严格的日志等级
auto error_logger = parent_logger->clone("error_module", slog::LogLevel::Error);

debug_logger->debug("This will be shown");   // 会输出
parent_logger->debug("This won't be shown"); // 不会输出（等级是Info）
error_logger->info("This won't be shown");   // 不会输出（等级是Error）
```

**使用场景**：
- **子模块开发**：子模块不需要知道主进程的日志配置，只需从默认logger克隆即可
- **日志分类**：通过不同的logger名称区分不同模块的日志，同时共享相同的输出目标
- **动态配置**：主进程可以动态调整默认logger的配置，所有克隆的logger会自动继承新的配置

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

### 编译配置选项

simple_log 提供以下 CMake 配置选项（使用 `-D` 参数设置）：

#### 外部配置选项（用户可见）

- **`SLOG_SINK_SPDLOG`**（默认：`ON`）
  - 是否启用 spdlog sink 支持
  - 设置为 `ON` 时，会尝试查找系统安装的 spdlog 库
  - 如果找到 spdlog 库，则启用 spdlog 功能；如果未找到，会自动禁用并提示
  - 示例：`cmake -DSLOG_SINK_SPDLOG=ON ..`

- **`SLOG_EXTERNAL_LIBFMT`**（默认：`ON`）
  - 是否使用系统安装的 fmt 库
  - 设置为 `ON` 时，会尝试查找系统安装的 fmt 库
  - 如果找到 fmt 库，则使用系统版本；如果未找到，会使用 simple_log 内置的 fmt
  - 示例：`cmake -DSLOG_EXTERNAL_LIBFMT=OFF ..`（强制使用内置 fmt）

> **⚠️ 重要提示**：使用 spdlog 时，建议同时启用外部 libfmt（`SLOG_EXTERNAL_LIBFMT=ON`）。
> 
> 原因：
> - spdlog 库通常依赖系统安装的 fmt 库
> - 如果启用 spdlog 但关闭外部 fmt（使用内置 fmt），可能会因为版本不兼容或符号冲突导致编译错误
> - 推荐配置：`-DSLOG_SINK_SPDLOG=ON -DSLOG_EXTERNAL_LIBFMT=ON`（这也是默认配置）

#### 配置示例

```bash
# 默认配置（启用 spdlog，使用系统 fmt）
mkdir build && cd build
cmake ..
make

# 禁用 spdlog 支持
cmake -DSLOG_SINK_SPDLOG=OFF ..

# 禁用 spdlog，强制使用内置 fmt
cmake -DSLOG_SINK_SPDLOG=OFF -DSLOG_EXTERNAL_LIBFMT=OFF ..

# 启用 spdlog，强制使用内置 fmt（不推荐，可能导致编译错误）
cmake -DSLOG_SINK_SPDLOG=ON -DSLOG_EXTERNAL_LIBFMT=OFF ..
```

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

// 创建文件 logger（仅输出到文件）
auto file_logger = slog::make_file_logger(
    "name", 
    "/path/to/log.txt",
    slog::LogLevel::Info,
    false,             // 不输出到stdout
    true               // 立即刷新
);

// 创建文件 logger（同时输出到文件和stdout）
auto file_logger_with_stdout = slog::make_file_logger(
    "name",
    "/path/to/log.txt",
    slog::LogLevel::Info,
    true,              // 同时输出到stdout
    true
);

// 创建自动轮转的文件 logger
auto rotating_logger = slog::make_rotating_file_logger(
    "name",
    "/path/to/log.txt",
    slog::LogLevel::Info,
    10 * 1024 * 1024,  // 10MB
    5,                 // 保留5个旧文件
    true,              // 同时输出到stdout
    true               // 立即刷新
);

// 创建静默 logger
auto silent = slog::make_none_logger("name");

// 创建自定义 sink 的 logger
auto custom = slog::make_logger("name", custom_sink);

// 创建 spdlog console logger（同步）
auto spdlog_console = slog::make_spdlog_logger("name", slog::LogLevel::Info, false);

// 创建 spdlog console logger（异步）
auto spdlog_console_async = slog::make_spdlog_logger("name", slog::LogLevel::Info, true);

// 创建 spdlog file logger（同步）
auto spdlog_file = slog::make_spdlog_logger("name", "/path/to/log.txt", slog::LogLevel::Info, false, false);

// 创建 spdlog file logger（异步）
auto spdlog_file_async = slog::make_spdlog_logger("name", "/path/to/log.txt", slog::LogLevel::Info, false, true);

// 创建 spdlog file and console logger（同步）
auto spdlog_file = slog::make_spdlog_logger("name", "/path/to/log.txt", slog::LogLevel::Info, true, false);

// 创建 spdlog file and console logger（异步）
auto spdlog_file_async = slog::make_spdlog_logger("name", "/path/to/log.txt", slog::LogLevel::Info, true, true);

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
LOCAL_TRACE(logger, fmt, ...);
LOCAL_DEBUG(logger, fmt, ...);
LOCAL_INFO(logger, fmt, ...);
LOCAL_WARNING(logger, fmt, ...);
LOCAL_ERROR(logger, fmt, ...);
```

### 最佳实践

#### 动态库中使用日志

在分布式组件开发中，动态库模块的日志输出通常需要在编译时确定，无法灵活适配主程序的日志配置。slog 通过 `get_logger()` 函数解决了这一问题，使得动态库的日志配置能够与主程序保持一致。

**使用方式：**

1. **在动态库模块中**：使用 `get_logger("模块名")` 获取 logger 对象，无需关心 logger 的具体实现。

2. **在主程序中**：有两种配置方式：
   - **主动创建**：在程序启动时创建同名 logger，动态库模块会自动使用该 logger 对象。
   - **使用默认 logger**：创建默认 logger 后，动态库调用 `get_logger()` 时，如果指定的 logger 不存在，会自动从默认 logger 克隆一个同名 logger。

#### 运行时动态修改日志等级

当程序包含多个子模块时，可以在运行时动态调整特定模块的日志等级，修改会立即生效。通过全局日志等级规则，可以精确控制每个模块的日志输出级别。

#### 使用模式匹配预设日志等级

在 logger 创建之前，可以通过预设模式规则来设定未来创建的 logger 的日志等级。模式支持精确匹配和正则表达式匹配。

**示例：** 在 `main()` 函数中设置所有 driver 模块的日志等级为 Debug：

```cpp
slog::set_logger_level("driver*", slog::LogLevel::Debug);
```

这样，后续创建的所有名称匹配 `driver*` 模式的 logger 都会自动使用 Debug 级别。




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
- `test/test_file_sink.cpp` - 文件Sink使用示例

## 许可证

MIT License

Copyright (c) 2023-2025 Liu Chuansen

See [LICENSE](LICENSE) file for details.

## 作者

Liu Chuansen (179712066@qq.com)
