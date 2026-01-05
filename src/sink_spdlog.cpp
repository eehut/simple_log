/**
 * @file sink_spdlog.cpp
 * @author LiuChuansen (179712066@qq.com)
 * @brief Spdlog Sink实现
 * @version 0.1
 * @date 2025-12-20
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <mutex>
#include <memory>
#include <vector>
#include "slog/sink_spdlog.hpp"

// Include spdlog headers only in implementation file
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>
#include <spdlog/async_logger.h>

namespace slog {
namespace sink {

// Pimpl implementation
struct SpdlogSinkImpl {
    std::shared_ptr<spdlog::logger> logger;
    bool async;
    
    SpdlogSinkImpl(std::shared_ptr<spdlog::logger> l, bool a) 
        : logger(std::move(l)), async(a) {}
};

// Global mutex for async thread pool initialization
static std::mutex s_async_init_mutex;
static bool s_async_thread_pool_initialized = false;

// Initialize async thread pool (called once)
static void init_async_thread_pool() 
{
    std::lock_guard<std::mutex> lock(s_async_init_mutex);
    if (!s_async_thread_pool_initialized) {
        spdlog::init_thread_pool(8192, 2);
        s_async_thread_pool_initialized = true;
    }
}

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

Spdlog::Spdlog(LogLevel level, int sink_type, std::string const & filepath, bool async)
    : level_(level), sink_type_(sink_type), filepath_(filepath), async_(async)
{
    // Initialization will be done in setup()
}

Spdlog::~Spdlog() 
{
    // spdlog logger will be automatically cleaned up when shared_ptr is destroyed
}

Spdlog::Spdlog(Spdlog &&) noexcept = default;
Spdlog & Spdlog::operator=(Spdlog &&) noexcept = default;

std::shared_ptr<LoggerSink> Spdlog::clone(std::string const & logger_name) const 
{
    // If name is the same, return nullptr
    if (logger_name == name_) {
        return nullptr;
    }
    
    // Create a new sink with same configuration
    auto sink = std::make_shared<Spdlog>(level_, sink_type_, filepath_, async_);
    sink->name_ = logger_name;
    // 使用spdlog自带的clone实现，避免重复打开相同文件的问题
    auto logger = pimpl_->logger->clone(logger_name);
    sink->pimpl_ = std::make_unique<SpdlogSinkImpl>(logger, async_);

    //sink->setup(logger_name);
    return sink;
}

bool Spdlog::setup(std::string const & name) 
{
    name_ = name;
    
    // Check if sink type is valid
    if (sink_type_ == SpdlogSinkType::NoSink) {
        return false;
    }
    
    try {
        // Collect spdlog sinks based on sink_type_
        std::vector<spdlog::sink_ptr> sinks;
        
        // Add console sink if enabled
        if (sink_type_ & SpdlogSinkType::ToConsole) {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(spdlog::level::trace);
            sinks.push_back(console_sink);
        }
        
        // Add file sink if enabled
        if (sink_type_ & SpdlogSinkType::ToFile) {
            if (filepath_.empty()) {
                return false;  // File path is required when File sink is enabled
            }
            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filepath_);
            file_sink->set_level(spdlog::level::trace);
            sinks.push_back(file_sink);
        }
        
        // Create logger based on sync/async mode
        std::shared_ptr<spdlog::logger> logger;
        
        if (async_) {
            // Initialize async thread pool if needed
            init_async_thread_pool();
            
            // Create async logger with multiple sinks
            logger = std::make_shared<spdlog::async_logger>(
                name,
                sinks.begin(),
                sinks.end(),
                spdlog::thread_pool(),
                spdlog::async_overflow_policy::block
            );
        } else {
            // Create synchronous logger with multiple sinks
            logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
        }
        
        // Set pattern to match simple_log format: [timestamp] [level] [logger_name] message
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");
        
        // Disable buffering - flush immediately
        logger->flush_on(spdlog::level::trace);
        
        // Set level
        logger->set_level(to_spdlog_level(level_));
        
        pimpl_ = std::make_unique<SpdlogSinkImpl>(logger, async_);
        
    } catch (const spdlog::spdlog_ex& ex) {
        return false;
    }
    
    return true;
}

void Spdlog::log(LogLevel level, std::string const &msg) 
{
    // Check if level is allowed
    if (static_cast<int>(level) < static_cast<int>(level_)) {
        return;
    }
    
    if (!pimpl_ || !pimpl_->logger) {
        return;
    }
    
    // Convert slog level to spdlog level
    spdlog::level::level_enum spdlog_level = to_spdlog_level(level);
    
    // Log using spdlog
    pimpl_->logger->log(spdlog_level, msg);
}

void Spdlog::set_level(LogLevel level) 
{
    level_ = level;
    if (pimpl_ && pimpl_->logger) {
        pimpl_->logger->set_level(to_spdlog_level(level));
    }
}

LogLevel Spdlog::get_level() const 
{
    return level_;
}

const char* Spdlog::name() const 
{
    return "Spdlog";
}

} // namespace sink
} // namespace slog


