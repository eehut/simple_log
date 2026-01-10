# cmake for simple-log library
# author: Liu Chuansen
# email: 179712066@qq.com
# date: 2025-12-04

cmake_minimum_required(VERSION 3.10)
project(simple_log VERSION 0.3 LANGUAGES CXX)

# Set C++ standard
# Default to C++14 for compatibility with older compilers (e.g., gcc 7.5)
# Users can override with -DSLOG_CXX_STANDARD=17 to use C++17 features
# option(SLOG_CXX_STANDARD "C++ standard version (14 or 17)" 14)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
    else()
        add_compile_options(-O2)
    endif()
endif()

# Output directory for binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 配置选项，使用SLOG_前缀，避免冲突
# 是否开启SPDLOG相关sink支持
option(SLOG_SINK_SPDLOG "Build with spdlog support" ON)
# 是否使用外部libfmt库
option(SLOG_EXTERNAL_LIBFMT "Build with external libfmt" ON)
# 是否编译examples
option(SLOG_BUILD_EXAMPLES "Build examples" OFF)
# 是否编译test
option(SLOG_BUILD_TEST "Build test" OFF)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Try to find system fmt library
set(BUILD_WITH_LIBFMT OFF CACHE INTERNAL "Internal: whether to use system fmt")
if (SLOG_EXTERNAL_LIBFMT)
    find_package(fmt QUIET)
    if(fmt_FOUND)
        message(STATUS "Found library fmt: ${fmt_VERSION}")
        set(BUILD_WITH_LIBFMT ON)
    else()
        message(STATUS "Library fmt not found, using bundled fmt")
    endif()
endif()

# Try to find spdlog
set(BUILD_WITH_SPDLOG OFF CACHE INTERNAL "Internal: whether spdlog is available")
if(SLOG_SINK_SPDLOG)
    find_package(spdlog QUIET)
    if(spdlog_FOUND)
        message(STATUS "Found library spdlog: ${spdlog_VERSION}")
        set(BUILD_WITH_SPDLOG ON)
    else()
        message(STATUS "SLOG_SINK_SPDLOG is ON but spdlog not found. Disabling spdlog support.")
    endif()
endif()

# Apply version to slog.hpp during configuration
# This updates SLOG_VERSION_MAJOR, SLOG_VERSION_MINOR, and SLOG_VERSION_STRING
# based on PROJECT_VERSION
execute_process(
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version_apply.sh
            ${CMAKE_CURRENT_SOURCE_DIR}/include/slog/slog.hpp
            ${PROJECT_VERSION}
    RESULT_VARIABLE VERSION_APPLY_RESULT
    OUTPUT_VARIABLE VERSION_APPLY_OUTPUT
    ERROR_VARIABLE VERSION_APPLY_ERROR
)
if(NOT VERSION_APPLY_RESULT EQUAL 0)
    message(WARNING "Failed to apply version to slog.hpp: ${VERSION_APPLY_ERROR}")
else()
    message(STATUS "${VERSION_APPLY_OUTPUT}")
endif()

# Add source
add_subdirectory(src)

# build examples (only if explicitly enabled)
if(SLOG_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Print configuration info
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
if (BUILD_WITH_LIBFMT)
message(STATUS "Build with libfmt: ${fmt_VERSION}")
else()
message(STATUS "Build with bundled fmt")
endif()
message(STATUS "Stdout color output: ${SLOG_STDOUT_COLOR}")
if(BUILD_WITH_SPDLOG)
    message(STATUS "Build with spdlog: ${spdlog_VERSION}")
else()
    message(STATUS "Build without spdlog supported")
endif()
message(STATUS "Build examples: ${SLOG_BUILD_EXAMPLES}")

# Add test subdirectory     
if(SLOG_BUILD_TEST)
    enable_testing()
    add_subdirectory(test)
endif()

# Install headers
install(DIRECTORY include/slog
    DESTINATION include
    FILES_MATCHING
    PATTERN "*.h"
    PATTERN "*.hpp"
)

# Install CMake config files
include(CMakePackageConfigHelpers)

# Create slogConfigVersion.cmake
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/slogConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Create slogConfig.cmake
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/slogConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/slogConfig.cmake"
    INSTALL_DESTINATION lib/cmake/slog
)

# Install CMake config files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/slogConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/slogConfigVersion.cmake"
    DESTINATION lib/cmake/slog
)
