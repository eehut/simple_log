/**
 * @file test_spdlog.cpp
 * @author LiuChuansen (179712066@qq.com)
 * @brief Test spdlog integration
 * @version 0.1
 * @date 2025-12-20
 * 
 * @copyright Copyright (c) 2025
 * 
 */


#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>

#include <slog/slog.hpp>

// Test spdlog console logger (synchronous)
void test_spdlog_console_sync() {
    std::cout << "\n=== Test 1: Spdlog Console Logger (Sync) ===" << std::endl;
    
    auto logger = slog::make_spdlog_logger("test_console_sync", slog::LogLevel::Trace, false);
    
    logger->trace("This is a trace message from spdlog");
    logger->debug("This is a debug message from spdlog");
    logger->info("This is an info message from spdlog");
    logger->warning("This is a warning message from spdlog");
    logger->error("This is an error message from spdlog");
}

// Test spdlog console logger (async)
void test_spdlog_console_async() {
    std::cout << "\n=== Test 2: Spdlog Console Logger (Async) ===" << std::endl;
    
    auto logger = slog::make_spdlog_logger("test_console_async", slog::LogLevel::Info, true);
    
    logger->info("This is an info message from async spdlog");
    logger->warning("This is a warning message from async spdlog");
    logger->error("This is an error message from async spdlog");
    
    // Wait a bit for async logger to flush
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

// Test spdlog file logger (synchronous)
void test_spdlog_file_sync() {
    std::cout << "\n=== Test 3: Spdlog File Logger (Sync) ===" << std::endl;
    
    std::string filepath = "/tmp/test_spdlog_sync.log";
    
    // Remove existing file if any
    struct stat st;
    if (stat(filepath.c_str(), &st) == 0) {
        std::remove(filepath.c_str());
    }
    
    auto logger = slog::make_spdlog_logger("test_file_sync", filepath, slog::LogLevel::Trace, false);
    
    logger->trace("This is a trace message to file");
    logger->debug("This is a debug message to file");
    logger->info("This is an info message to file");
    logger->warning("This is a warning message to file");
    logger->error("This is an error message to file");
    
    // Verify file was created and contains content
    struct stat st2;
    if (stat(filepath.c_str(), &st2) == 0) {
        std::cout << "File created successfully: " << filepath << std::endl;
        std::ifstream file(filepath);
        std::string line;
        int count = 0;
        while (std::getline(file, line)) {
            count++;
        }
        std::cout << "File contains " << count << " log lines" << std::endl;
    } else {
        std::cerr << "ERROR: File was not created!" << std::endl;
    }
}

// Test spdlog file logger (async)
void test_spdlog_file_async() {
    std::cout << "\n=== Test 4: Spdlog File Logger (Async) ===" << std::endl;
    
    std::string filepath = "/tmp/test_spdlog_async.log";
    
    // Remove existing file if any
    struct stat st;
    if (stat(filepath.c_str(), &st) == 0) {
        std::remove(filepath.c_str());
    }
    
    auto logger = slog::make_spdlog_logger("test_file_async", filepath, slog::LogLevel::Info, false, true);
    
    logger->info("This is an info message to async file");
    logger->warning("This is a warning message to async file");
    logger->error("This is an error message to async file");
    
    // Wait a bit for async logger to flush
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Verify file was created and contains content
    struct stat st2;
    if (stat(filepath.c_str(), &st2) == 0) {
        std::cout << "File created successfully: " << filepath << std::endl;
        std::ifstream file(filepath);
        std::string line;
        int count = 0;
        while (std::getline(file, line)) {
            count++;
        }
        std::cout << "File contains " << count << " log lines" << std::endl;
    } else {
        std::cerr << "ERROR: File was not created!" << std::endl;
    }
}

// Test multi-threaded logging
void test_spdlog_multithreaded() {
    std::cout << "\n=== Test 5: Spdlog Multi-threaded Logging ===" << std::endl;
    
    auto logger = slog::make_spdlog_logger("test_mt", slog::LogLevel::Info, false);
    
    std::vector<std::thread> threads;
    const int num_threads = 5;
    const int messages_per_thread = 10;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&logger, i, messages_per_thread]() {
            for (int j = 0; j < messages_per_thread; ++j) {
                logger->info("Thread {} message {}", i, j);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Multi-threaded test completed: " 
              << num_threads * messages_per_thread << " messages logged" << std::endl;
}

// Test async multi-threaded logging
void test_spdlog_async_multithreaded() {
    std::cout << "\n=== Test 6: Spdlog Async Multi-threaded Logging ===" << std::endl;
    
    auto logger = slog::make_spdlog_logger("test_async_mt", slog::LogLevel::Info, true);
    
    std::vector<std::thread> threads;
    const int num_threads = 5;
    const int messages_per_thread = 10;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&logger, i, messages_per_thread]() {
            for (int j = 0; j < messages_per_thread; ++j) {
                logger->info("Async thread {} message {}", i, j);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    // Wait for async logger to flush
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    std::cout << "Async multi-threaded test completed: " 
              << num_threads * messages_per_thread << " messages logged" << std::endl;
}

// Test logger cloning
void test_spdlog_clone() {
    std::cout << "\n=== Test 7: Spdlog Logger Cloning ===" << std::endl;
    
    auto parent = slog::make_spdlog_logger("parent", slog::LogLevel::Info, false);
    parent->info("Message from parent logger");
    
    auto child = parent->clone("child");
    if (child) {
        child->info("Message from cloned child logger");
        std::cout << "Logger cloning successful" << std::endl;
    } else {
        std::cerr << "ERROR: Logger cloning failed!" << std::endl;
    }
}

// Test log level filtering
void test_spdlog_level_filtering() {
    std::cout << "\n=== Test 8: Spdlog Level Filtering ===" << std::endl;
    
    auto logger = slog::make_spdlog_logger("test_filter", slog::LogLevel::Warning, false);
    
    std::cout << "Setting log level to Warning, only Warning and Error should appear:" << std::endl;
    logger->trace("Trace message (should not appear)");
    logger->debug("Debug message (should not appear)");
    logger->info("Info message (should not appear)");
    logger->warning("Warning message (should appear)");
    logger->error("Error message (should appear)");
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Spdlog Integration Test" << std::endl;
    std::cout << "========================================" << std::endl;
    
    try {
        test_spdlog_console_sync();
        test_spdlog_console_async();
        test_spdlog_file_sync();
        test_spdlog_file_async();
        test_spdlog_multithreaded();
        test_spdlog_async_multithreaded();
        test_spdlog_clone();
        test_spdlog_level_filtering();
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "All tests completed successfully!" << std::endl;
        std::cout << "========================================" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}



