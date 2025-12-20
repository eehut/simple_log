#!/bin/bash
# 快速性能测试脚本（约1分钟）

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PERF_TEST="${SCRIPT_DIR}/../build/bin/test_slog_performance"

if [ ! -f "$PERF_TEST" ]; then
    echo "❌ Error: test_slog_performance not found!"
    echo "Please build the project first:"
    echo "  cd build && make test_slog_performance"
    exit 1
fi

echo "=========================================="
echo "   Quick Performance Test Suite"
echo "=========================================="
echo ""

# Stdout测试
echo "┌─────────────────────────────────────────┐"
echo "│ [1/4] Stdout Single Thread (10k logs)  │"
echo "└─────────────────────────────────────────┘"
$PERF_TEST -t stdout -n 10000

echo ""
echo "┌─────────────────────────────────────────────────┐"
echo "│ [2/4] Stdout Multi-Thread (10k logs, 4 threads)│"
echo "└─────────────────────────────────────────────────┘"
$PERF_TEST -t stdout -n 10000 -j 4

# File测试
echo ""
echo "┌──────────────────────────────────────────────────┐"
echo "│ [3/4] File Single Thread (10k logs, no flush)   │"
echo "└──────────────────────────────────────────────────┘"
$PERF_TEST -t file -n 10000

echo ""
echo "┌────────────────────────────────────────────────────────┐"
echo "│ [4/4] File Multi-Thread (10k logs, 4 threads, no flush)│"
echo "└────────────────────────────────────────────────────────┘"
$PERF_TEST -t file -n 10000 -j 4

echo ""
echo "=========================================="
echo "   Quick Test Suite Completed!"
echo "=========================================="

