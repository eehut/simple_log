/**
 * @file test_performance.cpp
 * @author LiuChuansen (179712066@qq.com)
 * @brief 日志性能测试程序
 * @version 0.1
 * @date 2025-12-19
 * 
 * 测试日志库在不同场景下的性能表现
 */


#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <cstring>


#include <slog/slog.hpp>

#ifdef BUILD_WITH_SPDLOG
#define ENABLE_SPDLOG 1
#endif

// 测试配置
struct TestConfig 
{
    std::string log_type = "stdout";    // stdout 或 file 或 spdlog-file 或 spdlog-console
    std::string log_file = "/tmp/perf_test.log";
    int log_count = 100000;             // 日志总数
    int thread_count = 1;               // 线程数量
    bool flush_on_write = false;        // 是否立即刷新(file专用)
#ifdef ENABLE_SPDLOG
    bool spdlog = false;                // 是否使用spdlog
    bool async = false;                // 是否使用异步模式(spdlog专用)
#endif 
    slog::LogLevel level = slog::LogLevel::Info;
    
    void print() const 
    {
        std::cout << "========================================" << std::endl;
        std::cout << "  Performance Test Configuration" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Log Type        : " << log_type << std::endl;
        if (log_type == "file") {
            std::cout << "Log File        : " << log_file << std::endl;
            std::cout << "Flush On Write  : " << (flush_on_write ? "Yes" : "No") << std::endl;
        }
#ifdef ENABLE_SPDLOG
        if (log_type == "spdlog-file") {
            std::cout << "Log File        : " << log_file << std::endl;
        }
#endif
#ifdef ENABLE_SPDLOG
        if (log_type == "spdlog-file" || log_type == "spdlog-console") {
            std::cout << "Async          : " << (async ? "Yes" : "No") << std::endl;
        }
#endif 
        std::cout << "Total Logs      : " << log_count << std::endl;
        std::cout << "Thread Count    : " << thread_count << std::endl;
        std::cout << "Log Level       : " << slog::log_level_name(level) << std::endl;
        std::cout << "========================================" << std::endl;
    }
};

// 测试结果
struct TestResult 
{
    std::string test_name;
    int total_logs;
    int thread_count;
    double elapsed_ms;
    double logs_per_second;
    double us_per_log;
    
    void print() const {
        std::cout << "\n=== " << test_name << " ===" << std::endl;
        std::cout << "Total Logs      : " << total_logs << std::endl;
        std::cout << "Thread Count    : " << thread_count << std::endl;
        std::cout << "Elapsed Time    : " << std::fixed << std::setprecision(2) 
                  << elapsed_ms << " ms" << std::endl;
        std::cout << "Throughput      : " << std::fixed << std::setprecision(0) 
                  << logs_per_second << " logs/sec" << std::endl;
        std::cout << "Avg Time/Log    : " << std::fixed << std::setprecision(2) 
                  << us_per_log << " μs" << std::endl;
    }
};

/**
 * @brief 创建logger
 * @param config 测试配置
 * @param clean_file 是否清理旧文件（默认false，避免删除之前测试的数据）
 */
std::shared_ptr<slog::Logger> create_logger(TestConfig const & config, bool clean_file = false)
{
    if (config.log_type == "file") 
    {
        // 只在明确指定时才清理旧的日志文件
        if (clean_file && std::filesystem::exists(config.log_file)) {
            std::filesystem::remove(config.log_file);
        }
        
        // 创建文件logger，不轮转（max_file_size = 0）
        return slog::make_file_logger(
            "perf_test",
            config.log_file,
            config.level,
            false,
            config.flush_on_write
        );
    } 
    #ifdef ENABLE_SPDLOG
    else if (config.log_type == "spdlog-file") 
    {
        // 只在明确指定时才清理旧的日志文件
        if (clean_file && std::filesystem::exists(config.log_file)) {
            std::filesystem::remove(config.log_file);
        }
        
        // 创建文件logger，不轮转（max_file_size = 0）
        return slog::make_spdlog_logger(
            "perf_test",
            config.log_file,
            config.level,
            false,
            config.async
        );
    }
    else if (config.log_type == "spdlog-console") 
    {
        return slog::make_spdlog_logger(
            "perf_test",
            config.level,
            config.async
        );
    }
    #endif // ENABLE_SPDLOG
    else 
    {
        // 创建stdout logger
        return slog::make_stdout_logger("perf_test", config.level);
    }
}

/**
 * @brief 单线程性能测试
 */
TestResult test_single_thread(TestConfig const & config)
{
    auto logger = create_logger(config);
    
    std::cout << "\n[Running] Single Thread Test..." << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // 输出指定数量的日志
    for (int i = 0; i < config.log_count; ++i) {
        logger->info("Single thread performance test message {}, with some additional text to simulate real log content", i);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    TestResult result;
    result.test_name = "Single Thread Performance";
    result.total_logs = config.log_count;
    result.thread_count = 1;
    result.elapsed_ms = duration / 1000.0;
    result.logs_per_second = (config.log_count * 1000000.0) / duration;
    result.us_per_log = static_cast<double>(duration) / config.log_count;
    
    return result;
}

/**
 * @brief 多线程性能测试
 */
TestResult test_multi_thread(TestConfig const & config)
{
    auto logger = create_logger(config);
    
    std::cout << "\n[Running] Multi-Thread Test with " << config.thread_count << " threads..." << std::endl;
    
    int logs_per_thread = config.log_count / config.thread_count;
    std::vector<std::thread> threads;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // 启动多个线程
    for (int t = 0; t < config.thread_count; ++t) {
        threads.emplace_back([&logger, t, logs_per_thread]() {
            for (int i = 0; i < logs_per_thread; ++i) {
                logger->info("Multi-thread test from thread {} message {}, with some additional content", t, i);
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    int actual_logs = logs_per_thread * config.thread_count;
    
    TestResult result;
    result.test_name = "Multi-Thread Performance";
    result.total_logs = actual_logs;
    result.thread_count = config.thread_count;
    result.elapsed_ms = duration / 1000.0;
    result.logs_per_second = (actual_logs * 1000000.0) / duration;
    result.us_per_log = static_cast<double>(duration) / actual_logs;
    
    return result;
}

/**
 * @brief 打印使用说明
 */
void print_usage(const char* prog_name) 
{
    std::cout << "Usage: " << prog_name << " [options]\n\n"
              << "Options:\n"
              << "  -t, --type <type>        Log type: stdout,file,spdlog-file,spdlog-console (default: stdout)\n"
              << "  -f, --file <path>        Log file path (default: /tmp/perf_test.log)\n"
              << "  -n, --count <number>     Total number of logs (default: 100000)\n"
              << "  -j, --threads <number>   Number of threads for multi-thread test (default: 1)\n"
              << "  -F, --flush              Enable flush on write for file logger (default: off)\n"
#ifdef ENABLE_SPDLOG
              << "  -a, --async              Enable async mode for spdlog (default: off)\n"
#endif 
              << "  -l, --level <level>      Log level: trace/debug/info/warning/error (default: info)\n"
              << "  -h, --help               Show this help message\n\n"
              << "Examples:\n"
              << "  # Test stdout with 100k logs, single thread\n"
              << "  " << prog_name << " -t stdout -n 100000\n\n"
              << "  # Test file with 100k logs, 4 threads, no flush\n"
              << "  " << prog_name << " -t file -n 100000 -j 4\n\n"
              << "  # Test file with 100k logs, 8 threads, with flush\n"
              << "  " << prog_name << " -t file -n 100000 -j 8 -F\n\n";
}

/**
 * @brief 解析命令行参数
 */
bool parse_args(int argc, char* argv[], TestConfig& config) 
{
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return false;
        }
        else if (arg == "-t" || arg == "--type") {
            if (i + 1 < argc) {
                config.log_type = argv[++i];
#ifdef ENABLE_SPDLOG
                if (config.log_type != "stdout" && config.log_type != "file" 
                    && config.log_type != "spdlog-file" && config.log_type != "spdlog-console") {
                    std::cerr << "Error: Invalid log type '" << config.log_type 
                              << "'. Must be 'stdout', 'file', 'spdlog-file', or 'spdlog-console'." << std::endl;
                    return false;
                }
#else
                if (config.log_type != "stdout" && config.log_type != "file") {
                    std::cerr << "Error: Invalid log type '" << config.log_type 
                              << "'. Must be 'stdout' or 'file'." << std::endl;
                    return false;
                }
#endif
            } else {
                std::cerr << "Error: --type requires an argument" << std::endl;
                return false;
            }
        }
        else if (arg == "-f" || arg == "--file") {
            if (i + 1 < argc) {
                config.log_file = argv[++i];
            } else {
                std::cerr << "Error: --file requires an argument" << std::endl;
                return false;
            }
        }
        else if (arg == "-n" || arg == "--count") {
            if (i + 1 < argc) {
                config.log_count = std::atoi(argv[++i]);
                if (config.log_count <= 0) {
                    std::cerr << "Error: Invalid log count" << std::endl;
                    return false;
                }
            } else {
                std::cerr << "Error: --count requires an argument" << std::endl;
                return false;
            }
        }
        else if (arg == "-j" || arg == "--threads") {
            if (i + 1 < argc) {
                config.thread_count = std::atoi(argv[++i]);
                if (config.thread_count <= 0) {
                    std::cerr << "Error: Invalid thread count" << std::endl;
                    return false;
                }
            } else {
                std::cerr << "Error: --threads requires an argument" << std::endl;
                return false;
            }
        }
        else if (arg == "-F" || arg == "--flush") {
            config.flush_on_write = true;
        }
#ifdef ENABLE_SPDLOG
        else if (arg == "-a" || arg == "--async") {
            config.async = true;
        }
#endif 
        else if (arg == "-l" || arg == "--level") {
            if (i + 1 < argc) {
                std::string level_str = argv[++i];
                config.level = slog::log_level_from_name(level_str, slog::LogLevel::Unknown);
                if (config.level == slog::LogLevel::Unknown) {
                    std::cerr << "Error: Invalid log level '" << level_str << "'" << std::endl;
                    return false;
                }
            } else {
                std::cerr << "Error: --level requires an argument" << std::endl;
                return false;
            }
        }
        else {
            std::cerr << "Error: Unknown option '" << arg << "'" << std::endl;
            print_usage(argv[0]);
            return false;
        }
    }
    
    return true;
}

/**
 * @brief 验证文件日志输出
 */
void verify_file_output(std::string const & filepath, int expected_lines)
{
    if (!std::filesystem::exists(filepath)) {
        std::cout << "\n⚠️  Warning: Log file does not exist!" << std::endl;
        return;
    }
    
    auto file_size = std::filesystem::file_size(filepath);
    std::cout << "\n=== File Output Verification ===" << std::endl;
    std::cout << "File Path       : " << filepath << std::endl;
    std::cout << "File Size       : " << file_size << " bytes" << std::endl;
    std::cout << "Expected Lines  : " << expected_lines << std::endl;
    
    // 简单验证：检查文件大小是否合理
    // 假设每行日志约100-150字节
    size_t min_expected_size = expected_lines * 80;
    size_t max_expected_size = expected_lines * 200;
    
    if (file_size >= min_expected_size && file_size <= max_expected_size) {
        std::cout << "Status          : ✅ File size looks reasonable" << std::endl;
    } else if (file_size < min_expected_size) {
        std::cout << "Status          : ⚠️  File size seems too small (possible data loss)" << std::endl;
    } else {
        std::cout << "Status          : ⚠️  File size seems too large" << std::endl;
    }
}

int main(int argc, char* argv[]) 
{
    TestConfig config;
    
    // 解析命令行参数
    if (!parse_args(argc, argv, config)) {
        return 1;
    }
    
    // 打印配置
    config.print();
    
    // 清理旧的日志文件（只在测试开始前清理一次）
    if ((config.log_type == "file" || config.log_type == "spdlog-file") 
        && std::filesystem::exists(config.log_file)) {
        std::filesystem::remove(config.log_file);
        std::cout << "\n[Cleanup] Removed old log file: " << config.log_file << std::endl;
    }
    
    std::vector<TestResult> results;
    
    try {
        // 单线程测试（不清理文件）
        auto single_result = test_single_thread(config);
        results.push_back(single_result);
        single_result.print();
        
        // 多线程测试（如果线程数大于1，不清理文件，追加到同一文件）
        if (config.thread_count > 1) {
            auto multi_result = test_multi_thread(config);
            results.push_back(multi_result);
            multi_result.print();
        }
        
        // 如果是文件日志，验证输出
        if (config.log_type == "file" || config.log_type == "spdlog-file") {
            // 确保所有logger都已销毁，文件已关闭
            // 短暂延迟以确保文件系统更新
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            int total_expected = config.log_count;
            if (config.thread_count > 1) {
                total_expected += (config.log_count / config.thread_count) * config.thread_count;
            }
            verify_file_output(config.log_file, total_expected);
        }
        
        // 打印汇总
        std::cout << "\n========================================" << std::endl;
        std::cout << "  Performance Test Summary" << std::endl;
        std::cout << "========================================" << std::endl;
        
        for (const auto& result : results) {
            std::cout << result.test_name << ":" << std::endl;
            std::cout << "  - Throughput: " << std::fixed << std::setprecision(0) 
                      << result.logs_per_second << " logs/sec" << std::endl;
            std::cout << "  - Avg Time : " << std::fixed << std::setprecision(2) 
                      << result.us_per_log << " μs/log" << std::endl;
        }
        
        std::cout << "\n✅ All tests completed successfully!" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}

