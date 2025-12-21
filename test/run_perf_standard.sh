#!/bin/bash
# 标准性能测试脚本（约5分钟）

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PERF_TEST="${SCRIPT_DIR}/../build/bin/test_slog_performance"

if [ ! -f "$PERF_TEST" ]; then
    echo "❌ Error: test_slog_performance not found!"
    echo "Please build the project first:"
    echo "  cd build && make test_slog_performance"
    exit 1
fi

echo "=========================================="
echo "   Standard Performance Test Suite"
echo "=========================================="
echo ""

# File测试（不刷新）
echo ""
echo "┌───────────────────────────────────────────────────┐"
echo "│ [1/6] File Single Thread (100k logs, no flush)   │"
echo "└───────────────────────────────────────────────────┘"
$PERF_TEST -t file -n 100000

echo ""
echo "┌──────────────────────────────────────────────────────────┐"
echo "│ [2/6] File Multi-Thread (100k logs, 4 threads, no flush)│"
echo "└──────────────────────────────────────────────────────────┘"
$PERF_TEST -t file -n 100000 -j 4

echo ""
echo "┌──────────────────────────────────────────────────────────┐"
echo "│ [3/6] File Multi-Thread (100k logs, 8 threads, no flush)│"
echo "└──────────────────────────────────────────────────────────┘"
$PERF_TEST -t file -n 100000 -j 8

# File测试（立即刷新）
echo ""
echo "┌──────────────────────────────────────────────────────┐"
echo "│ [4/6] File Single Thread (100k logs, with flush)    │"
echo "└──────────────────────────────────────────────────────┘"
$PERF_TEST -t file -n 100000 -F

echo ""
echo "┌────────────────────────────────────────────────────────────┐"
echo "│ [5/6] File Multi-Thread (100k logs, 4 threads, with flush)│"
echo "└────────────────────────────────────────────────────────────┘"
$PERF_TEST -t file -n 100000 -j 4 -F

echo ""
echo "┌────────────────────────────────────────────────────────────┐"
echo "│ [6/6] File Multi-Thread (100k logs, 8 threads, with flush)│"
echo "└────────────────────────────────────────────────────────────┘"
$PERF_TEST -t file -n 100000 -j 8 -F

echo ""
echo "=========================================="
echo "   Standard Test Suite Completed!"
echo "=========================================="

