

#ifndef _SLOG_SPDLOG_DEMO_HPP_
#define _SLOG_SPDLOG_DEMO_HPP_

/**
 * @file slog_spdlog_demo.hpp
 * @author LiuChuansen (179712066@qq.com)
 * @brief  演示自定义的一个SINK，使用spdlog（头文件实现版本）
 * @version 0.1
 * @date 2026-01-04
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include <string>
#include <memory>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>

#include "slog/slog.hpp"

// Include spdlog headers
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace slog {
namespace sink {

/**
 * @brief SpdlogDemo Sink 实现（头文件版本）
 * 
 * 使用 spdlog 实现 stdout 和 file sink
 * - 支持控制台输出（可选）
 * - 自动创建进程名子目录
 * - 日志文件格式：{YY-mm-dd_HH-MM-SS}_{pid}.txt
 */
class SpdlogDemo: public LoggerSink
{
public:
    explicit SpdlogDemo(LogLevel level, std::string const & log_files_dir = "/tmp", bool console = false)
        : level_(level), log_files_dir_(log_files_dir), console_(console), logger_(nullptr)
    {
    }
    
    ~SpdlogDemo() = default;
    
    // 禁止复制构造和赋值
    SpdlogDemo(SpdlogDemo const &) = delete;
    SpdlogDemo & operator=(SpdlogDemo const &) = delete;
    
    // 允许移动
    SpdlogDemo(SpdlogDemo &&) noexcept = default;
    SpdlogDemo & operator=(SpdlogDemo &&) noexcept = default;
    
    std::shared_ptr<LoggerSink> clone(std::string const & logger_name) const override
    {
        // If name is the same, return nullptr
        if (logger_name == name_) {
            return nullptr;
        }

        if (logger_ == nullptr) {
            std::cerr << "***Internal error***: logger_ is nullptr" << std::endl;
            return nullptr;
        }
        
        // Create a new sink with same configuration
        auto sink = std::make_shared<SpdlogDemo>(level_, log_files_dir_, console_);

        // 使用spdlog自带的clone实现，避免重复打开相同文件的问题
        auto clone_logger = logger_->clone(logger_name);
        sink->name_ = logger_name;
        sink->logger_ = clone_logger;

        //sink->setup(logger_name);
        return sink;
    }
    
    bool setup(std::string const & name) override
    {
        // set name
        name_ = name;
        
        try {
            // Collect spdlog sinks
            std::vector<spdlog::sink_ptr> sinks;
            
            // Add console sink if enabled
            if (console_) {
                std::cout << "console logger enabled" << std::endl;
                auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
                console_sink->set_level(spdlog::level::trace);
                sinks.push_back(console_sink);
            }
            
            // Add file sink
            std::string filepath = create_log_filepath();
            if (!filepath.empty()) {
                std::cout << "file logger enabled, path: " << filepath << std::endl;
                auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filepath);
                file_sink->set_level(spdlog::level::trace);
                sinks.push_back(file_sink);
            }
            
            if (sinks.empty()) {
                return false;
            }
            
            // Create synchronous logger with multiple sinks
            logger_ = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
            
            // Set pattern to match simple_log format: [timestamp] [level] [logger_name] message
            logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");
            
            // Disable buffering - flush immediately
            logger_->flush_on(spdlog::level::trace);
            
            // Set level
            logger_->set_level(to_spdlog_level(level_));
            
        } catch (const spdlog::spdlog_ex& ex) {
            return false;
        }
        
        return true;
    }
    
    void log(LogLevel level, std::string const &msg) override
    {
        // Check if level is allowed
        if (static_cast<int>(level) < static_cast<int>(level_)) {
            return;
        }
        
        if (!logger_) {
            return;
        }
        
        // Convert slog level to spdlog level
        spdlog::level::level_enum spdlog_level = to_spdlog_level(level);
        
        // Log using spdlog
        logger_->log(spdlog_level, msg);
    }
    
    void set_level(LogLevel level) override
    {
        level_ = level;
        if (logger_) {
            logger_->set_level(to_spdlog_level(level));
        }
    }
    
    LogLevel get_level() const override
    {
        return level_;
    }
    
    const char* name() const override
    {
        return "SpdlogDemo";
    }

private:
    // Convert slog LogLevel to spdlog level
    static spdlog::level::level_enum to_spdlog_level(LogLevel level)
    {
        switch (level) {
            case slog::LogLevel::Trace:   return spdlog::level::trace;
            case slog::LogLevel::Debug:   return spdlog::level::debug;
            case slog::LogLevel::Info:    return spdlog::level::info;
            case slog::LogLevel::Warning: return spdlog::level::warn;
            case slog::LogLevel::Error:   return spdlog::level::err;
            case slog::LogLevel::Off:     return spdlog::level::off;
            default:                      return spdlog::level::info;
        }
    }
    
    // Get process name from /proc/self/exe (no length limit, unlike /proc/self/comm)
    static std::string get_process_name()
    {
        std::string proc_name;
        
        // Try to read /proc/self/exe symlink to get full executable path
        char exe_path[4096] = {0};
        ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
        
        if (len > 0) {
            exe_path[len] = '\0';
            std::string full_path(exe_path);
            
            // Extract filename from path (handle both / and \ for cross-platform)
            size_t last_slash = full_path.find_last_of("/\\");
            if (last_slash != std::string::npos && last_slash + 1 < full_path.length()) {
                proc_name = full_path.substr(last_slash + 1);
            } else {
                proc_name = full_path;
            }
        }
        
        // Fallback to /proc/self/comm if readlink failed
        if (proc_name.empty()) {
            std::ifstream comm_file("/proc/self/comm");
            if (comm_file.is_open()) {
                std::getline(comm_file, proc_name);
                comm_file.close();
            }
        }
        
        // Final fallback to "unknown" if all methods failed
        if (proc_name.empty()) {
            proc_name = "unknown";
        }
        
        return proc_name;
    }
    
    // Create directory recursively (C++14 compatible)
    static bool create_directories_recursive(const std::string& path)
    {
        if (path.empty()) {
            return false;
        }
        
        struct stat st;
        if (stat(path.c_str(), &st) == 0) {
            // Directory already exists
            return S_ISDIR(st.st_mode);
        }
        
        // Try to create parent directory first
        size_t pos = path.find_last_of('/');
        if (pos != std::string::npos && pos > 0) {
            std::string parent = path.substr(0, pos);
            // Skip root directory (e.g., "/")
            if (!parent.empty() && parent != "/") {
                if (!create_directories_recursive(parent)) {
                    return false;
                }
            }
        }
        
        // Create this directory
        if (mkdir(path.c_str(), 0755) != 0) {
            // Check if it was created by another thread
            if (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
                return true;
            }
            return false;
        }
        
        return true;
    }
    
    // Create log file path: {log_files_dir}/{process_name}/{YY-mm-dd_HH-MM-SS}_{pid}.txt
    std::string create_log_filepath()
    {
        // Get process name
        std::string proc_name = get_process_name();
        
        // Create subdirectory path: {log_files_dir}/{process_name}
        std::ostringstream dir_path;
        dir_path << log_files_dir_;
        if (!log_files_dir_.empty() && log_files_dir_.back() != '/') {
            dir_path << "/";
        }
        dir_path << proc_name;
        
        std::string log_dir = dir_path.str();
        
        // Create directory if not exists
        if (!create_directories_recursive(log_dir)) {
            return "";  // Failed to create directory
        }
        
        // Generate filename: {YY-mm-dd_HH-MM-SS}_{pid}.txt
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&time);
        
        std::ostringstream filename;
        filename << std::put_time(&tm, "%Y%m%d_%H%M%S");
        filename << "_" << getpid() << ".txt";
        
        // Combine directory and filename
        std::ostringstream filepath;
        filepath << log_dir << "/" << filename.str();
        
        return filepath.str();
    }
    
    std::string name_;
    LogLevel level_;
    std::string log_files_dir_;
    bool console_;
    std::shared_ptr<spdlog::logger> logger_;
};

} // namespace sink

inline std::shared_ptr<Logger> make_spdlog_demo_logger(std::string const &name, LogLevel level, std::string const & log_files_dir = "/tmp", bool console = false)
{
    auto sink = std::make_shared<sink::SpdlogDemo>(level, log_files_dir, console);
    return make_logger(name, sink);
}

} // namespace slog

#endif // _SLOG_SPDLOG_DEMO_HPP_
