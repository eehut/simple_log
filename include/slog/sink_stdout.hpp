#ifndef __SLOG_SINK_STDOUT_H__
#define __SLOG_SINK_STDOUT_H__

/**
 * @file sink_stdout.hpp
 * @author LiuChuansen (179712066@qq.com)
 * @brief Stdout Sink实现
 * @version 0.1
 * @date 2025-12-04
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <string>
#include <memory>
#include "slog/slog.hpp"

namespace slog {
namespace sink {

/**
 * @brief Stdout Sink实现
 * 
 * 线程安全：所有 Stdout sink 共享一个全局 mutex，确保输出到 std::cout 的日志不会交错
 */
class Stdout: public LoggerSink
{
public:
    explicit Stdout(LogLevel level);
    
    std::shared_ptr<LoggerSink> clone(std::string const & logger_name) const override;
    
    bool setup(std::string const & name) override;
    
    void log(LogLevel level, std::string const &msg) override;
    
    void set_level(LogLevel level) override;
    
    LogLevel get_level() const override;
    
    const char* name() const override;

private:
    std::string name_;
    LogLevel level_;
    
    /// @brief 获取全局 stdout mutex（所有 Stdout sink 共享）
    /// @return std::mutex& 
    static std::mutex& get_stdout_mutex();
};

} // namespace sink
} // namespace slog

#endif // __SLOG_SINK_STDOUT_H__
