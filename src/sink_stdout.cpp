/**
 * @file sink_stdout.cpp
 * @author LiuChuansen (179712066@qq.com)
 * @brief Stdout Sink实现
 * @version 0.1
 * @date 2025-12-19
 * 
 * @copyright Copyright (c) 2025
 * 
 */


#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <mutex>

#include "slog/sink_stdout.hpp"

namespace slog {
namespace sink {


std::shared_ptr<LoggerSink> Stdout::clone(const std::string & logger_name) const 
{
    auto sink = std::make_shared<Stdout>(level_);
    sink->setup(logger_name);
    return sink;
}


void Stdout::output(const std::string & logger_name, LogLevel level, std::string const &msg) 
{    
    // 使用全局锁保护所有 stdout 输出，确保多线程环境下日志不会交错
    std::lock_guard<std::mutex> lock(get_stdout_mutex());
    
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);
    std::ostringstream oss;
    
    // output timestamp
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S.");
    oss << std::setfill('0') << std::setw(3) << ms % 1000;

#if SLOG_STDOUT_COLOR
    // 颜色 ANSI escape codes
    constexpr const char* _RESET   = "\033[0m";
    constexpr const char* _RED     = "\033[0;31m";      /* Red */
    constexpr const char* _GREEN   = "\033[0;32m";      /* Green */
    constexpr const char* _YELLOW  = "\033[0;33m";      /* Yellow */
    constexpr const char* _BLUE    = "\033[0;34m";      /* Blue */

    const char *color = _RESET;
    switch(level)
    {
        case LogLevel::Debug:
        color = _BLUE;
        break;
        case LogLevel::Info:
        color = _GREEN;
        break;
        case LogLevel::Warning:
        color = _YELLOW;
        break;
        case LogLevel::Error:
        color = _RED;
        break;

        default:    
        color = _RESET;    
        break;
    }
    oss << color;
#endif // SLOG_STDOUT_COLOR

    // output log level
    oss << " <" << log_level_name(level) << "> (" << logger_name << ") ";

#if SLOG_STDOUT_COLOR
    std::cout << oss.str() << msg << _RESET << std::endl;
#else
    std::cout << oss.str() << msg << std::endl;
#endif // SLOG_STDOUT_COLOR
}

const char* Stdout::name() const 
{ 
    return "Stdout"; 
}

std::mutex& Stdout::get_stdout_mutex() 
{
    static std::mutex s_stdout_mutex;
    return s_stdout_mutex;
}

} // namespace sink
} // namespace slog


