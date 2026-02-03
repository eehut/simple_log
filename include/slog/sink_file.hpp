#ifndef __SLOG_SINK_FILE_H__
#define __SLOG_SINK_FILE_H__

/**
 * @file sink_file.hpp
 * @author LiuChuansen (179712066@qq.com)
 * @brief File Sink实现
 * @version 0.1
 * @date 2025-12-19
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <fstream>
#include <memory>
#include <string>
#include <mutex>
#include "slog/slog.hpp"

namespace slog {
namespace sink {

/**
 * @brief 共享的文件状态，所有写入同一文件的sink共享此对象
 */
struct SharedFileState 
{
    std::ofstream file;
    size_t current_size = 0;
    size_t max_file_size = 0;
    size_t max_files = 0;
    bool flush_on_write = true;
    std::string filepath;
    
    SharedFileState(std::string const & path, size_t max_size, size_t max_file_count, bool flush);
};

/**
 * @brief File Sink实现
 * 
 * 线程安全：所有 File sink 共享一个文件路径对应的 mutex 和文件流对象，确保写入同一文件的日志不会交错
 * 
 * 特性：
 * - 支持多个logger或单个logger多线程使用
 * - 支持文件大小限制和自动rotation
 * - 支持自动刷新策略（每次写入后flush或延迟flush）
 * - 线程安全的文件操作
 * - 多个logger写入同一文件时共享文件流对象
 */
class File: public LoggerSink
{
public:
    /**
     * @brief 构造函数
     * @param level 日志等级
     * @param filepath 日志文件路径
     * @param max_file_size 最大文件大小（字节），0表示无限制，默认10MB
     * @param max_files 保留的旧日志文件数量，默认5个
     * @param flush_on_write 是否每次写入后立即刷新，默认true
     */
    explicit File(LogLevel level, 
                  std::string const & filepath,
                  size_t max_file_size = 10 * 1024 * 1024,  // 10MB
                  size_t max_files = 5,
                  bool flush_on_write = true)
        : LoggerSink(level)
        , filepath_(filepath)
        , max_file_size_(max_file_size)
        , max_files_(max_files)
        , flush_on_write_(flush_on_write)
    {
    }

    std::shared_ptr<LoggerSink> clone(const std::string & logger_name) const override;

    bool setup(const std::string & logger_name) override;

    const char* name() const override;

    ~File();

protected:
    void output(const std::string & logger_name, LogLevel level, std::string const &msg) override;

private:
    std::string filepath_;
    size_t max_file_size_;
    size_t max_files_;
    bool flush_on_write_;
    std::shared_ptr<SharedFileState> file_state_;

    /**
     * @brief 获取文件路径对应的全局 mutex
     * 
     * 使用文件路径作为key，确保写入同一文件的所有sink共享同一个mutex
     * 这样可以保证多个logger写入同一个文件时的线程安全
     */
    static std::mutex& get_file_mutex(std::string const & filepath);

    /**
     * @brief 获取或创建共享的文件状态
     * 
     * 所有写入同一文件的sink共享同一个文件状态对象
     * 注意：调用此函数前必须已获取file_mutex
     */
    static std::shared_ptr<SharedFileState> get_shared_file_state(
        std::string const & filepath, size_t max_file_size, size_t max_files, bool flush_on_write);

    /**
     * @brief 格式化日志消息
     */
    std::string format_log_message(const std::string & logger_name, LogLevel level, std::string const &msg);

    /**
     * @brief 执行文件rotation
     * 
     * 将当前日志文件重命名为 filename.1，并将旧的文件依次重命名
     * filename.1 -> filename.2, filename.2 -> filename.3, ...
     * 删除最旧的文件（如果超过max_files_）
     * 注意：调用此函数前必须已获取file_mutex
     */
    void rotate_files();
};

} // namespace sink
} // namespace slog

#endif // __SLOG_SINK_FILE_H__
