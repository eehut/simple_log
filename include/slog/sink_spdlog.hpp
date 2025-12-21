#ifndef __SLOG_SINK_SPDLOG_H__
#define __SLOG_SINK_SPDLOG_H__

/**
 * @file sink_spdlog.hpp
 * @author LiuChuansen (179712066@qq.com)
 * @brief Spdlog Sink实现（支持 Console 和/或 File）
 * @version 0.1
 * @date 2025-12-20
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <string>
#include <memory>

#include "slog/slog.hpp"

namespace slog {
namespace sink {

// Forward declaration for pimpl pattern
struct SpdlogSinkImpl;

/**
 * @brief Spdlog Sink 类型标志
 * 
 * 使用位标志，可以组合使用
 */
enum SpdlogSinkType {
    NoSink      = 0,        // 无输出
    ToConsole   = 1 << 0,   // 控制台输出 (0x01)
    ToFile      = 1 << 1    // 文件输出 (0x02)
};

/**
 * @brief Spdlog Sink 实现
 * 
 * 封装 spdlog 的 logger，支持 console 和/或 file 输出
 * 可以单独启用 console 或 file，也可以同时启用两者
 * 
 * 特性：
 * - 线程安全：使用 spdlog 的 _mt 版本（多线程安全）
 * - 无缓存：关闭 spdlog 的缓存功能，立即刷新
 * - 同步/异步：支持运行时选择
 * - 多 sink：一个 logger 可以同时输出到多个目标
 */
class Spdlog: public LoggerSink
{
public:
    /**
     * @brief 构造函数
     * @param level 日志等级
     * @param sink_type Sink 类型标志，可以组合（如 Console | File）
     * @param filepath 日志文件路径（仅当启用 File 时需要）
     * @param async 是否使用异步模式，默认false（同步）
     */
    explicit Spdlog(LogLevel level, int sink_type, std::string const & filepath = "", bool async = false);
    
    ~Spdlog();
    
    // 禁止复制构造和赋值
    Spdlog(Spdlog const &) = delete;
    Spdlog & operator=(Spdlog const &) = delete;
    
    // 允许移动
    Spdlog(Spdlog &&) noexcept;
    Spdlog & operator=(Spdlog &&) noexcept;
    
    std::shared_ptr<LoggerSink> clone(std::string const & logger_name) const override;
    
    bool setup(std::string const & name) override;
    
    void log(LogLevel level, std::string const &msg) override;
    
    void set_level(LogLevel level) override;
    
    LogLevel get_level() const override;
    
    const char* name() const override;

private:
    std::unique_ptr<SpdlogSinkImpl> pimpl_;
    std::string name_;
    LogLevel level_;
    int sink_type_;
    std::string filepath_;
    bool async_;
};

} // namespace sink
} // namespace slog

#endif // __SLOG_SINK_SPDLOG_H__

