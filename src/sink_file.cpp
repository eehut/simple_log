/**
 * @file sink_file.cpp
 * @author LiuChuansen (179712066@qq.com)
 * @brief File Sink实现
 * @version 0.1
 * @date 2025-12-19
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <unordered_map>

#include "slog/sink_file.hpp"

namespace slog {
namespace sink {

// SharedFileState implementation
SharedFileState::SharedFileState(std::string const & path, size_t max_size, size_t max_file_count, bool flush)
    : max_file_size(max_size)
    , max_files(max_file_count)
    , flush_on_write(flush)
    , filepath(path)
{
}

// File implementation
File::File(LogLevel level, 
          std::string const & filepath,
          size_t max_file_size,
          size_t max_files,
          bool flush_on_write) 
    : level_(level)
    , filepath_(filepath)
    , max_file_size_(max_file_size)
    , max_files_(max_files)
    , flush_on_write_(flush_on_write)
{
}

File::~File() 
{
    // 不需要在析构函数中关闭文件，因为文件状态是共享的
    // 文件会在最后一个shared_ptr被销毁时自动关闭
}

std::shared_ptr<LoggerSink> File::clone(std::string const & logger_name) const 
{
    // 如果名称一样，则返回空
    if (logger_name == name_) 
    {
        return nullptr;
    }

    auto sink = std::make_shared<File>(level_, filepath_, max_file_size_, max_files_, flush_on_write_);
    sink->setup(logger_name);
    return sink;
}

bool File::setup(std::string const & name) 
{
    name_ = name;
    
    // 获取或创建共享文件状态
    std::lock_guard<std::mutex> lock(get_file_mutex(filepath_));
    file_state_ = get_shared_file_state(filepath_, max_file_size_, max_files_, flush_on_write_);
    
    // 如果文件还没打开，则打开它
    if (!file_state_->file.is_open()) 
    {
        // 检查文件是否已存在，如果存在则获取当前大小
        if (std::filesystem::exists(filepath_)) 
        {
            try 
            {
                file_state_->current_size = std::filesystem::file_size(filepath_);
            } 
            catch (...) 
            {
                file_state_->current_size = 0;
            }
        }
        
        // 以追加模式打开文件
        file_state_->file.open(filepath_, std::ios::out | std::ios::app);
        if (!file_state_->file.is_open()) 
        {
            return false;
        }
    }
    
    return true;
}

void File::log(LogLevel level, std::string const &msg) 
{    
    // 等级不允许输出
    if (static_cast<int>(level) < static_cast<int>(level_)) 
    {
        return;
    }
    
    if (!file_state_) 
    {
        return;
    }
    
    // 格式化日志消息
    std::string formatted_msg = format_log_message(level, msg);
    
    // 使用文件路径对应的mutex保护文件写入
    std::lock_guard<std::mutex> lock(get_file_mutex(filepath_));
    
    // 检查是否需要rotation
    if (file_state_->max_file_size > 0 && 
        file_state_->current_size + formatted_msg.size() > file_state_->max_file_size) 
    {
        rotate_files();
    }
    
    // 写入文件
    if (file_state_->file.is_open()) 
    {
        file_state_->file << formatted_msg;
        file_state_->current_size += formatted_msg.size();
        
        // 根据配置决定是否立即刷新
        if (file_state_->flush_on_write) 
        {
            file_state_->file.flush();
        }
    }
}

void File::set_level(LogLevel level) 
{
    level_ = level;
}

LogLevel File::get_level() const 
{ 
    return level_; 
}

const char* File::name() const 
{ 
    return "File"; 
}

std::mutex& File::get_file_mutex(std::string const & filepath) 
{
    static std::mutex map_mutex;
    static std::unordered_map<std::string, std::unique_ptr<std::mutex>> file_mutexes;
    
    std::lock_guard<std::mutex> lock(map_mutex);
    auto it = file_mutexes.find(filepath);
    if (it == file_mutexes.end()) 
    {
        file_mutexes[filepath] = std::make_unique<std::mutex>();
        return *file_mutexes[filepath];
    }
    return *it->second;
}

std::shared_ptr<SharedFileState> File::get_shared_file_state(
    std::string const & filepath,
    size_t max_file_size,
    size_t max_files,
    bool flush_on_write)
{
    static std::unordered_map<std::string, std::weak_ptr<SharedFileState>> file_states;
    
    auto it = file_states.find(filepath);
    if (it != file_states.end()) 
    {
        auto state = it->second.lock();
        if (state) 
        {
            return state;
        }
    }
    
    // 创建新的共享文件状态
    auto state = std::make_shared<SharedFileState>(filepath, max_file_size, max_files, flush_on_write);
    file_states[filepath] = state;
    return state;
}

std::string File::format_log_message(LogLevel level, std::string const &msg) 
{
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);
    std::ostringstream oss;
    
    // output timestamp
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S.");
    oss << std::setfill('0') << std::setw(3) << ms % 1000;
    
    // output log level and logger name
    oss << " <" << log_level_name(level) << "> (" << name_ << ") ";
    oss << msg << "\n";
    
    return oss.str();
}

void File::rotate_files() 
{
    if (!file_state_) 
    {
        return;
    }
    
    if (file_state_->file.is_open()) 
    {
        file_state_->file.flush();
        file_state_->file.close();
    }
    
    namespace fs = std::filesystem;
    
    // 删除最旧的文件
    if (file_state_->max_files > 0) 
    {
        std::string oldest_file = filepath_ + "." + std::to_string(file_state_->max_files);
        if (fs::exists(oldest_file)) 
        {
            try 
            {
                fs::remove(oldest_file);
            } 
            catch (...) 
            {
                // 忽略删除错误
            }
        }
    }
    
    // 将旧文件依次重命名
    for (size_t i = file_state_->max_files; i > 0; --i) 
    {
        std::string old_name = (i == 1) ? filepath_ : (filepath_ + "." + std::to_string(i - 1));
        std::string new_name = filepath_ + "." + std::to_string(i);
        
        if (fs::exists(old_name)) 
        {
            try 
            {
                fs::rename(old_name, new_name);
            } 
            catch (...) 
            {
                // 忽略重命名错误
            }
        }
    }
    
    // 重新打开文件
    file_state_->file.open(filepath_, std::ios::out | std::ios::app);
    file_state_->current_size = 0;
}

} // namespace sink
} // namespace slog
