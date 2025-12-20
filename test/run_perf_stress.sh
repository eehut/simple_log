#!/bin/bash
# 压力测试脚本（约30分钟）

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PERF_TEST="${SCRIPT_DIR}/../build/bin/test_slog_performance"

if [ ! -f "$PERF_TEST" ]; then
    echo "❌ Error: test_slog_performance not found!"
    echo "Please build the project first:"
    echo "  cd build && make test_slog_performance"
    exit 1
fi

echo "=========================================="
echo "   Stress Performance Test Suite"
echo "   ⚠️  This will take ~30 minutes"
echo "=========================================="
echo ""

# 百万级测试
echo "┌────────────────────────────────────────┐"
echo "│ [1/6] File Single Thread (1M logs)    │"
echo "└────────────────────────────────────────┘"
$PERF_TEST -t file -n 1000000

echo ""
echo "┌─────────────────────────────────────────────────┐"
echo "│ [2/6] File Multi-Thread (1M logs, 4 threads)   │"
echo "└─────────────────────────────────────────────────┘"
$PERF_TEST -t file -n 1000000 -j 4

echo ""
echo "┌─────────────────────────────────────────────────┐"
echo "│ [3/6] File Multi-Thread (1M logs, 8 threads)   │"
echo "└─────────────────────────────────────────────────┘"
$PERF_TEST -t file -n 1000000 -j 8

echo ""
echo "┌─────────────────────────────────────────────────┐"
echo "│ [4/6] File Multi-Thread (1M logs, 16 threads)  │"
echo "└─────────────────────────────────────────────────┘"
$PERF_TEST -t file -n 1000000 -j 16

# 千万级测试
echo ""
echo "┌────────────────────────────────────────┐"
echo "│ [5/6] File Single Thread (10M logs)   │"
echo "│       ⚠️  This may take 10-15 minutes  │"
echo "└────────────────────────────────────────┘"
$PERF_TEST -t file -n 10000000

echo ""
echo "┌──────────────────────────────────────────────────┐"
echo "│ [6/6] File Multi-Thread (10M logs, 8 threads)   │"
echo "│       ⚠️  This may take 10-15 minutes            │"
echo "└──────────────────────────────────────────────────┘"
$PERF_TEST -t file -n 10000000 -j 8

echo ""
echo "=========================================="
echo "   Stress Test Suite Completed!"
echo "=========================================="

