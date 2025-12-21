#!/bin/bash
# spdlog 性能测试脚本
# 测试 spdlog 文件日志在不同模式（同步/异步）和不同线程数下的性能

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PERF_TEST="${SCRIPT_DIR}/../build/bin/test_slog_performance"
LOG_FILE="/tmp/perf_test_spdlog.log"

if [ ! -f "$PERF_TEST" ]; then
    echo "❌ Error: test_slog_performance not found!"
    echo "Please build the project first:"
    echo "  cd build && make test_slog_performance"
    exit 1
fi

echo "=========================================="
echo "   spdlog Performance Test Suite"
echo "=========================================="
echo ""

# 清理旧的日志文件
if [ -f "$LOG_FILE" ]; then
    rm -f "$LOG_FILE"
    echo "[Cleanup] Removed old log file: $LOG_FILE"
    echo ""
fi

# ========== 同步模式测试 ==========
echo "┌────────────────────────────────────────────────────────────┐"
echo "│              SYNC MODE TESTS                               │"
echo "└────────────────────────────────────────────────────────────┘"
echo ""

# 同步模式 - 单线程
echo ""
echo "┌────────────────────────────────────────────────────────────┐"
echo "│ [1/6] Sync Mode - Single Thread (100k logs)               │"
echo "└────────────────────────────────────────────────────────────┘"
$PERF_TEST -t spdlog-file -f "$LOG_FILE" -n 100000

# 同步模式 - 4线程
echo ""
echo "┌────────────────────────────────────────────────────────────┐"
echo "│ [2/6] Sync Mode - Multi-Thread (100k logs, 4 threads)     │"
echo "└────────────────────────────────────────────────────────────┘"
$PERF_TEST -t spdlog-file -f "$LOG_FILE" -n 100000 -j 4

# 同步模式 - 8线程
echo ""
echo "┌────────────────────────────────────────────────────────────┐"
echo "│ [3/6] Sync Mode - Multi-Thread (100k logs, 8 threads)     │"
echo "└────────────────────────────────────────────────────────────┘"
$PERF_TEST -t spdlog-file -f "$LOG_FILE" -n 100000 -j 8

# ========== 异步模式测试 ==========
echo ""
echo "┌────────────────────────────────────────────────────────────┐"
echo "│              ASYNC MODE TESTS                              │"
echo "└────────────────────────────────────────────────────────────┘"
echo ""

# 异步模式 - 单线程
echo ""
echo "┌────────────────────────────────────────────────────────────┐"
echo "│ [4/6] Async Mode - Single Thread (100k logs)              │"
echo "└────────────────────────────────────────────────────────────┘"
$PERF_TEST -t spdlog-file -f "$LOG_FILE" -n 100000 -a

# 异步模式 - 4线程
echo ""
echo "┌────────────────────────────────────────────────────────────┐"
echo "│ [5/6] Async Mode - Multi-Thread (100k logs, 4 threads)    │"
echo "└────────────────────────────────────────────────────────────┘"
$PERF_TEST -t spdlog-file -f "$LOG_FILE" -n 100000 -j 4 -a

# 异步模式 - 8线程
echo ""
echo "┌────────────────────────────────────────────────────────────┐"
echo "│ [6/6] Async Mode - Multi-Thread (100k logs, 8 threads)    │"
echo "└────────────────────────────────────────────────────────────┘"
$PERF_TEST -t spdlog-file -f "$LOG_FILE" -n 100000 -j 8 -a

echo ""
echo "=========================================="
echo "   spdlog Test Suite Completed!"
echo "=========================================="