#ifndef __SLOG_SINK_NONE_H__
#define __SLOG_SINK_NONE_H__

/**
 * @file sink_none.hpp
 * @author LiuChuansen (179712066@qq.com)
 * @brief 
 * @version 0.1
 * @date 2025-12-04
 * 
 * @copyright Copyright (c) 2025
 * 
 */


#include "slog/slog.hpp"

namespace slog {
namespace sink {

/**
 * @brief None Sink实现（内部使用，header-only）- 不输出任何日志，用于关闭日志输出
 */
class None: public LoggerSink
{
public:
    explicit None(LogLevel level = LogLevel::Off) : LoggerSink(level) { }

    std::shared_ptr<LoggerSink> clone(const std::string & logger_name) const override 
    {
        auto sink = std::make_shared<None>(level_);
        sink->setup(logger_name);
        return sink;
    }

    const char* name() const override 
    { 
        return "None"; 
    }

protected:
    void output(const std::string & logger_name, LogLevel level, std::string const &msg) override 
    {
        // 空实现，不输出任何内容
        (void)logger_name;
        (void)level;  // 避免未使用参数警告
        (void)msg;    // 避免未使用参数警告
    }
};

} // namespace sink
} // namespace slog

#endif // __SLOG_SINK_NONE_H__

