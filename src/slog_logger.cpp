/**
 * @file slog_logger.cpp
 * @author LiuChuansen (179712066@qq.com)
 * @brief Logger实现
 * @version 0.1
 * @date 2025-12-04
 */

#include <iostream>
#include <sstream>
#include <cstdio>
#include <cstdint>

#include "slog/slog.hpp"
#include "sink_stdout.hpp"
#include "sink_none.hpp"

namespace slog {

// Logger implementation
Logger::Logger(std::string const &name, std::shared_ptr<LoggerSink> sink)
    : name_(name), valid_(false)
{
    if (sink == nullptr)
    {
        // 如果为空，使用一个默认的stdout SINK
        sink = std::make_shared<sink::Stdout>(LogLevel::Info);        
    }

    sink_ = std::move(sink);

    // 建立sink
    valid_ = sink_->setup(this->name_);

    if (!valid_)
    {
        std::cerr << "setup logger("<< sink_->name() << ") failed" << std::endl;
    }
}

std::string const &Logger::name() const {
    return name_;
}

LogLevel Logger::get_level() const {
    return sink_->get_level();
}

void Logger::set_level(LogLevel level) {
    sink_->set_level(level);
}

bool Logger::is_allowed(LogLevel level) const noexcept {
    return static_cast<int>(level) >= static_cast<int>(sink_->get_level());
}

void Logger::log(LogLevel level, std::string const &msg) {
    if (valid_ && is_allowed(level))
    {
        sink_->log(level, msg);
    }
}

void Logger::log(LogLevel level, const char* msg) {
    if (valid_ && is_allowed(level))
    {
        sink_->log(level, std::string(msg));
    }
}

void Logger::dump(LogLevel level, void const *data, size_t size, std::string const &msg) {
    if (!valid_ || !is_allowed(level))
    {
        return;
    }

    std::string hex;
    std::size_t size_per_line = 16;
    if (size >= size_per_line)
    {
        hex = "\r\n";
    }
    const uint8_t *array = static_cast<const uint8_t *>(data);
    for (size_t i = 0; i < size; ++ i)
    {
        if (!(i % size_per_line))
        {
            if (i > 0)
            {
                hex += "\r\n";
            }

            char buf[32];
            std::snprintf(buf, sizeof(buf), "%04zX: ", i);
            hex += buf;
        }

        char buf[8];
        std::snprintf(buf, sizeof(buf), "%02X ", array[i]);
        hex += buf;
    }

    sink_->log(level, msg + hex);    
}

void Logger::dump(LogLevel level, std::vector<uint8_t> const & data, std::string const &msg) {
    if (!valid_ || !is_allowed(level))
    {
        return;
    }

    std::string hex;
    std::size_t size_per_line = 16;
    if (data.size() >= size_per_line)
    {
        hex = "\r\n";
    }
    for (size_t i = 0; i < data.size(); ++ i)
    {
        if (!(i % size_per_line))
        {
            if (i > 0)
            {
                hex += "\r\n";
            }

            char buf[32];
            std::snprintf(buf, sizeof(buf), "%04zX: ", i);
            hex += buf;
        }

        char buf[8];
        std::snprintf(buf, sizeof(buf), "%02X ", data[i]);
        hex += buf;
    }

    sink_->log(level, msg + hex);
}


namespace detail
{

/**
 * @brief 全局日志注册表管理器（使用Meyer's Singleton模式）
 * 
 * 使用静态局部变量实现线程安全的单例模式，无需源文件即可实现全局对象管理
 */
class LoggerRegistry
{
public:
    /**
     * @brief 获取单例实例
     * 
     * @return LoggerRegistry& 
     */
    static LoggerRegistry& instance() {
        static LoggerRegistry reg;
        return reg;
    }

    /**
     * @brief 获取默认logger
     * 
     * @return std::shared_ptr<Logger> 
     */
    std::shared_ptr<Logger> get_default() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!default_logger_) {
            // 如果注册表中有logger，使用第一个
            if (!registry_.empty()) {
                default_logger_ = registry_.begin()->second;
            } else {
                // 创建默认logger
                default_logger_ = std::make_shared<Logger>("default", std::make_shared<sink::Stdout>(LogLevel::Info));
            }
        }
        return default_logger_;
    }

    /**
     * @brief 注册一个logger
     * 
     * @param logger logger指针
     * @return true 成功
     * @return false 失败（logger为空）
     */
    bool register_logger(std::shared_ptr<Logger> logger) {
        if (!logger) {
            return false;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        registry_[logger->name()] = logger;
        return true;
    }

    /**
     * @brief 设置默认logger
     * 
     * @param name logger名称
     * @return true 成功
     * @return false 失败（logger不存在）
     */
    bool set_default(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = registry_.find(name);
        if (it != registry_.end()) {
            default_logger_ = it->second;
            return true;
        }
        return false;
    }

    /**
     * @brief 获取指定名称的logger
     * 
     * @param name logger名称
     * @return std::shared_ptr<Logger> logger指针，如果不存在返回nullptr
     */
    std::shared_ptr<Logger> get_logger(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = registry_.find(name);
        return (it != registry_.end()) ? it->second : nullptr;
    }

    /**
     * @brief 检查logger是否存在
     * 
     * @param name logger名称
     * @return true 存在
     * @return false 不存在
     */
    bool has_logger(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex_);
        return registry_.find(name) != registry_.end();
    }

    /**
     * @brief 移除一个logger
     * 
     * @param name logger名称
     */
    void drop_logger(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex_);
        registry_.erase(name);
        // 如果被移除的是默认logger，重置默认logger
        if (default_logger_ && default_logger_->name() == name) {
            default_logger_.reset();
        }
    }

    /**
     * @brief 创建并注册一个logger
     * 
     * @param name logger名称
     * @param sink sink指针，如果为空则使用默认stdout sink
     * @return std::shared_ptr<Logger> logger指针
     */
    std::shared_ptr<Logger> make_logger(const std::string& name, std::shared_ptr<LoggerSink> sink = nullptr) {
        auto logger = std::make_shared<Logger>(name, sink);
        register_logger(logger);
        return logger;
    }

private:
    LoggerRegistry() = default;
    ~LoggerRegistry() = default;
    LoggerRegistry(const LoggerRegistry&) = delete;
    LoggerRegistry& operator=(const LoggerRegistry&) = delete;

    std::mutex mutex_;
    std::unordered_map<std::string, std::shared_ptr<Logger>> registry_;
    std::shared_ptr<Logger> default_logger_;
};

} // namespace detail

// Global function implementations
std::shared_ptr<Logger> default_logger() {
    return detail::LoggerRegistry::instance().get_default();
}

bool register_logger(std::shared_ptr<Logger> logger) {
    return detail::LoggerRegistry::instance().register_logger(logger);
}

bool set_default_logger(const std::string& name) {
    return detail::LoggerRegistry::instance().set_default(name);
}

std::shared_ptr<Logger> get_logger(const std::string& name) {
    return detail::LoggerRegistry::instance().get_logger(name);
}

bool has_logger(const std::string& name) {
    return detail::LoggerRegistry::instance().has_logger(name);
}

void drop_logger(const std::string& name) {
    detail::LoggerRegistry::instance().drop_logger(name);
}

std::shared_ptr<Logger> make_logger(const std::string& name, std::shared_ptr<LoggerSink> sink) {
    return detail::LoggerRegistry::instance().make_logger(name, sink);
}

std::shared_ptr<Logger> make_none_logger(std::string const &name)
{
    return make_logger(name, std::make_shared<sink::None>());
}

std::shared_ptr<Logger> make_stdout_logger(std::string const &name, LogLevel level)
{
    return make_logger(name, std::make_shared<sink::Stdout>(level));
}


} // namespace slog

