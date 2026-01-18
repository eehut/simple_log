#!/bin/bash

# Script to generate VSCode and clangd configuration files
# These files are user-specific and can be customized according to personal preferences

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"

# Create .vscode directory if it doesn't exist
mkdir -p "${PROJECT_ROOT}/.vscode"

# Generate .vscode/settings.json
cat > "${PROJECT_ROOT}/.vscode/settings.json" << 'EOF'
{
  // 禁用 clangd 的智能提示
  "C_Cpp.intelliSenseEngine": "disabled",

  // C/C++ extension settings
  "C_Cpp.default.intelliSenseMode": "linux-gcc-x64",
  "C_Cpp.default.compilerPath": "/usr/bin/g++",
  "C_Cpp.default.cppStandard": "c++17",
  "C_Cpp.default.includePath": [
    "/usr/include",
    "/usr/include/x86_64-linux-gnu",
    "${workspaceFolder}/**",
    "${workspaceFolder}/include",
    "${workspaceFolder}/build"
  ],
  "C_Cpp.default.defines": [
    "SLOG_SINK_SPDLOG"
  ],

  // 启用 clangd
  "clangd.path": "clangd",
  "clangd.arguments": [
    "--background-index",
    "--clang-tidy",
    "--completion-style=detailed",
    "--header-insertion=iwyu",
    "--pch-storage=memory",
    "--compile-commands-dir=${workspaceFolder}/build"
  ],

  // 禁用 CMake 插件的自动配置
  "cmake.configureOnOpen": false,

  // 禁用 CMake 插件的自动编译
  "cmake.buildBeforeRun": false,

  // 禁用 CMake 插件的自动配置（当 CMakeLists.txt 改变时）
  "cmake.configureOnEdit": false,

  // 文件关联 - 确保 C++ 文件使用 clangd
  "files.associations": {
    "*.h": "cpp",
    "*.hpp": "cpp",
    "*.cpp": "cpp",
    "*.cc": "cpp",
    "*.cxx": "cpp"
  }
}
EOF

# Generate .clangd configuration file
cat > "${PROJECT_ROOT}/.clangd" << EOF
# Clangd configuration file
# This file configures the clangd language server for better C++ code intelligence

# Path to compile_commands.json (relative to project root)
CompileFlags:
  CompilationDatabase: build

  # Add include paths (using absolute paths)
  Add:
    - -I/usr/include
    - -I/usr/include/x86_64-linux-gnu
    - -I${PROJECT_ROOT}/include
    - -I${PROJECT_ROOT}/build
    # C++ standard
    - -std=c++17
    # Compiler flags
    - -Wall
    - -Wextra
    - -Wpedantic
    # Preprocessor definitions
    - -DSLOG_SINK_SPDLOG

# Index settings
Index:
  Background: Build

# Inlay hints
InlayHints:
  Enabled: Yes
  ParameterNames: Yes
  DeducedTypes: Yes

# Completion settings
Completion:
  AllScopes: Yes

# Diagnostic settings
Diagnostics:
  UnusedIncludes: Strict
#  MissingIncludes: Strict
#  ClangTidy:
#    Add: [performance-*, readability-*, modernize-*]
#    Remove: [modernize-use-trailing-return-type]

# Hover settings
Hover:
  ShowAKA: Yes
EOF

echo "Generated configuration files:"
echo "  - .vscode/settings.json"
echo "  - .clangd"
echo ""
echo "Note: These files are user-specific and can be customized according to your preferences."
echo "Make sure to run 'cmake' in the build directory to generate compile_commands.json for clangd."
