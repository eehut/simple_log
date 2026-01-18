#ifndef __SLOG_H__
#define __SLOG_H__

/**
 * @file slog.hpp
 * @author Liu Chuansen (179712066@qq.com)
 * @brief 日志库头文件，支持stdout输出
 * @version 0.2
 * @date 2023-05-25
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <mutex>
#include <unordered_map>
#include <map>


#ifdef BUILD_WITH_LIBFMT
// Use system fmt library
#include <fmt/format.h>
#else 
// Use bundled fmt library
#include <slog/fmt/format.h>
#endif 

/// 版本号定义
#define SLOG_VERSION_MAJOR 0
#define SLOG_VERSION_MINOR 3
#define SLOG_VERSION_STRING "0.3"

namespace slog 
{

/// 日志等级
enum class LogLevel : int 
{
    Trace = 0,
    Debug,
    Info,
    Warning,
    Error,
    Off,
    Unknown = Off + 2,
};

/**
 * @brief 获取日志等级的名称
 * 
 * @param level 
 * @return const char* 
 */
constexpr const char *log_level_name(LogLevel level) noexcept
{
    switch(level)
    {
        case LogLevel::Trace:
        return "TRACE";
        case LogLevel::Debug:
        return "DEBUG";
        case LogLevel::Info:
        return "INFO";
        case LogLevel::Warning:
        return "WARN";
        case LogLevel::Error:
        return "ERROR";
        default:
        return "";
    }
}

/**
 * @brief 获取日志等级的简称
 * 
 * @param level 
 * @return char 
 */
constexpr char log_level_short_name(LogLevel level) noexcept
{
    constexpr const char brief[] = "TDIWEON";
    constexpr size_t brief_size = sizeof(brief) - 1; // 减去 '\0'
    int index = static_cast<int>(level);
    return (index >= 0 && static_cast<size_t>(index) < brief_size) ? brief[index] : '-';
}

/**
 * @brief 将字串转换日志名称，支持短名称如T D, d等
 * 
 * @param level 
 * @param default_level 默认值
 * @return LogLevel 
 */
inline LogLevel log_level_from_name(std::string const &level, LogLevel default_level = LogLevel::Unknown)
{
    if (level.empty())
    {
        return default_level;
    }

    // 先转成全小写（原地转换，避免额外拷贝）
    std::string in = level;
    for (auto & c : in){
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    constexpr const char* level_names[] = {"trace", "debug", "info", "warning", "error"};
    constexpr LogLevel levels[] = {LogLevel::Trace, LogLevel::Debug, LogLevel::Info, LogLevel::Warning, LogLevel::Error};
    constexpr size_t level_count = sizeof(level_names) / sizeof(level_names[0]);

    // 比较每个字串
    for (size_t i = 0; i < level_count; ++i)
    {
        if ((in.size() == 1) && (level_names[i][0] == in[0]))
        {
            return levels[i];
        }

        if (level_names[i] == in)
        {
            return levels[i];
        }
    }

    return default_level;
}

/**
 * @brief 一个日志SINK接口
 * 
 */
class LoggerSink
{
public:    
    virtual ~LoggerSink() = default;

    // 克隆一个SINK，必须保证名称不一样，如果是一样，则返回空指针
    virtual std::shared_ptr<LoggerSink> clone(std::string const & logger_name) const = 0;

    virtual bool setup(std::string const & logger_name) = 0;
    virtual void log(LogLevel level, std::string const & msg) = 0;
    virtual void set_level(LogLevel level) = 0;
    virtual LogLevel get_level() const = 0;
    virtual const char * name() const = 0;
};

/**
 * @brief Logger对象声明
 * 
 */
class Logger
{

public:
    /// 共享指针
    using SharedPtr = std::shared_ptr<Logger>; 

    /// @brief 默认构造函数
    Logger() : valid_(false) {}

    /// @brief 单sink构造函数
    /// @param name logger名称
    /// @param sink sink指针
    Logger(std::string const &name, std::shared_ptr<LoggerSink> sink = nullptr);
    
    /// @brief 多sink构造函数
    /// @param name logger名称
    /// @param sinks sink列表
    Logger(std::string const &name, std::vector<std::shared_ptr<LoggerSink>> sinks);

    // 禁止复制构造
    Logger(Logger const &) = delete;
    Logger & operator=(Logger const &) = delete;

    ~Logger() = default;

    /// @brief 返回名称
    /// @return 
    std::string const &name() const;

    /// @brief 返回日志等级
    /// @return 
    LogLevel get_level() const;

    /// @brief 设置日志等级，将修改所有sink的等级
    /// @param level 
    void set_level(LogLevel level);

    /// @brief 是否允许打印指定等级, 受限使用，单sink有效，多sink,取决于等级值最小的。
    /// @param level 
    /// @return 
    bool is_allowed(LogLevel level) const noexcept;

    /// @brief 显示指定日志
    /// @param level 日志等级
    /// @param msg 日志消息
    void log(LogLevel level, std::string const &msg);

    /// @brief 显示指定日志（const char*版本，避免字符串拷贝）
    /// @param level 日志等级
    /// @param msg 日志消息
    void log(LogLevel level, const char* msg);

    /// @brief 显示多行日志，自动识别换行符并逐行输出
    /// @param level 日志等级
    /// @param msg 日志消息，可能包含 \r\n 或 \n 换行符
    void log_lines(LogLevel level, std::string const &msg);

    /// @brief 显示十六进制数据
    /// @param level 日志等级
    /// @param data 数据地址
    /// @param size 数据大小
    /// @param msg 日志消息
    void log_data(LogLevel level, void const *data, size_t size, std::string const &msg);

    /// @brief 显示vector中的数据
    /// @param level 
    /// @param data 
    /// @param msg 
    void log_data(LogLevel level, std::vector<uint8_t> const & data, std::string const &msg);

    /// @brief 具有限制属性的日志输出方法
    /// @param tag 限制的标签
    /// @param allowed_num 允许打印的日志数量
    /// @param level 日志等级
    /// @param msg 日志消息
    void log_limited(std::string const &tag, int allowed_num, LogLevel level, std::string const &msg);

    /// @brief 克隆一个Logger对象，从当前sink fork出新的logger
    /// @param logger_name 新logger的名称
    /// @return std::shared_ptr<Logger> 新的logger对象，如果克隆失败返回nullptr
    std::shared_ptr<Logger> clone(std::string const & logger_name) const;

    /// @brief 克隆一个Logger对象，从当前sink fork出新的logger，并设置指定的日志等级
    /// @param logger_name 新logger的名称
    /// @param level 新logger的日志等级
    /// @return std::shared_ptr<Logger> 新的logger对象，如果克隆失败返回nullptr
    std::shared_ptr<Logger> clone(std::string const & logger_name, LogLevel level) const;

    // 以下是基础函数
    template<typename... Args>
    void log(LogLevel level, fmt::format_string<Args...> fmt, Args &&... args)
    {
        log(level, fmt::format(fmt, std::forward<Args>(args)...));        
    }

    template<typename... Args>
    void dump(LogLevel level, void const *data, size_t size, fmt::format_string<Args...> fmt, Args &&... args)
    {
        log_data(level, data, size, fmt::format(fmt, std::forward<Args>(args)...));        
    }

    template<typename... Args>
    void dump(LogLevel level, std::vector<uint8_t> const & data, fmt::format_string<Args...> fmt, Args &&... args)
    {
        log_data(level, data, fmt::format(fmt, std::forward<Args>(args)...));        
    }

    /// 以下是便捷函数

    template<typename... Args>
    void trace(fmt::format_string<Args...> fmt, Args &&... args)
    {
        log(LogLevel::Trace, fmt::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void debug(fmt::format_string<Args...> fmt, Args &&... args)
    {
        log(LogLevel::Debug, fmt::format(fmt, std::forward<Args>(args)...));        
    }

    template<typename... Args>
    void info(fmt::format_string<Args...> fmt, Args &&... args)
    {
        log(LogLevel::Info, fmt::format(fmt, std::forward<Args>(args)...));        
    }    

    template<typename... Args>
    void warning(fmt::format_string<Args...> fmt, Args &&... args)
    {
        log(LogLevel::Warning, fmt::format(fmt, std::forward<Args>(args)...));        
    }    

    template<typename... Args>
    void error(fmt::format_string<Args...> fmt, Args &&... args)
    {
        log(LogLevel::Error, fmt::format(fmt, std::forward<Args>(args)...));        
    }

    // 日志抑制
    void reset_limited(std::string const & tag)
    {
        auto it = limited_items_.find(tag);
        if (it != limited_items_.end()){
            it->second.count = 0;
        }
    }

    template<typename... Args>
    void trace_limited(std::string const &tag, int allowed_num, fmt::format_string<Args...> fmt, Args &&... args)
    {
        log_limited(tag, allowed_num, LogLevel::Trace, fmt::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void debug_limited(std::string const &tag, int allowed_num, fmt::format_string<Args...> fmt, Args &&... args)
    {
        log_limited(tag, allowed_num, LogLevel::Debug, fmt::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void info_limited(std::string const &tag, int allowed_num, fmt::format_string<Args...> fmt, Args &&... args)
    {
        log_limited(tag, allowed_num, LogLevel::Info, fmt::format(fmt, std::forward<Args>(args)...));
    }    

    template<typename... Args>
    void warning_limited(std::string const &tag, int allowed_num, fmt::format_string<Args...> fmt, Args &&... args)
    {
        log_limited(tag, allowed_num, LogLevel::Warning, fmt::format(fmt, std::forward<Args>(args)...));
    }    

    template<typename... Args>
    void error_limited(std::string const &tag, int allowed_num, fmt::format_string<Args...> fmt, Args &&... args)
    {
        log_limited(tag, allowed_num, LogLevel::Error, fmt::format(fmt, std::forward<Args>(args)...));        
    }


private:
    std::string name_;
    std::vector<std::shared_ptr<LoggerSink>> sinks_;
    bool valid_;
    LogLevel min_level_; // 过滤等级，如果为Off，则不进行过滤
    LogLevel max_level_; // 最大等级，如果为Off，则不进行过滤

    /// 用来管理日志抑制
    struct LimitedControl
    {
        int allowed;
        int count;
    };
    std::map<std::string, LimitedControl> limited_items_;

    /// @brief 检测日志是否允许打印，如果该抑制TAG未创建，将自动创建
    /// @param tag 抑制的标签
    /// @param allowed_num 允许打印的日志数量
    int limited_allowed_left(std::string const &tag, int allowed_num);

    /// @brief 计算最小等级
    /// @return 最小等级
    void update_filter_level();

};


/**
* @brief 获取库版本号
* 
* @return const char* 版本号字符串，格式为 "major.minor"
*/
inline const char* version() noexcept
{
    return SLOG_VERSION_STRING;
}

/**
* @brief 获取默认logger
* 
* @return std::shared_ptr<Logger> 
*/
std::shared_ptr<Logger> default_logger();

/**
 * @brief 从默认logger中克隆一个
 * 
 * @param name 新的logger的名称
 * @return std::shared_ptr<Logger> 
 */
inline std::shared_ptr<Logger> clone(const std::string& name)
{
    return default_logger()->clone(name);
}

/**
* @brief 注册一个logger到全局注册表
* 
* @param logger logger指针
* @return true 成功
* @return false 失败
*/
bool register_logger(std::shared_ptr<Logger> logger);

/**
* @brief 设置默认logger
* 
* @param name logger名称
* @return true 成功
* @return false 失败（logger不存在）
*/
bool set_default_logger(const std::string& name);

/**
* @brief 获取指定名称的logger，如果不存在，则从默认logger中clone一个。
*        如果默认logger不存在，它会以此名称创建一个默认logger
* @param name logger名称
* @return std::shared_ptr<Logger> logger指针，总是能返回一个可用的logger
*/
std::shared_ptr<Logger> get_logger(const std::string& name);

/**
* @brief 检查logger是否存在
* 
* @param name logger名称
* @return true 存在
* @return false 不存在
*/
bool has_logger(const std::string& name);

/**
* @brief 移除一个logger
* 
* @param name logger名称
*/
void drop_logger(const std::string& name);

/**
* @brief 创建并注册一个logger
* 
* @param name logger名称
* @param sink sink指针，如果为空则使用默认stdout sink
* @return std::shared_ptr<Logger> logger指针
*/
std::shared_ptr<Logger> make_logger(const std::string& name, std::shared_ptr<LoggerSink> sink = nullptr);

/**
 * @brief 创建一个通用的多sink logger
 * 
 * @param name logger名称
 * @param sinks sink列表
 * @return std::shared_ptr<Logger> 
 */
 std::shared_ptr<Logger> make_logger(std::string const &name, std::vector<std::shared_ptr<LoggerSink>> sinks);

/**
* @brief 创建一个空SINK，不输出任何信息
* 
* @param name logger名称
* @return std::shared_ptr<Logger> 
*/
std::shared_ptr<Logger> make_none_logger(std::string const &name);

/**
* @brief 创建一个标准输出的日志
* 
* @param name logger名称
* @param level 日志等级
* @return std::shared_ptr<Logger> 
*/
std::shared_ptr<Logger> make_stdout_logger(std::string const &name, LogLevel level = LogLevel::Info);


/**
 * @brief 创建一个文件日志logger
 * 
 * @param name logger名称
 * @param filepath 日志文件路径
 * @param level 日志等级，默认Info
 * @param to_stdout 是否输出到stdout，默认false
 * @param flush_on_write 是否每次写入后立即刷新，默认true
 * @return std::shared_ptr<Logger> 
 */
std::shared_ptr<Logger> make_file_logger(std::string const &name, 
    std::string const &filepath, 
    LogLevel level = LogLevel::Info, 
    bool to_stdout = false, 
    bool flush_on_write = true);


/**
 * @brief 创建一个自动轮转的文件日志logger
 * 
 * @param name logger名称
 * @param filepath 日志文件路径
 * @param level 日志等级，默认Info
 * @param max_file_size 最大文件大小（字节），默认10MB
 * @param max_files 保留的旧日志文件数量，默认5个
 * @param to_stdout 是否输出到stdout，默认true
 * @param flush_on_write 是否每次写入后立即刷新，默认true
 * @return std::shared_ptr<Logger> 
 */
std::shared_ptr<Logger> make_rotating_file_logger(std::string const &name, 
    std::string const &filepath, 
    LogLevel level = LogLevel::Info, 
    size_t max_file_size = 10 * 1024 * 1024, 
    size_t max_files = 5, 
    bool to_stdout = false, 
    bool flush_on_write = true);

#ifdef BUILD_WITH_SPDLOG
/**
 * @brief 创建 spdlog console logger
 * 
 * @param name logger名称
 * @param level 日志等级，默认Info
 * @param async 是否使用异步模式，默认false（同步）
 * @return std::shared_ptr<Logger> 
 */
std::shared_ptr<Logger> make_spdlog_logger(std::string const &name, 
    LogLevel level = LogLevel::Info,
    bool async = false);

/**
 * @brief 创建 spdlog file logger
 * 
 * @param name logger名称
 * @param filepath 日志文件路径
 * @param level 日志等级，默认Info
 * @param async 是否使用异步模式，默认false（同步）
 * @return std::shared_ptr<Logger> 
 */
std::shared_ptr<Logger> make_spdlog_logger(std::string const &name,
    std::string const &filepath,
    LogLevel level = LogLevel::Info,
    bool to_console = false,
    bool async = false);
#endif // BUILD_WITH_SPDLOG


/**
* @brief 设置全局日志等级规则
* 
* 设置一个日志等级规则，该规则会在以下情况生效：
* 1. 调用此函数时，会立即应用到所有已存在的匹配的 logger
* 2. 创建新的 logger 时，如果名称匹配，会自动应用此规则
* 
* 支持两种匹配模式：
* - 精确匹配：直接使用 logger 名称，如 "my_logger"
* - 正则表达式匹配：使用正则表达式模式，如 ".*_debug" 可以匹配所有以 "_debug" 结尾的 logger
*   正则表达式使用 ECMAScript 语法（C++ std::regex）
* 
* 优先级：精确匹配 > 正则匹配（按添加顺序，第一个匹配的正则规则生效）
* 
* 注意：如果 logger 已经通过 set_level() 手动设置过等级，则不会被全局规则覆盖
* 
* @param pattern logger名称或正则表达式模式（支持多次调用设置不同的规则）
* @param level 日志等级
* 
* @example
* ```cpp
* // 精确匹配
* slog::set_logger_level("my_logger", slog::LogLevel::Debug);
* 
* // 正则表达式匹配：匹配所有以 "_debug" 结尾的 logger
* slog::set_logger_level(".*_debug", slog::LogLevel::Trace);
* 
* // 正则表达式匹配：匹配所有以 "camera_" 开头的 logger
* slog::set_logger_level("^camera_.*", slog::LogLevel::Info);
* ```
*/
void set_logger_level(const std::string& pattern, LogLevel level);

/**
 * @brief 应用日志规则
 * 
 * @param rule_text 日志规则文本，格式为："pattern:level"，多个规则用逗号或分号分隔, level支持trace,debug,info,warning,error, 大小写可以混用
 * @example
 * ```cpp
 * slog::apply_logger_rules("my_logger:debug;camera_.*:info;driver:trace");
 * ```
 */
int apply_logger_rules(const std::string& rule_text);

/**
 * @brief 获取所有日志规则
 * 
 * @return std::map<std::string, LogLevel> 日志规则
 */
std::map<std::string, LogLevel> get_logger_rules();


template<typename... Args>
inline void log(LogLevel level, fmt::format_string<Args...> fmt, Args &&...args)
{
    default_logger()->log(level, fmt::format(fmt, std::forward<Args>(args)...));
}

/**
 * @brief 全局多行日志函数，自动识别换行符并逐行输出
 * @param level 日志等级
 * @param msg 日志消息，可能包含 \r\n 或 \n 换行符
 */
inline void log_lines(LogLevel level, std::string const &msg)
{
    default_logger()->log_lines(level, msg);
}

template<typename... Args>
inline void dump(LogLevel level, void const *data, size_t size, fmt::format_string<Args...> fmt, Args &&... args)
{
    default_logger()->log_data(level, data, size, fmt::format(fmt, std::forward<Args>(args)...));        
}

template<typename... Args>
inline void dump(LogLevel level, std::vector<uint8_t> const & data, fmt::format_string<Args...> fmt, Args &&... args)
{
    default_logger()->log_data(level, data, fmt::format(fmt, std::forward<Args>(args)...));        
}

template<typename... Args>
inline void trace(fmt::format_string<Args...> fmt, Args &&...args)
{
    default_logger()->log(LogLevel::Trace, fmt::format(fmt, std::forward<Args>(args)...));
}

template<typename... Args>
inline void debug(fmt::format_string<Args...> fmt, Args &&...args)
{
    default_logger()->log(LogLevel::Debug, fmt::format(fmt, std::forward<Args>(args)...));
}

template<typename... Args>
inline void info(fmt::format_string<Args...> fmt, Args &&...args)
{
    default_logger()->log(LogLevel::Info, fmt::format(fmt, std::forward<Args>(args)...));
}

template<typename... Args>
inline void warning(fmt::format_string<Args...> fmt, Args &&...args)
{
    default_logger()->log(LogLevel::Warning, fmt::format(fmt, std::forward<Args>(args)...));
}

template<typename... Args>
inline void error(fmt::format_string<Args...> fmt, Args &&...args)
{
    default_logger()->log(LogLevel::Error, fmt::format(fmt, std::forward<Args>(args)...));
}

template<typename... Args>
inline void trace_limited(std::string const &tag, int allowed_num, fmt::format_string<Args...> fmt, Args &&...args)
{
    default_logger()->log_limited(tag, allowed_num, LogLevel::Trace, fmt::format(fmt, std::forward<Args>(args)...));
}

template<typename... Args>
inline void debug_limited(std::string const &tag, int allowed_num, fmt::format_string<Args...> fmt, Args &&...args)
{
    default_logger()->log_limited(tag, allowed_num, LogLevel::Debug, fmt::format(fmt, std::forward<Args>(args)...));
}

template<typename... Args>
inline void info_limited(std::string const &tag, int allowed_num, fmt::format_string<Args...> fmt, Args &&...args)
{
    default_logger()->log_limited(tag, allowed_num, LogLevel::Info, fmt::format(fmt, std::forward<Args>(args)...));
}

template<typename... Args>
inline void warning_limited(std::string const &tag, int allowed_num, fmt::format_string<Args...> fmt, Args &&...args)
{
    default_logger()->log_limited(tag, allowed_num, LogLevel::Warning, fmt::format(fmt, std::forward<Args>(args)...));
}

template<typename... Args>
inline void error_limited(std::string const &tag, int allowed_num, fmt::format_string<Args...> fmt, Args &&...args)
{
    default_logger()->log_limited(tag, allowed_num, LogLevel::Error, fmt::format(fmt, std::forward<Args>(args)...));
}

} // namespace slog

// Suppress warning about GNU extension for variadic macros
// This is needed for compatibility with Clang on macOS
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif

/**
 * @brief 日志宏，支持编译时格式字符串检查
 * 
 * 使用这些宏可以在编译时检查格式字符串和参数是否匹配。
 * 这些宏内部使用 FMT_STRING 来启用编译时检查。
 * 
 * @example
 * ```cpp
 * SLOG_INFO("User {} logged in", username);
 * SLOG_ERROR("Failed to connect: {}", error_code);
 * ```
 */
#define SLOG_TRACE(fmt, ...) \
    slog::trace(FMT_STRING(fmt), ##__VA_ARGS__)

#define SLOG_DEBUG(fmt, ...) \
    slog::debug(FMT_STRING(fmt), ##__VA_ARGS__)

#define SLOG_INFO(fmt, ...) \
    slog::info(FMT_STRING(fmt), ##__VA_ARGS__)

#define SLOG_WARNING(fmt, ...) \
    slog::warning(FMT_STRING(fmt), ##__VA_ARGS__)

#define SLOG_ERROR(fmt, ...) \
    slog::error(FMT_STRING(fmt), ##__VA_ARGS__)


#define LOCAL_TRACE(local_logger, fmt, ...) \
    local_logger->trace(FMT_STRING(fmt), ##__VA_ARGS__)

#define LOCAL_DEBUG(local_logger, fmt, ...) \
    local_logger->debug(FMT_STRING(fmt), ##__VA_ARGS__)

#define LOCAL_INFO(local_logger, fmt, ...) \
    local_logger->info(FMT_STRING(fmt), ##__VA_ARGS__)

#define LOCAL_WARNING(local_logger, fmt, ...) \
    local_logger->warning(FMT_STRING(fmt), ##__VA_ARGS__)

#define LOCAL_ERROR(local_logger, fmt, ...) \
    local_logger->error(FMT_STRING(fmt), ##__VA_ARGS__)

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif  // __SLOG_H__
