#ifndef __SLOG_SINK_STDOUT_H__
#define __SLOG_SINK_STDOUT_H__

#include "slog/slog.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

namespace slog {
namespace sink {

/**
 * @brief Stdout Sink实现（内部使用，header-only）
 */
class Stdout: public LoggerSink
{
public:
    explicit Stdout(LogLevel level) : level_(level) { }

    std::shared_ptr<LoggerSink> clone(std::string const & logger_name) const override {
        auto sink = std::make_shared<Stdout>(level_);
        sink->setup(logger_name);
        return sink;
    }

    bool setup(std::string const & name) override {
        name_ = name;
        return true;
    }

    void log(LogLevel level, std::string const &msg) override {    
        // 等级不允许输出
        if (static_cast<int>(level) < static_cast<int>(level_))
        {
            return;
        }
        
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
        oss << " <" << log_level_name(level) << "> (" << name_ << ") ";

#if SLOG_STDOUT_COLOR
        std::cout << oss.str() << msg << _RESET << std::endl;
#else
        std::cout << oss.str() << msg << std::endl;
#endif // SLOG_STDOUT_COLOR
    }

    void set_level(LogLevel level) override {
        level_ = level;
    }

    LogLevel get_level() const override { 
        return level_; 
    }

    const char* name() const override { 
        return "Stdout"; 
    }

private:
    std::string name_;
    LogLevel level_;
};

} // namespace sink
} // namespace slog

#endif // __SLOG_SINK_STDOUT_H__

