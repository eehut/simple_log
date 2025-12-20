# 性能测试文档

## 概述

`test_performance` 是一个专门用于测试日志库性能的工具，支持测试不同场景下的吞吐量和延迟。

## 编译

```bash
cd build
cmake ..
make test_slog_performance
```

## 使用方法

### 基本语法

```bash
./test_slog_performance [options]
```

### 命令行参数

| 参数 | 长参数 | 说明 | 默认值 |
|------|--------|------|--------|
| `-t` | `--type` | 日志类型：`stdout` 或 `file` | `stdout` |
| `-f` | `--file` | 日志文件路径 | `/tmp/perf_test.log` |
| `-n` | `--count` | 总日志数量 | `100000` |
| `-j` | `--threads` | 多线程测试的线程数 | `1` |
| `-F` | `--flush` | 文件日志立即刷新（影响性能） | `false` |
| `-l` | `--level` | 日志等级 | `info` |
| `-h` | `--help` | 显示帮助信息 | - |

## 测试场景

### 1. Stdout日志性能测试

#### 单线程测试（100k日志）

```bash
./test_slog_performance -t stdout -n 100000
```

**预期输出示例：**
```
========================================
  Performance Test Configuration
========================================
Log Type        : stdout
Total Logs      : 100000
Thread Count    : 1
Log Level       : INFO
========================================

[Running] Single Thread Test...

=== Single Thread Performance ===
Total Logs      : 100000
Thread Count    : 1
Elapsed Time    : 850.25 ms
Throughput      : 117640 logs/sec
Avg Time/Log    : 8.50 μs
```

#### 多线程测试（4线程，100k日志）

```bash
./test_slog_performance -t stdout -n 100000 -j 4
```

**测试说明：**
- 测试4个线程同时使用同一个logger输出日志
- 每个线程输出25k日志（100k / 4）
- 观察多线程竞争情况下的性能

### 2. File日志性能测试

#### 单线程测试（不刷新，100k日志）

```bash
./test_slog_performance -t file -n 100000
```

**特点：**
- 不立即刷新，由操作系统缓冲
- 最高性能模式
- 适合高吞吐场景

**预期输出示例：**
```
========================================
  Performance Test Configuration
========================================
Log Type        : file
Log File        : /tmp/perf_test.log
Flush On Write  : No
Total Logs      : 100000
Thread Count    : 1
Log Level       : INFO
========================================

[Running] Single Thread Test...

=== Single Thread Performance ===
Total Logs      : 100000
Thread Count    : 1
Elapsed Time    : 320.45 ms
Throughput      : 312071 logs/sec
Avg Time/Log    : 3.20 μs

=== File Output Verification ===
File Path       : /tmp/perf_test.log
File Size       : 12458960 bytes
Expected Lines  : 100000
Status          : ✅ File size looks reasonable
```

#### 单线程测试（立即刷新，100k日志）

```bash
./test_slog_performance -t file -n 100000 -F
```

**特点：**
- 每条日志立即刷新到磁盘
- 保证数据安全
- 性能较低，适合关键日志

#### 多线程测试（8线程，不刷新，100k日志）

```bash
./test_slog_performance -t file -n 100000 -j 8
```

**测试说明：**
- 测试8个线程同时写入同一文件
- 验证线程安全性
- 观察多线程竞争下的性能

#### 多线程测试（8线程，立即刷新，100k日志）

```bash
./test_slog_performance -t file -n 100000 -j 8 -F
```

**测试说明：**
- 最严苛的测试场景
- 多线程 + 立即刷新
- 观察性能下降幅度

### 3. 大规模测试

#### 百万级日志测试

```bash
# 单线程，100万日志
./test_slog_performance -t file -n 1000000

# 多线程，100万日志
./test_slog_performance -t file -n 1000000 -j 8
```

#### 千万级日志测试

```bash
# 单线程，1000万日志（注意：需要较长时间和足够磁盘空间）
./test_slog_performance -t file -n 10000000

# 多线程，1000万日志
./test_slog_performance -t file -n 10000000 -j 16
```

## 性能测试套件

### 快速测试套件（约1分钟）

```bash
#!/bin/bash
# 快速性能测试

echo "=== Quick Performance Test Suite ==="

# Stdout测试
echo -e "\n[1/4] Stdout Single Thread (10k logs)"
./test_slog_performance -t stdout -n 10000

echo -e "\n[2/4] Stdout Multi-Thread (10k logs, 4 threads)"
./test_slog_performance -t stdout -n 10000 -j 4

# File测试
echo -e "\n[3/4] File Single Thread (10k logs, no flush)"
./test_slog_performance -t file -n 10000

echo -e "\n[4/4] File Multi-Thread (10k logs, 4 threads, no flush)"
./test_slog_performance -t file -n 10000 -j 4
```

### 标准测试套件（约5分钟）

```bash
#!/bin/bash
# 标准性能测试

echo "=== Standard Performance Test Suite ==="

# Stdout测试
echo -e "\n[1/8] Stdout Single Thread (100k logs)"
./test_slog_performance -t stdout -n 100000

echo -e "\n[2/8] Stdout Multi-Thread (100k logs, 4 threads)"
./test_slog_performance -t stdout -n 100000 -j 4

echo -e "\n[3/8] Stdout Multi-Thread (100k logs, 8 threads)"
./test_slog_performance -t stdout -n 100000 -j 8

# File测试（不刷新）
echo -e "\n[4/8] File Single Thread (100k logs, no flush)"
./test_slog_performance -t file -n 100000

echo -e "\n[5/8] File Multi-Thread (100k logs, 4 threads, no flush)"
./test_slog_performance -t file -n 100000 -j 4

echo -e "\n[6/8] File Multi-Thread (100k logs, 8 threads, no flush)"
./test_slog_performance -t file -n 100000 -j 8

# File测试（立即刷新）
echo -e "\n[7/8] File Single Thread (100k logs, with flush)"
./test_slog_performance -t file -n 100000 -F

echo -e "\n[8/8] File Multi-Thread (100k logs, 4 threads, with flush)"
./test_slog_performance -t file -n 100000 -j 4 -F
```

### 压力测试套件（约30分钟）

```bash
#!/bin/bash
# 压力测试

echo "=== Stress Performance Test Suite ==="

# 百万级测试
echo -e "\n[1/6] File Single Thread (1M logs)"
./test_slog_performance -t file -n 1000000

echo -e "\n[2/6] File Multi-Thread (1M logs, 4 threads)"
./test_slog_performance -t file -n 1000000 -j 4

echo -e "\n[3/6] File Multi-Thread (1M logs, 8 threads)"
./test_slog_performance -t file -n 1000000 -j 8

echo -e "\n[4/6] File Multi-Thread (1M logs, 16 threads)"
./test_slog_performance -t file -n 1000000 -j 16

# 千万级测试
echo -e "\n[5/6] File Single Thread (10M logs)"
./test_slog_performance -t file -n 10000000

echo -e "\n[6/6] File Multi-Thread (10M logs, 8 threads)"
./test_slog_performance -t file -n 10000000 -j 8
```

## 性能指标说明

### Throughput (吞吐量)
- 单位：logs/sec
- 含义：每秒能处理的日志数量
- 越高越好

### Avg Time/Log (平均延迟)
- 单位：μs (微秒)
- 含义：处理一条日志的平均时间
- 越低越好

## 性能参考值

基于现代x86_64服务器（仅供参考）：

### Stdout
- **单线程**：50,000 - 150,000 logs/sec
- **多线程**：30,000 - 100,000 logs/sec（受终端渲染影响）

### File (不刷新)
- **单线程**：200,000 - 500,000 logs/sec
- **多线程（4线程）**：150,000 - 400,000 logs/sec
- **多线程（8线程）**：100,000 - 300,000 logs/sec

### File (立即刷新)
- **单线程**：5,000 - 20,000 logs/sec
- **多线程**：3,000 - 15,000 logs/sec

**注意**：实际性能受以下因素影响：
- CPU性能
- 磁盘I/O性能（特别是SSD vs HDD）
- 操作系统和文件系统
- 系统负载
- 日志内容长度

## 性能优化建议

### 1. 使用文件日志而非stdout
- 文件日志通常比stdout快3-5倍
- stdout受终端渲染速度限制

### 2. 禁用立即刷新（flush_on_write = false）
- 性能提升10-50倍
- 权衡：系统崩溃可能丢失部分日志

### 3. 使用SSD而非HDD
- SSD的随机写入性能远超HDD
- 特别是在多线程场景

### 4. 调整日志等级
- 只记录必要的日志
- 生产环境使用Info或Warning级别

### 5. 批量日志场景优化
- 考虑异步日志（如果库支持）
- 使用日志聚合工具

## 故障排查

### 性能异常低
1. 检查磁盘I/O性能：`iostat -x 1`
2. 检查系统负载：`top` 或 `htop`
3. 检查是否启用了flush_on_write
4. 检查磁盘空间是否充足

### 文件验证失败
1. 检查日志文件权限
2. 检查磁盘空间
3. 手动查看日志文件内容

### 多线程性能异常
1. 检查CPU核心数
2. 检查是否有其他进程占用CPU
3. 尝试调整线程数量

## 示例输出

完整的测试输出示例：

```
========================================
  Performance Test Configuration
========================================
Log Type        : file
Log File        : /tmp/perf_test.log
Flush On Write  : No
Total Logs      : 100000
Thread Count    : 8
Log Level       : INFO
========================================

[Running] Single Thread Test...

=== Single Thread Performance ===
Total Logs      : 100000
Thread Count    : 1
Elapsed Time    : 285.34 ms
Throughput      : 350452 logs/sec
Avg Time/Log    : 2.85 μs

[Running] Multi-Thread Test with 8 threads...

=== Multi-Thread Performance ===
Total Logs      : 100000
Thread Count    : 8
Elapsed Time    : 125.67 ms
Throughput      : 795734 logs/sec
Avg Time/Log    : 1.26 μs

=== File Output Verification ===
File Path       : /tmp/perf_test.log
File Size       : 24917920 bytes
Expected Lines  : 200000
Status          : ✅ File size looks reasonable

========================================
  Performance Test Summary
========================================
Single Thread Performance:
  - Throughput: 350452 logs/sec
  - Avg Time : 2.85 μs/log
Multi-Thread Performance:
  - Throughput: 795734 logs/sec
  - Avg Time : 1.26 μs/log

✅ All tests completed successfully!
```

