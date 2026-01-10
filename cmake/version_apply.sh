#!/bin/bash
# version_apply.sh - 自动更新 slog.hpp 中的版本号定义
# Usage: version_apply.sh <path_to_slog.hpp> <version_string>
# Example: version_apply.sh include/slog/slog.hpp "0.3"

set -euo pipefail

if [ $# -lt 2 ]; then
    echo "Usage: $0 <path_to_slog.hpp> <version_string>" >&2
    echo "Example: $0 include/slog/slog.hpp \"0.3\"" >&2
    exit 1
fi

HPP_FILE="$1"
VERSION_STRING="$2"

# 检查文件是否存在
if [ ! -f "$HPP_FILE" ]; then
    echo "Error: File not found: $HPP_FILE" >&2
    exit 1
fi

# 解析版本号：提取主版本号和次版本号
# 支持格式：0.3, 0.3.0, 1.2.3 等
VERSION_MAJOR=$(echo "$VERSION_STRING" | cut -d. -f1)
VERSION_MINOR=$(echo "$VERSION_STRING" | cut -d. -f2)

# 验证版本号格式
if [ -z "$VERSION_MAJOR" ] || [ -z "$VERSION_MINOR" ]; then
    echo "Error: Invalid version format: $VERSION_STRING (expected format: X.Y)" >&2
    exit 1
fi

# 创建临时文件
TMP_FILE="${HPP_FILE}.tmp"

# 使用 sed 替换版本定义
# 处理 SLOG_VERSION_MAJOR
sed -e "s/^#define[[:space:]]*SLOG_VERSION_MAJOR[[:space:]]*[0-9]*/#define SLOG_VERSION_MAJOR ${VERSION_MAJOR}/" \
    -e "s/^#define[[:space:]]*SLOG_VERSION_MINOR[[:space:]]*[0-9]*/#define SLOG_VERSION_MINOR ${VERSION_MINOR}/" \
    -e "s/^#define[[:space:]]*SLOG_VERSION_STRING[[:space:]]*\"[^\"]*\"/#define SLOG_VERSION_STRING \"${VERSION_STRING}\"/" \
    "$HPP_FILE" > "$TMP_FILE"

# 如果文件内容有变化，替换原文件
# 注意：cmp 在文件不同时返回非零，这是正常的，所以需要临时禁用 set -e
set +e
cmp -s "$HPP_FILE" "$TMP_FILE" >/dev/null 2>&1
FILES_DIFFER=$?
set -e

if [ $FILES_DIFFER -ne 0 ]; then
    mv "$TMP_FILE" "$HPP_FILE"
    echo "Updated version in $HPP_FILE: ${VERSION_STRING} (${VERSION_MAJOR}.${VERSION_MINOR})"
else
    rm -f "$TMP_FILE"
    echo "Version in $HPP_FILE is already up to date: ${VERSION_STRING}"
fi
