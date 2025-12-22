/**
 * @file test_file_sink_fix.cpp
 * @brief 测试文件sink修复：验证多个logger写入同一文件时所有日志都能正确保存
 */

#include <fstream>
#include <thread>
#include <vector>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>

#include <slog/slog.hpp>


const bool to_stdout = false;

void test_multiple_loggers_same_file() 
{
    const std::string test_file = "/tmp/test_file_sink.log";
    
    // 清理旧文件
    struct stat st;
    if (stat(test_file.c_str(), &st) == 0) {
        std::remove(test_file.c_str());
    }
    
    std::cout << "=== Test: Multiple Loggers Writing to Same File ===" << std::endl;
    
    // 创建3个logger，都写入同一个文件
    auto logger1 = slog::make_file_logger("logger1", test_file, slog::LogLevel::Info, to_stdout);
    auto logger2 = slog::make_file_logger("logger2", test_file, slog::LogLevel::Info, to_stdout);
    auto logger3 = slog::make_file_logger("logger3", test_file, slog::LogLevel::Info, to_stdout);
    
    // 每个logger写入10条日志
    std::cout << "Writing logs from logger1..." << std::endl;
    for (int i = 0; i < 10; ++i) {
        logger1->info("Logger1 message {}", i);
    }
    
    std::cout << "Writing logs from logger2..." << std::endl;
    for (int i = 0; i < 10; ++i) {
        logger2->info("Logger2 message {}", i);
    }
    
    std::cout << "Writing logs from logger3..." << std::endl;
    for (int i = 0; i < 10; ++i) {
        logger3->info("Logger3 message {}", i);
    }
    
    // 验证文件中有30行日志
    std::ifstream file(test_file);
    std::string line;
    int line_count = 0;
    
    while (std::getline(file, line)) {
        line_count++;
        std::cout << "Line " << line_count << ": " << line << std::endl;
    }
    
    std::cout << "\n=== Result ===" << std::endl;
    std::cout << "Expected: 30 lines" << std::endl;
    std::cout << "Actual: " << line_count << " lines" << std::endl;
    
    if (line_count == 30) {
        std::cout << "✅ TEST PASSED: All logs are preserved!" << std::endl;
    } else {
        std::cout << "❌ TEST FAILED: Some logs are missing!" << std::endl;
    }
}

void test_multithreaded_logging() 
{
    const std::string test_file = "/tmp/test_file_sink_multithread.log";
    
    // 清理旧文件
    struct stat st;
    if (stat(test_file.c_str(), &st) == 0) {
        std::remove(test_file.c_str());
    }
    
    std::cout << "\n=== Test: Multithreaded Logging ===" << std::endl;
    
    auto logger = slog::make_file_logger("multithread", test_file, slog::LogLevel::Info, to_stdout);
    
    std::vector<std::thread> threads;
    const int num_threads = 5;
    const int logs_per_thread = 20;
    
    // 启动多个线程同时写入
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&logger, i]() {
            for (int j = 0; j < logs_per_thread; ++j) {
                logger->info("Thread {} message {}", i, j);
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }
    
    // 验证文件中有100行日志
    std::ifstream file(test_file);
    std::string line;
    int line_count = 0;
    
    while (std::getline(file, line)) {
        line_count++;
    }
    
    std::cout << "\n=== Result ===" << std::endl;
    std::cout << "Expected: " << (num_threads * logs_per_thread) << " lines" << std::endl;
    std::cout << "Actual: " << line_count << " lines" << std::endl;
    
    if (line_count == num_threads * logs_per_thread) {
        std::cout << "✅ TEST PASSED: All multithreaded logs are preserved!" << std::endl;
    } else {
        std::cout << "❌ TEST FAILED: Some logs are missing!" << std::endl;
    }
}

int main() 
{
    try {
        test_multiple_loggers_same_file();
        test_multithreaded_logging();
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "  All Tests Completed!" << std::endl;
        std::cout << "========================================" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

