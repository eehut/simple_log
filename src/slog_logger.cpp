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
#include <regex>

#include "slog/slog.hpp"
#include "sink_stdout.hpp"
#include "sink_none.hpp"

namespace slog {

//#define SLOG_DEBUG_ENABLE 1

// Debug macro for internal debugging (can be enabled by defining SLOG_DEBUG_ENABLE)
#ifdef SLOG_DEBUG_ENABLE
#define __debug(fmt, ...) \
    do { \
        std::printf("[SLOG_DEBUG] " fmt "\n", ##__VA_ARGS__); \
        std::fflush(stdout); \
    } while(0)
#else
#define __debug(fmt, ...) ((void)0)
#endif 

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

void Logger::log_data(LogLevel level, void const *data, size_t size, std::string const &msg) {
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

void Logger::log_data(LogLevel level, std::vector<uint8_t> const & data, std::string const &msg) {
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

void Logger::log_limited(std::string const &tag, int allowed_num, LogLevel level, std::string const &msg)
{
    int left = limited_allowed_left(tag, allowed_num);
    if (valid_ && is_allowed(level) && (left > 0))
    {
        sink_->log(level, (left == 1) ? (msg + " (more messages will be suppressed)") : msg);
    }
}

int Logger::limited_allowed_left(std::string const &tag, int allowed_num)
{
    auto it = limited_items_.find(tag);
    if (it == limited_items_.end()) {
        LimitedControl ctrl;
        ctrl.allowed = allowed_num;
        ctrl.count = 0;
        limited_items_.insert(std::make_pair(tag, ctrl));
    }

    auto & ctrl = limited_items_[tag];
    // 如果allowed不一样，更新这个值
    if (ctrl.allowed != allowed_num){
        ctrl.allowed = allowed_num;
    }

    if (ctrl.count < ctrl.allowed) {
        int tmp = ctrl.allowed - ctrl.count;
        ctrl.count ++;
        return tmp;
    } else {
        return 0;
    }
}


std::shared_ptr<Logger> Logger::clone(std::string const & logger_name) const
{
    auto sink = sink_->clone(logger_name);
    if (!sink) {
        return nullptr;
    }

    __debug("clone logger: %s", logger_name.c_str());

    auto logger = std::make_shared<Logger>(logger_name, sink);
    logger->valid_ = valid_;
    register_logger(logger);

    return logger;
}

std::shared_ptr<Logger> Logger::clone(std::string const & logger_name, LogLevel level) const
{
    auto sink = sink_->clone(logger_name);
    if (!sink) {
        return nullptr;
    }

    __debug("clone logger: %s", logger_name.c_str());

    auto logger = std::make_shared<Logger>(logger_name, sink);
    logger->valid_ = valid_;
    logger->set_level(level);  // 设置指定的日志等级
    register_logger(logger);

    return logger;
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

        __debug("register logger: %s", logger->name().c_str());

        // 应用全局日志等级规则（如果存在匹配的规则）
        auto level = detail::LoggerRegistry::instance().get_logger_level_rule(logger->name());
        if (level != LogLevel::Unknown) {
            logger->set_level(level);
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
        __debug("make logger: %s", name.c_str());
        auto logger = std::make_shared<Logger>(name, sink);
        register_logger(logger);        
        return logger;
    }

    /**
     * @brief 将 shell 通配符模式转换为正则表达式
     * 
     * 将 shell 风格的 * 和 ? 转换为正则表达式：
     * - * 转换为 .*
     * - ? 转换为 .
     * - 转义其他特殊字符
     * 
     * @param pattern shell 通配符模式
     * @return std::string 转换后的正则表达式
     */
    static std::string wildcard_to_regex(const std::string& pattern) {
        std::string regex_pattern;
        regex_pattern.reserve(pattern.size() * 2);  // 预留空间
        
        for (char c : pattern) {
            switch (c) {
                case '*':
                    regex_pattern += ".*";
                    break;
                case '?':
                    regex_pattern += '.';
                    break;
                case '.':
                case '+':
                case '^':
                case '$':
                case '[':
                case ']':
                case '(':
                case ')':
                case '{':
                case '}':
                case '|':
                    // 转义正则表达式特殊字符
                    regex_pattern += '\\';
                    regex_pattern += c;
                    break;
                default:
                    regex_pattern += c;
                    break;
            }
        }
        
        return regex_pattern;
    }

    /**
     * @brief 设置全局日志等级规则
     * 
     * 支持三种模式：
     * 1. 精确匹配：直接使用 logger 名称，如 "my_logger"
     * 2. Shell 通配符模式：使用 * 和 ?，如 "v4l2-*" 可以匹配所有以 "v4l2-" 开头的 logger
     * 3. 正则表达式匹配：使用正则表达式模式，如 ".*_debug" 可以匹配所有以 "_debug" 结尾的 logger
     * 
     * 优先级：精确匹配 > Shell 通配符 > 正则匹配
     * 
     * @param pattern logger名称、shell 通配符模式或正则表达式模式
     * @param level 日志等级
     */
    void set_logger_level_rule(const std::string& pattern, LogLevel level) {

        // 不允许 为空
        if (pattern.empty()) {
            return;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        
        // 检查是否包含 shell 通配符（* 或 ?），但不包含正则表达式特殊字符
        bool has_wildcard = false;
        bool has_regex_special = false;
        
        for (char c : pattern) {
            if (c == '*' || c == '?') {
                has_wildcard = true;
            }
            if (c == '.' || c == '+' || c == '^' || c == '$' || 
                c == '[' || c == ']' || c == '(' || c == ')' ||
                c == '{' || c == '}' || c == '|' || c == '\\') {
                has_regex_special = true;
                break;
            }
        }
        
        // 如果包含通配符但不包含正则特殊字符，转换为正则表达式
        std::string regex_pattern_str = pattern;
        if (has_wildcard && !has_regex_special) {
            regex_pattern_str = wildcard_to_regex(pattern);
            __debug("convert wildcard pattern '%s' to regex '%s'", pattern.c_str(), regex_pattern_str.c_str());
        }
        
        // 尝试编译为正则表达式
        try {
            std::regex regex_pattern(regex_pattern_str, std::regex::ECMAScript | std::regex::optimize);
            
            // 如果包含通配符或正则特殊字符，作为正则规则
            if (has_wildcard || has_regex_special) {
                // 作为正则表达式规则存储（保存原始字符串、编译后的正则表达式和日志等级）
                regex_level_rules_.emplace_back(std::make_tuple(pattern, std::move(regex_pattern), level));
                __debug("add regex level rule: %s", pattern.c_str());
            } else {
                // 作为精确匹配规则存储
                level_rules_[pattern] = level;
                __debug("add exact level rule: %s", pattern.c_str());
            }
        } catch (const std::regex_error&) {
            // 如果正则表达式编译失败，作为精确匹配规则
            level_rules_[pattern] = level;
            __debug("add exact level rule (regex compile failed): %s", pattern.c_str());
        }
        
        // 立即应用到所有已存在的匹配的 logger
        bool is_regex = has_wildcard || has_regex_special;
        apply_rules_to_existing_loggers(pattern, level, is_regex);
    }

    /**
     * @brief 获取全局日志等级规则
     * 
     * 优先级：精确匹配 > 正则匹配（按添加顺序，第一个匹配的规则生效）
     * 
     * @param name logger名称
     * @return LogLevel 如果存在规则返回对应等级，否则返回 LogLevel::Unknown
     */
    LogLevel get_logger_level_rule(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // 首先检查精确匹配
        auto it = level_rules_.find(name);
        if (it != level_rules_.end()) {
            __debug("get exact level rule for logger: %s %s", name.c_str(), log_level_name(it->second));
            return it->second;
        }
        
        // 然后检查正则表达式匹配（按添加顺序，第一个匹配的规则生效）
        for (const auto& regex_rule : regex_level_rules_) {
            #ifdef SLOG_DEBUG_ENABLE
            const std::string& pattern_str = std::get<0>(regex_rule);
            #endif
            const std::regex& regex_pattern = std::get<1>(regex_rule);
            LogLevel rule_level = std::get<2>(regex_rule);
            __debug("try match regex rule: %s to logger: %s", pattern_str.c_str(), name.c_str());
            if (std::regex_match(name, regex_pattern)) {
                __debug("get regex level rule for logger: %s %s", name.c_str(), log_level_name(rule_level));
                return rule_level;
            }
        }
        
        return LogLevel::Unknown;
    }

private:
    /**
     * @brief 应用规则到已存在的 logger
     * 
     * @param pattern 规则模式（精确匹配或正则表达式）
     * @param level 日志等级（未使用，保留用于未来扩展）
     * @param is_regex 是否是正则表达式
     */
    void apply_rules_to_existing_loggers(const std::string& pattern, LogLevel  level, bool is_regex) {
        if (is_regex) {
            // 正则表达式匹配：遍历所有 logger
            try {
                // 检查是否包含 shell 通配符，需要转换
                bool has_wildcard = false;
                bool has_regex_special = false;
                
                for (char c : pattern) {
                    if (c == '*' || c == '?') {
                        has_wildcard = true;
                    }
                    if (c == '.' || c == '+' || c == '^' || c == '$' || 
                        c == '[' || c == ']' || c == '(' || c == ')' ||
                        c == '{' || c == '}' || c == '|' || c == '\\') {
                        has_regex_special = true;
                        break;
                    }
                }
                
                std::string regex_pattern_str = pattern;
                if (has_wildcard && !has_regex_special) {
                    regex_pattern_str = wildcard_to_regex(pattern);
                }
                
                std::regex regex_pattern(regex_pattern_str, std::regex::ECMAScript | std::regex::optimize);
                for (auto& pair : registry_) {
                    __debug("try match regex rule: %s to logger: %s", pattern.c_str(), pair.first.c_str());
                    if (std::regex_match(pair.first, regex_pattern)) {
                        auto logger = pair.second;
                        logger->set_level(level);
                        __debug("apply regex level rule to logger: %s", pair.first.c_str());
                    }
                }
            } catch (const std::regex_error&) {
                // 正则表达式编译失败，忽略
            }
        } else {
            // 精确匹配
            for (auto& pair : registry_) {
                if (pair.first == pattern) {
                    auto logger = pair.second;
                    logger->set_level(level);
                    __debug("apply exact level rule to logger: %s", pair.first.c_str());
                }
            }
        }
    }

private:
    LoggerRegistry() = default;
    ~LoggerRegistry() = default;
    LoggerRegistry(const LoggerRegistry&) = delete;
    LoggerRegistry& operator=(const LoggerRegistry&) = delete;

    mutable std::mutex mutex_;  ///< 使用 mutable 以便在 const 方法中锁定
    std::unordered_map<std::string, std::shared_ptr<Logger>> registry_;
    std::shared_ptr<Logger> default_logger_;
    std::map<std::string, LogLevel> level_rules_;  ///< 全局日志等级规则（精确匹配）
    std::vector<std::tuple<std::string, std::regex, LogLevel>> regex_level_rules_;  ///< 全局日志等级规则（正则表达式匹配）：存储原始字符串、编译后的正则表达式和日志等级
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

void set_logger_level(const std::string& name, LogLevel level)
{
    detail::LoggerRegistry::instance().set_logger_level_rule(name, level);
}


} // namespace slog

