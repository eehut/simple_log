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
#include <cctype>
#include <regex>
#include <mutex>

#include "slog/slog.hpp"
#include "slog/sink_stdout.hpp"
#include "slog/sink_none.hpp"
#include "slog/sink_file.hpp"

#ifdef BUILD_WITH_SPDLOG
#include "slog/sink_spdlog.hpp"
#endif

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

    sinks_.push_back(std::move(sink));

    // 建立所有sink
    for (auto& s : sinks_)
    {
        if (!s->setup(this->name_))
        {
            std::cerr << "setup sink(" << s->name() << ") failed" << std::endl;
            return;
        }
    }
    
    valid_ = true;
    update_filter_level();
}

// 多sink构造函数实现
Logger::Logger(std::string const &name, std::vector<std::shared_ptr<LoggerSink>> sinks)
    : name_(name), sinks_(std::move(sinks)), valid_(false)
{
    if (sinks_.empty())
    {
        // 如果为空，使用一个默认的stdout SINK
        sinks_.push_back(std::make_shared<sink::Stdout>(LogLevel::Info));
    }

    // 建立所有sink
    for (auto& s : sinks_)
    {
        if (!s->setup(this->name_))
        {
            std::cerr << "setup sink(" << s->name() << ") failed" << std::endl;
            return;
        }
    }
    
    valid_ = true;
    update_filter_level();
}

std::string const &Logger::name() const 
{
    return name_;
}

LogLevel Logger::get_level() const 
{
    if (sinks_.empty())
    {
        return LogLevel::Off;
    }
    
    return min_level_;
}

void Logger::set_level(LogLevel level) 
{
    // 设置所有sink的level
    for (auto& sink : sinks_)
    {
        if (sink)
        {
            sink->set_level(level);
        }
    }

    update_filter_level();
}

bool Logger::is_allowed(LogLevel level) const noexcept 
{
    // 只要有一个sink允许就返回true
    // 多sink情况下，取决于等级值最小的。
    // 如有两个LEVEL， DEBUG， INFO， 应该允许DEBUG的信息通过
    return static_cast<int>(level) >= static_cast<int>(min_level_);
}

void Logger::log(LogLevel level, std::string const &msg) 
{
    if (!valid_ || static_cast<int>(level) < static_cast<int>(min_level_))
    {
        return;
    }
    
    // 遍历所有sink
    for (auto& sink : sinks_)
    {
        sink->log(name_,level, msg);
    }
}

void Logger::log(LogLevel level, const char* msg) 
{
    if (!valid_ || static_cast<int>(level) < static_cast<int>(min_level_))
    {
        return;
    }
    
    // 遍历所有sink
    std::string msg_str(msg);
    for (auto& sink : sinks_)
    {
        sink->log(name_, level, msg);
    }
}

void Logger::log_lines(LogLevel level, std::string const &msg) 
{
    if (!valid_ || static_cast<int>(level) < static_cast<int>(min_level_))
    {
        return;
    }

    if (msg.empty())
    {
        return;
    }

    // 边解析边输出，处理 \r\n, \n, \r 三种换行符
    std::string current_line;
    
    for (size_t i = 0; i < msg.length(); ++i)
    {
        if (msg[i] == '\r')
        {
            // 检查是否是 \r\n
            if (i + 1 < msg.length() && msg[i + 1] == '\n')
            {
                // 遇到 \r\n，立即输出当前行并跳过下一个字符
                log(level, current_line);
                current_line.clear();
                ++i; // 跳过 \n
            }
            else
            {
                // 单独的 \r，立即输出当前行
                log(level, current_line);
                current_line.clear();
            }
        }
        else if (msg[i] == '\n')
        {
            // 单独的 \n，立即输出当前行
            log(level, current_line);
            current_line.clear();
        }
        else
        {
            current_line += msg[i];
        }
    }
    
    // 输出最后一行（即使为空也要输出，以保持完整性）
    log(level, current_line);
}

void Logger::log_data(LogLevel level, void const *data, size_t size, std::string const &msg) 
{
    if (!valid_ || static_cast<int>(level) < static_cast<int>(min_level_))
    {
        return;
    }

    if (size == 0)
    {
        return;
    }

    const uint8_t *array = static_cast<const uint8_t *>(data);
    constexpr size_t size_per_line = 16;
    std::string hex = "\r\n";
    
    // Process data line by line
    for (size_t offset = 0; offset < size; offset += size_per_line)
    {
        // Print address (8 hex digits)
        char addr_buf[24];
        std::snprintf(addr_buf, sizeof(addr_buf), "%04zx  ", offset);
        hex += addr_buf;
        
        // Print hex bytes (16 bytes per line, with extra space after 8 bytes)
        size_t line_end = (offset + size_per_line < size) ? (offset + size_per_line) : size;
        for (size_t i = offset; i < line_end; ++i)
        {
            char hex_buf[4];
            std::snprintf(hex_buf, sizeof(hex_buf), "%02x ", array[i]);
            hex += hex_buf;
            
            // Add extra space after 8 bytes
            if ((i - offset + 1) % 8 == 0 && i + 1 < line_end)
            {
                hex += ' ';
            }
        }
        
        // Pad hex output if line is incomplete
        if (line_end < offset + size_per_line)
        {
            size_t padding = (size_per_line - (line_end - offset)) * 3; // 3 chars per byte: "xx "
            hex.append(padding, ' ');
        }
        
        // Print ASCII representation with | delimiters
        hex += " |";
        for (size_t i = offset; i < line_end; ++i)
        {
            uint8_t byte = array[i];
            if (std::isprint(static_cast<unsigned char>(byte)))
            {
                hex += static_cast<char>(byte);
            }
            else
            {
                hex += '.';
            }
        }
        if (line_end == size)
            hex += "|";
        else
            hex += "|\r\n";
    }

    std::string full_msg = msg + hex;
    // 遍历所有sink
    for (auto& sink : sinks_)
    {
        sink->log(name_, level, full_msg);
    }
}

void Logger::log_limited(std::string const &tag, int allowed_num, LogLevel level, std::string const &msg)
{
    int left = limited_allowed_left(tag, allowed_num);
    if (valid_ && (static_cast<int>(level) >= static_cast<int>(min_level_)) && (left > 0))
    {
        std::string final_msg = (left == 1) ? (msg + " (more messages will be suppressed)") : msg;
        // 遍历所有sink
        for (auto& sink : sinks_)
        {
            sink->log(name_, level, final_msg);
        }
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

/**
 * @brief 更新过滤等级
 * 计算所有sink的等级，更新min_level_和max_level_
 * min_level_为所有sink的等级中最小的, 比如一个SINK 是INFO，另一个是DEBUG，则min_level_为DEBUG
 * max_level_为所有sink的等级中最大的, 比如一个SINK 是INFO，另一个是DEBUG，则max_level_为INFO
 */
void Logger::update_filter_level()
{
    min_level_ = LogLevel::Off;
    max_level_ = LogLevel::Trace;
    for (auto& sink : sinks_)
    {
        if (sink)
        {
            LogLevel sink_level = sink->get_level();
            if (static_cast<int>(sink_level) < static_cast<int>(min_level_))
            {
                min_level_ = sink_level;
            }
            if (static_cast<int>(sink_level) > static_cast<int>(max_level_))
            {
                max_level_ = sink_level;
            }
        }
    }
    __debug("update filter level: min=%s, max=%s", log_level_name(min_level_), log_level_name(max_level_));
}

/**
 * @brief 设定所有规则等级
 * 
 * @param level 
 */
void Logger::set_rule_level(LogLevel level)
{
    __debug("set rule level to %s", log_level_name(level));
    for (auto& sink : sinks_){
        if (sink){
            sink->set_rule_level(level);
        }
    }
    update_filter_level();
}


std::shared_ptr<Logger> Logger::clone(std::string const & logger_name) const
{
    if (logger_name == name_)
    {
        return default_logger();
    }
    
    // 克隆所有sink
    std::vector<std::shared_ptr<LoggerSink>> cloned_sinks;
    for (auto& sink : sinks_)
    {
        if (sink)
        {
            auto cloned = sink->clone(logger_name);
            if (cloned)
            {
                // 重置规则等级，避免继承父logger的规则等级
                cloned->set_rule_level(LogLevel::Unknown);
                cloned_sinks.push_back(cloned);
            }
        }
    }
    
    if (cloned_sinks.empty())
    {
        return default_logger();
    }

    __debug("clone logger: %s", logger_name.c_str());

    auto logger = std::make_shared<Logger>();
    logger->name_ = logger_name;
    logger->sinks_ = cloned_sinks;
    logger->valid_ = true;
    logger->update_filter_level();
    register_logger(logger);

    return logger;
}

std::shared_ptr<Logger> Logger::clone(std::string const & logger_name, LogLevel level) const
{
    if (logger_name == name_)
    {
        return default_logger();
    }
    
    // 克隆所有sink
    std::vector<std::shared_ptr<LoggerSink>> cloned_sinks;
    for (auto& sink : sinks_)
    {
        if (sink)
        {
            auto cloned = sink->clone(logger_name);
            if (cloned)
            {
                cloned->set_level(level);
                cloned_sinks.push_back(cloned);
            }
        }
    }
    
    if (cloned_sinks.empty())
    {
        return default_logger();
    }

    __debug("clone logger: %s", logger_name.c_str());

    auto logger = std::make_shared<Logger>();
    logger->name_ = logger_name;
    logger->sinks_ = cloned_sinks;
    logger->valid_ = true;
    logger->update_filter_level();
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
     * @brief 检查注册表是否为空
     * 
     */
     bool empty() const {
        return registry_.empty();
    }


    /**
     * @brief 获取默认logger, 如果不存在，则创建一个
     * 
     * @return std::shared_ptr<Logger> 
     */
    std::shared_ptr<Logger> get_default(const std::string& logger_name = "default") 
    {
        std::shared_ptr<Logger> new_default_logger = nullptr;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (default_logger_) {
                return default_logger_;
            }
            // 如果注册表中有logger，使用第一个
            if (!registry_.empty()) {
                default_logger_ = registry_.begin()->second;
                return default_logger_;
            } else {
                default_logger_ = std::make_shared<Logger>(logger_name, std::make_shared<sink::Stdout>(LogLevel::Info));
                new_default_logger = default_logger_;
            }
        }

        // 里面有锁，这里不需要加锁
        register_logger(new_default_logger);
        return new_default_logger;
    }


        /**
     * @brief 注册一个logger
     * 
     * @param logger logger指针
     * @return true 成功
     * @return false 失败（logger为空）
     */
     bool register_logger(std::shared_ptr<Logger> logger) 
     { 
        if (!logger) {
            return false;
        }

        __debug("register logger: %s", logger->name().c_str());

        // 应用全局日志等级规则（如果存在匹配的规则）
        auto level = detail::LoggerRegistry::instance().get_logger_level_rule(logger->name());
        if (level != LogLevel::Unknown) {
            logger->set_rule_level(level);
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
    bool set_default(const std::string& name) 
    {
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
    std::shared_ptr<Logger> get_logger(const std::string& name) 
    {
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
    bool has_logger(const std::string& name) 
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return registry_.find(name) != registry_.end();
    }

    /**
     * @brief 移除一个logger
     * 
     * @param name logger名称
     */
    void drop_logger(const std::string& name) 
    {
        std::lock_guard<std::mutex> lock(mutex_);
        registry_.erase(name);
        // 如果被移除的是默认logger，重置默认logger
        if (default_logger_ && default_logger_->name() == name) {
            default_logger_.reset();
        }
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
    static std::string wildcard_to_regex(const std::string& pattern) 
    {
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
    void set_logger_level_rule(const std::string& pattern, LogLevel level) 
    {

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
    LogLevel get_logger_level_rule(const std::string& name) const 
    {
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

    /**
     * @brief 获取所有日志规则（包括精确匹配和正则匹配）
     * 
     * @return std::map<std::string, LogLevel> 所有规则的映射，key 为 pattern 字符串，value 为日志级别
     */
    std::map<std::string, LogLevel> get_logger_rules() const 
    {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // 创建结果映射，先添加精确匹配规则
        std::map<std::string, LogLevel> all_rules = level_rules_;
        
        // 添加正则匹配规则（使用原始 pattern 字符串）
        for (const auto& regex_rule : regex_level_rules_) {
            const std::string& pattern = std::get<0>(regex_rule);
            LogLevel level = std::get<2>(regex_rule);
            all_rules[pattern] = level;
        }
        
        return all_rules;
    }

private:
    /**
     * @brief 应用规则到已存在的 logger
     * 
     * @param pattern 规则模式（精确匹配或正则表达式）
     * @param level 日志等级（未使用，保留用于未来扩展）
     * @param is_regex 是否是正则表达式
     */
    void apply_rules_to_existing_loggers(const std::string& pattern, LogLevel  level, bool is_regex) 
    {
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
                        logger->set_rule_level(level);
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
                    logger->set_rule_level(level);
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

bool register_logger(std::shared_ptr<Logger> logger) 
{
    return detail::LoggerRegistry::instance().register_logger(logger);
}

bool set_default_logger(const std::string& name) 
{
    return detail::LoggerRegistry::instance().set_default(name);
}

std::shared_ptr<Logger> get_logger(const std::string& name) 
{
    // 先找一下，指定名称的logger是否有
    auto logger = detail::LoggerRegistry::instance().get_logger(name);
    if (logger) {
        return logger;
    }
    // 如果还没有创建logger， 就以这个名称创建一个默认的logger
    if (detail::LoggerRegistry::instance().empty()) {
        return detail::LoggerRegistry::instance().get_default(name);
    }
    // 否则，就从默认logger中复制一个
    return default_logger()->clone(name);
}

bool has_logger(const std::string& name) 
{
    return detail::LoggerRegistry::instance().has_logger(name);
}

void drop_logger(const std::string& name) 
{
    detail::LoggerRegistry::instance().drop_logger(name);
}

std::shared_ptr<Logger> make_logger(const std::string& name, std::shared_ptr<LoggerSink> sink) 
{
    auto logger = std::make_shared<Logger>(name, sink);
    if (logger && register_logger(logger))
    {
        return logger;
    }
    return nullptr;
}

std::shared_ptr<Logger> make_logger(std::string const &name, std::vector<std::shared_ptr<LoggerSink>> sinks)
{
    auto logger = std::make_shared<Logger>(name, sinks);
    if (logger && register_logger(logger))
    {
        return logger;
    }
    return nullptr;
}

std::shared_ptr<Logger> make_none_logger(std::string const &name)
{
    return make_logger(name, std::make_shared<sink::None>(LogLevel::Off));
}

std::shared_ptr<Logger> make_stdout_logger(std::string const &name, LogLevel level)
{
    return make_logger(name, std::make_shared<sink::Stdout>(level));
}

std::shared_ptr<Logger> make_file_logger(std::string const &name, std::string const &filepath, LogLevel level, bool to_stdout, bool flush_on_write)
{
    std::vector<std::shared_ptr<LoggerSink>> sinks;
    if (to_stdout) {
        sinks.push_back(std::make_shared<sink::Stdout>(level));
    }
    sinks.push_back(std::make_shared<sink::File>(level, filepath, 0, 1, flush_on_write));
    return make_logger(name, sinks);
}

std::shared_ptr<Logger> make_rotating_file_logger(std::string const &name, std::string const &filepath,
    LogLevel level, size_t max_file_size, size_t max_files, bool to_stdout, bool flush_on_write)
{
    std::vector<std::shared_ptr<LoggerSink>> sinks;
    if (to_stdout) {
        sinks.push_back(std::make_shared<sink::Stdout>(level));
    }
    sinks.push_back(std::make_shared<sink::File>(level, filepath, max_file_size, max_files, flush_on_write));
    return make_logger(name, sinks);
}

#ifdef BUILD_WITH_SPDLOG
std::shared_ptr<Logger> make_spdlog_logger(std::string const &name, LogLevel level, bool async)
{
    auto sink = std::make_shared<sink::Spdlog>(level, sink::SpdlogSinkType::ToConsole, "", async);
    return make_logger(name, sink);
}


std::shared_ptr<Logger> make_spdlog_logger(std::string const &name, std::string const &filepath, LogLevel level, bool to_console, bool async)
{
    if (filepath.empty()) {
        std::cerr << "**ERROR** make_spdlog_logger(): filepath is empty" << std::endl;
        return nullptr;
    }

    int sink_type = sink::SpdlogSinkType::ToFile;
    if (to_console) {
        sink_type |= sink::SpdlogSinkType::ToConsole;
    }

    auto sink = std::make_shared<sink::Spdlog>(level, sink_type, filepath, async);
    return make_logger(name, sink);
}
#endif // BUILD_WITH_SPDLOG


/**
 * @brief 设置 logger 的日志等级
 * 
 * @param name logger名称
 * @param level 日志等级
 */
void set_logger_level(const std::string& name, LogLevel level)
{
    detail::LoggerRegistry::instance().set_logger_level_rule(name, level);
}

/**
 * @brief 应用日志规则
 * 
 * @param rule_text 日志规则文本，格式为："pattern:level"，多个规则用逗号或分号分隔
 * @example
 * ```cpp
 * slog::apply_logger_rules("my_logger:debug;camera_.*:i;driver:trace");
 * ```
 * @return int 成功应用的规则数量
 */
int apply_logger_rules(const std::string& rule_text)
{
    if (rule_text.empty()) {
        return 0;
    }
    
    int applied_count = 0;
    std::string current_rule;
    
    // 遍历规则文本，分割规则（支持逗号和分号作为分隔符）
    for (size_t i = 0; i <= rule_text.length(); ++i) {
        char c = (i < rule_text.length()) ? rule_text[i] : '\0';
        
        // 遇到分隔符或字符串结尾，处理当前规则
        if (c == ',' || c == ';' || c == '\0') {
            if (!current_rule.empty()) {
                // 去除前后空白字符
                size_t start = current_rule.find_first_not_of(" \t\r\n");
                size_t end = current_rule.find_last_not_of(" \t\r\n");
                
                if (start != std::string::npos && end != std::string::npos) {
                    std::string trimmed_rule = current_rule.substr(start, end - start + 1);
                    
                    // 查找冒号分隔符
                    size_t colon_pos = trimmed_rule.find(':');
                    if (colon_pos != std::string::npos && colon_pos > 0 && colon_pos < trimmed_rule.length() - 1) {
                        // 提取 pattern 和 level
                        std::string pattern = trimmed_rule.substr(0, colon_pos);
                        std::string level_str = trimmed_rule.substr(colon_pos + 1);
                        
                        // 去除 pattern 和 level 的空白字符
                        size_t pattern_start = pattern.find_first_not_of(" \t\r\n");
                        size_t pattern_end = pattern.find_last_not_of(" \t\r\n");
                        size_t level_start = level_str.find_first_not_of(" \t\r\n");
                        size_t level_end = level_str.find_last_not_of(" \t\r\n");
                        
                        if (pattern_start != std::string::npos && pattern_end != std::string::npos &&
                            level_start != std::string::npos && level_end != std::string::npos) {
                            
                            pattern = pattern.substr(pattern_start, pattern_end - pattern_start + 1);
                            level_str = level_str.substr(level_start, level_end - level_start + 1);
                            
                            // 解析日志级别
                            LogLevel level = log_level_from_name(level_str, LogLevel::Unknown);
                            
                            if (level != LogLevel::Unknown) {
                                // 应用规则
                                set_logger_level(pattern, level);
                                applied_count++;
                                __debug("applied rule: pattern='%s', level='%s'", pattern.c_str(), level_str.c_str());
                            } else {
                                __debug("invalid log level in rule: '%s'", trimmed_rule.c_str());
                            }
                        }
                    } else {
                        __debug("invalid rule format (missing colon): '%s'", trimmed_rule.c_str());
                    }
                }
            }
            
            // 清空当前规则，准备处理下一个
            current_rule.clear();
        } else {
            // 累积当前规则的字符
            current_rule += c;
        }
    }
    
    return applied_count;
}

std::map<std::string, LogLevel> get_logger_rules()
{
    return detail::LoggerRegistry::instance().get_logger_rules();
}

} // namespace slog

