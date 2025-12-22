
/**
 * @file test_all.cpp
 * @author Liu Chuansen (179712066@qq.com)
 * @brief Test all the features of the slog library
 * @version 0.2
 * @date 2025-12-04
 * 
 * @copyright Copyright (c) 2025 Liu Chuansen
 * 
 */



#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>

#include <slog/slog.hpp>
#include <slog/sink_file.hpp>

// Test basic logger creation and logging
void test_basic_logging() {
    std::cout << "\n=== Test 1: Basic Logging ===" << std::endl;
    
    auto logger = slog::make_stdout_logger("test_basic", slog::LogLevel::Trace);
    
    logger->trace("This is a trace message");
    logger->debug("This is a debug message");
    logger->info("This is an info message");
    logger->warning("This is a warning message");
    logger->error("This is an error message");
}

// Test log level filtering
void test_log_level_filtering() {
    std::cout << "\n=== Test 2: Log Level Filtering ===" << std::endl;
    
    auto logger = slog::make_stdout_logger("test_filter", slog::LogLevel::Warning);
    
    std::cout << "Setting log level to Warning, only Warning and Error should appear:" << std::endl;
    logger->trace("Trace message (should not appear)");
    logger->debug("Debug message (should not appear)");
    logger->info("Info message (should not appear)");
    logger->warning("Warning message (should appear)");
    logger->error("Error message (should appear)");
}

// Test formatted logging
void test_formatted_logging() {
    std::cout << "\n=== Test 3: Formatted Logging ===" << std::endl;
    
    auto logger = slog::make_stdout_logger("test_format", slog::LogLevel::Debug);
    
    int value = 42;
    double pi = 3.14159;
    const char* name = "slog";
    std::string msg = "world";
    
    std::cout << "{} style formatting:" << std::endl;
    logger->info("hello, this is {}", msg);
    logger->info("Integer value: {}", value);
    logger->info("Double value: {}", pi);
    logger->info("String value: {}", name);
    logger->debug("Combined: {} = {}, pi = {}", name, value, pi);
    
    std::cout << "\nEscaped braces ({{ and }}):" << std::endl;
    logger->info("Literal braces: {{ and }}");
    logger->info("Mixed: value is {} and braces are {{}}", value);
    logger->info("Nested: {{value: {}}}", value);
}

// Test hex dump functionality
void test_hex_dump() {
    std::cout << "\n=== Test 4: Hex Dump ===" << std::endl;
    
    auto logger = slog::make_stdout_logger("test_dump", slog::LogLevel::Debug);
    
    // Test with raw data
    uint8_t data[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                      0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                      0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                      0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F};
    
    logger->debug("Dumping raw data:");
    logger->dump(slog::LogLevel::Debug, data, sizeof(data), "Raw data dump: ");
    
    // Test with vector
    std::vector<uint8_t> vec = {0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA};
    logger->debug("Dumping vector data:");
    logger->dump(slog::LogLevel::Debug, vec, "Vector data dump: ");
}

// Test logger registry
void test_logger_registry() {
    std::cout << "\n=== Test 5: Logger Registry ===" << std::endl;
    
    // Create and register loggers
    auto logger1 = slog::make_stdout_logger("logger1", slog::LogLevel::Info);
    auto logger2 = slog::make_stdout_logger("logger2", slog::LogLevel::Debug);
    auto logger3 = slog::make_stdout_logger("logger3", slog::LogLevel::Warning);
    
    // Test has_logger
    std::cout << "Checking if loggers exist:" << std::endl;
    std::cout << "  logger1 exists: " << (slog::has_logger("logger1") ? "yes" : "no") << std::endl;
    std::cout << "  logger2 exists: " << (slog::has_logger("logger2") ? "yes" : "no") << std::endl;
    std::cout << "  logger_nonexist exists: " << (slog::has_logger("logger_nonexist") ? "yes" : "no") << std::endl;
    
    // Test get_logger
    auto retrieved = slog::get_logger("logger1");
    if (retrieved) {
        retrieved->info("Retrieved logger1 successfully");
    }
    
    // Test set_default_logger
    slog::set_default_logger("logger2");
    auto default_log = slog::default_logger();
    if (default_log) {
        default_log->info("This is from default logger (logger2)");
    }
    
    // Test drop_logger
    slog::drop_logger("logger3");
    std::cout << "After dropping logger3, exists: " << (slog::has_logger("logger3") ? "yes" : "no") << std::endl;
}

// Test log level utilities
void test_log_level_utilities() {
    std::cout << "\n=== Test 6: Log Level Utilities ===" << std::endl;
    
    // Test log_level_name
    std::cout << "Log level names:" << std::endl;
    std::cout << "  Trace: " << slog::log_level_name(slog::LogLevel::Trace) << std::endl;
    std::cout << "  Debug: " << slog::log_level_name(slog::LogLevel::Debug) << std::endl;
    std::cout << "  Info: " << slog::log_level_name(slog::LogLevel::Info) << std::endl;
    std::cout << "  Warning: " << slog::log_level_name(slog::LogLevel::Warning) << std::endl;
    std::cout << "  Error: " << slog::log_level_name(slog::LogLevel::Error) << std::endl;
    
    // Test log_level_short_name
    std::cout << "Log level short names:" << std::endl;
    std::cout << "  Trace: " << slog::log_level_short_name(slog::LogLevel::Trace) << std::endl;
    std::cout << "  Debug: " << slog::log_level_short_name(slog::LogLevel::Debug) << std::endl;
    std::cout << "  Info: " << slog::log_level_short_name(slog::LogLevel::Info) << std::endl;
    std::cout << "  Warning: " << slog::log_level_short_name(slog::LogLevel::Warning) << std::endl;
    std::cout << "  Error: " << slog::log_level_short_name(slog::LogLevel::Error) << std::endl;
    
    // Test log_level_from_name
    std::cout << "Parsing log level names:" << std::endl;
    auto level1 = slog::log_level_from_name("trace");
    auto level2 = slog::log_level_from_name("DEBUG");
    auto level3 = slog::log_level_from_name("i");
    auto level4 = slog::log_level_from_name("W");
    auto level5 = slog::log_level_from_name("error");
    auto level6 = slog::log_level_from_name("invalid", slog::LogLevel::Info);
    
    std::cout << "  \"trace\" -> " << slog::log_level_name(level1) << std::endl;
    std::cout << "  \"DEBUG\" -> " << slog::log_level_name(level2) << std::endl;
    std::cout << "  \"i\" -> " << slog::log_level_name(level3) << std::endl;
    std::cout << "  \"W\" -> " << slog::log_level_name(level4) << std::endl;
    std::cout << "  \"error\" -> " << slog::log_level_name(level5) << std::endl;
    std::cout << "  \"invalid\" (default Info) -> " << slog::log_level_name(level6) << std::endl;
}

// Test dynamic level change
void test_dynamic_level_change() {
    std::cout << "\n=== Test 7: Dynamic Level Change ===" << std::endl;
    
    auto logger = slog::make_stdout_logger("test_dynamic", slog::LogLevel::Error);
    
    std::cout << "Initial level: Error (only errors visible)" << std::endl;
    logger->info("Info message (should not appear)");
    logger->error("Error message (should appear)");
    
    logger->set_level(slog::LogLevel::Debug);
    std::cout << "Changed level to Debug (all messages visible)" << std::endl;
    logger->trace("Trace message (should appear)");
    logger->debug("Debug message (should appear)");
    logger->info("Info message (should appear)");
    logger->warning("Warning message (should appear)");
    logger->error("Error message (should appear)");
}

// Test is_allowed
void test_is_allowed() {
    std::cout << "\n=== Test 8: is_allowed Check ===" << std::endl;
    
    auto logger = slog::make_stdout_logger("test_allowed", slog::LogLevel::Info);
    
    std::cout << "Logger level: Info" << std::endl;
    std::cout << "  Trace allowed: " << (logger->is_allowed(slog::LogLevel::Trace) ? "yes" : "no") << std::endl;
    std::cout << "  Debug allowed: " << (logger->is_allowed(slog::LogLevel::Debug) ? "yes" : "no") << std::endl;
    std::cout << "  Info allowed: " << (logger->is_allowed(slog::LogLevel::Info) ? "yes" : "no") << std::endl;
    std::cout << "  Warning allowed: " << (logger->is_allowed(slog::LogLevel::Warning) ? "yes" : "no") << std::endl;
    std::cout << "  Error allowed: " << (logger->is_allowed(slog::LogLevel::Error) ? "yes" : "no") << std::endl;
}

// Test const char* vs string versions
void test_string_versions() {
    std::cout << "\n=== Test 9: String Versions ===" << std::endl;
    
    auto logger = slog::make_stdout_logger("test_string", slog::LogLevel::Debug);
    
    // Test const char* version
    const char* cstr = "C-style string message";
    logger->info(cstr);
    
    // Test std::string version
    std::string str = "C++ string message";
    logger->info(str);
    
    // Test formatted version
    logger->info("Formatted: {}", cstr);
}

// Test multiple loggers with different levels
void test_multiple_loggers() {
    std::cout << "\n=== Test 10: Multiple Loggers ===" << std::endl;
    
    auto logger_trace = slog::make_stdout_logger("trace_logger", slog::LogLevel::Trace);
    auto logger_info = slog::make_stdout_logger("info_logger", slog::LogLevel::Info);
    auto logger_error = slog::make_stdout_logger("error_logger", slog::LogLevel::Error);
    
    std::cout << "Testing trace_logger (Trace level):" << std::endl;
    logger_trace->trace("Trace message");
    logger_trace->info("Info message");
    logger_trace->error("Error message");
    
    std::cout << "\nTesting info_logger (Info level):" << std::endl;
    logger_info->trace("Trace message (should not appear)");
    logger_info->info("Info message");
    logger_info->error("Error message");
    
    std::cout << "\nTesting error_logger (Error level):" << std::endl;
    logger_error->trace("Trace message (should not appear)");
    logger_error->info("Info message (should not appear)");
    logger_error->error("Error message");
}

// Test logger name
void test_logger_name() {
    std::cout << "\n=== Test 11: Logger Name ===" << std::endl;
    
    auto logger = slog::make_stdout_logger("my_custom_logger", slog::LogLevel::Info);
    std::cout << "Logger name: " << logger->name() << std::endl;
    
    auto logger2 = slog::make_stdout_logger("another_logger", slog::LogLevel::Info);
    std::cout << "Another logger name: " << logger2->name() << std::endl;
}

// Test default logger
void test_default_logger() {
    std::cout << "\n=== Test 12: Default Logger ===" << std::endl;
    
    // Get default logger (should create one if none exists)
    auto default_log = slog::default_logger();
    default_log->info("Message from default logger");
    
    // Register a custom logger and set it as default
    auto custom_logger = slog::make_stdout_logger("custom_default", slog::LogLevel::Debug);
    slog::set_default_logger("custom_default");
    
    auto new_default = slog::default_logger();
    new_default->debug("Debug message from new default logger");
    new_default->info("Info message from new default logger");
}

// Test None sink (silent logger)
void test_none_sink() {
    std::cout << "\n=== Test 13: None Sink (Silent Logger) ===" << std::endl;
    
    // Create a logger with None sink - should not output anything
    auto silent_logger = slog::make_none_logger("silent_logger");
    
    std::cout << "Testing None sink - no output should appear below:" << std::endl;
    silent_logger->trace("This trace message should not appear");
    silent_logger->debug("This debug message should not appear");
    silent_logger->info("This info message should not appear");
    silent_logger->warning("This warning message should not appear");
    silent_logger->error("This error message should not appear");
    
    std::cout << "None sink test completed - if no log messages appeared above, test passed!" << std::endl;
    
    // Test that we can still check the logger properties
    std::cout << "Logger name: " << silent_logger->name() << std::endl;
    std::cout << "Logger level: " << slog::log_level_name(silent_logger->get_level()) << std::endl;
    
    // Test creating None logger using factory function
    auto logger_with_none = slog::make_none_logger("logger_with_none");
    std::cout << "Testing logger with None sink created via factory:" << std::endl;
    logger_with_none->info("This message should not appear");
    logger_with_none->error("This error message should not appear");
    std::cout << "None logger factory test completed!" << std::endl;
}

// Test global logging functions
void test_global_logging() {
    std::cout << "\n=== Test 14: Global Logging Functions ===" << std::endl;
    
    // Ensure default logger exists
    auto default_log = slog::default_logger();
    
    std::cout << "Testing global logging functions:" << std::endl;
    
    // Test basic global logging
    slog::trace("Global trace message");
    slog::debug("Global debug message");
    slog::info("Global info message");
    slog::warning("Global warning message");
    slog::error("Global error message");
    
    // Test global logging with formatting
    std::string name = "slog";
    int value = 42;
    double pi = 3.14159;
    
    slog::info("hello, this is {}", name);
    slog::info("Integer value: {}", value);
    slog::info("Double value: {}", pi);
    slog::debug("Combined: {} = {}, pi = {}", name, value, pi);
    
    // Test with std::string
    std::string msg = "This is a string message";
    slog::info(msg);
    
    // Test escaped braces
    slog::info("Literal braces: {{ and }}");
    slog::info("Mixed: value is {} and braces are {{}}", value);

    slog::info(FMT_STRING("test error: {}"), 123456);
    slog::info("test error, format string: {}", 123456);
    
    // Test SLOG macros with format checking
    std::cout << "\n=== Test SLOG Macros ===" << std::endl;
    SLOG_TRACE("Trace message with value: {}", 42);
    SLOG_DEBUG("Debug message with value: {}", 3.14);
    SLOG_INFO("Info message: {} and {}", "hello", 123);
    SLOG_WARNING("Warning message: {}", "test");
    SLOG_ERROR("Error message: {} and {}", 100, "error");
}

// Test log limiting functionality
void test_log_limiting() {
    std::cout << "\n=== Test 15: Log Limiting ===" << std::endl;
    
    auto logger = slog::make_stdout_logger("test_limited", slog::LogLevel::Debug);
    
    // Test 1: Basic limiting - allow 3 messages
    std::cout << "\nTest 1: Basic limiting (allow 3 messages):" << std::endl;
    std::cout << "Sending 5 messages with tag 'test1', only first 3 should appear:" << std::endl;
    for (int i = 1; i <= 5; ++i) {
        logger->info_limited("test1", 3, "Limited message {} of 5", i);
    }
    
    // Test 2: Last message should have suppression notice
    std::cout << "\nTest 2: Last allowed message should show suppression notice:" << std::endl;
    std::cout << "Sending 2 messages with tag 'test2', second should show suppression notice:" << std::endl;
    logger->info_limited("test2", 2, "First message");
    logger->info_limited("test2", 2, "Second message (should show suppression notice)");
    logger->info_limited("test2", 2, "Third message (should not appear)");
    
    // Test 3: Different tags are independent
    std::cout << "\nTest 3: Different tags have independent counters:" << std::endl;
    std::cout << "Sending messages with different tags:" << std::endl;
    logger->info_limited("tag_a", 2, "Tag A message 1");
    logger->info_limited("tag_b", 2, "Tag B message 1");
    logger->info_limited("tag_a", 2, "Tag A message 2 (should show suppression notice)");
    logger->info_limited("tag_b", 2, "Tag B message 2 (should show suppression notice)");
    logger->info_limited("tag_a", 2, "Tag A message 3 (should not appear)");
    logger->info_limited("tag_b", 2, "Tag B message 3 (should not appear)");
    
    // Test 4: Different log levels
    std::cout << "\nTest 4: Limiting works with different log levels:" << std::endl;
    logger->debug_limited("debug_tag", 2, "Debug limited message 1");
    logger->debug_limited("debug_tag", 2, "Debug limited message 2");
    logger->debug_limited("debug_tag", 2, "Debug limited message 3 (should not appear)");
    
    logger->warning_limited("warn_tag", 2, "Warning limited message 1");
    logger->warning_limited("warn_tag", 2, "Warning limited message 2");
    logger->warning_limited("warn_tag", 2, "Warning limited message 3 (should not appear)");
    
    logger->error_limited("error_tag", 2, "Error limited message 1");
    logger->error_limited("error_tag", 2, "Error limited message 2");
    logger->error_limited("error_tag", 2, "Error limited message 3 (should not appear)");
    
    // Test 5: Reset functionality
    std::cout << "\nTest 5: Reset limited counter:" << std::endl;
    std::cout << "Sending 2 messages, then reset, then 2 more:" << std::endl;
    logger->info_limited("reset_tag", 2, "Before reset message 1");
    logger->info_limited("reset_tag", 2, "Before reset message 2");
    logger->info_limited("reset_tag", 2, "Before reset message 3 (should not appear)");
    
    logger->reset_limited("reset_tag");
    std::cout << "Counter reset, sending more messages:" << std::endl;
    logger->info_limited("reset_tag", 2, "After reset message 1");
    logger->info_limited("reset_tag", 2, "After reset message 2");
    logger->info_limited("reset_tag", 2, "After reset message 3 (should not appear)");
    
    // Test 6: Global limited functions
    std::cout << "\nTest 6: Global limited logging functions:" << std::endl;
    std::cout << "Sending 2 messages using global functions:" << std::endl;
    slog::info_limited("global_tag", 2, "Global limited message 1");
    slog::info_limited("global_tag", 2, "Global limited message 2");
    slog::info_limited("global_tag", 2, "Global limited message 3 (should not appear)");
    
    // Test 7: Formatting with limited logging
    std::cout << "\nTest 7: Formatted limited logging:" << std::endl;
    std::cout << "Sending formatted messages:" << std::endl;
    int value = 42;
    std::string name = "test";
    logger->info_limited("format_tag", 2, "Formatted message: {} = {}", name, value);
    logger->info_limited("format_tag", 2, "Another formatted: value is {}", value);
    logger->info_limited("format_tag", 2, "This should not appear: {}", value);
    
    // Test 8: Single message limit
    std::cout << "\nTest 8: Single message limit:" << std::endl;
    std::cout << "Sending 3 messages with limit of 1:" << std::endl;
    logger->info_limited("single_tag", 1, "Only this message should appear (with suppression notice)");
    logger->info_limited("single_tag", 1, "This should not appear");
    logger->info_limited("single_tag", 1, "This should not appear");
    
    // Test 9: Zero limit (should suppress all)
    std::cout << "\nTest 9: Zero limit (should suppress all messages):" << std::endl;
    std::cout << "Sending messages with limit of 0 (none should appear):" << std::endl;
    logger->info_limited("zero_tag", 0, "This should not appear");
    logger->info_limited("zero_tag", 0, "This should not appear");
    
    // Test 10: Changing allowed_num dynamically
    std::cout << "\nTest 10: Changing allowed_num dynamically:" << std::endl;
    std::cout << "Sending messages with changing limit:" << std::endl;
    logger->info_limited("dynamic_tag", 2, "Message 1 (limit=2)");
    logger->info_limited("dynamic_tag", 2, "Message 2 (limit=2)");
    logger->info_limited("dynamic_tag", 2, "Message 3 (limit=2, should not appear)");
    logger->info_limited("dynamic_tag", 5, "Message 4 (limit changed to 5, should appear)");
    logger->info_limited("dynamic_tag", 5, "Message 5 (limit=5, should appear)");
    logger->info_limited("dynamic_tag", 5, "Message 6 (limit=5, should appear)");
}

// Test global logger level rules
// Test file sink with thread safety
void test_file_sink() {
    std::cout << "\n=== Test: File Sink ===" << std::endl;
    
    const std::string test_log_file = "/tmp/test_slog.log";
    
    // Clean up old test files
    struct stat st;
    if (stat(test_log_file.c_str(), &st) == 0) {
        std::remove(test_log_file.c_str());
    }
    for (int i = 1; i <= 5; ++i) {
        std::string old_file = test_log_file + "." + std::to_string(i);
        if (stat(old_file.c_str(), &st) == 0) {
            std::remove(old_file.c_str());
        }
    }
    
    // Test 1: Basic file logging
    std::cout << "\nTest 1: Basic file logging" << std::endl;
    {
        auto logger = slog::make_file_logger("file_test", test_log_file, slog::LogLevel::Debug);
        logger->debug("This is a debug message to file");
        logger->info("This is an info message to file");
        logger->warning("This is a warning message to file");
        logger->error("This is an error message to file");
    }
    
    // Verify file was created and contains logs
    struct stat st2;
    if (stat(test_log_file.c_str(), &st2) == 0) {
        std::ifstream file(test_log_file);
        std::string line;
        int line_count = 0;
        while (std::getline(file, line)) {
            line_count++;
        }
        std::cout << "File created successfully with " << line_count << " lines" << std::endl;
    } else {
        std::cout << "ERROR: File was not created!" << std::endl;
    }
    
    // Test 2: Multiple loggers writing to same file (thread safety)
    std::cout << "\nTest 2: Multiple loggers writing to same file (thread safety test)" << std::endl;
    {
        auto logger1 = slog::make_file_logger("logger1", test_log_file, slog::LogLevel::Info);
        auto logger2 = slog::make_file_logger("logger2", test_log_file, slog::LogLevel::Info);
        auto logger3 = slog::make_file_logger("logger3", test_log_file, slog::LogLevel::Info);
        
        std::vector<std::thread> threads;
        
        // Create multiple threads writing to the same file
        for (int i = 0; i < 3; ++i) {
            threads.emplace_back([&logger1, i]() {
                for (int j = 0; j < 10; ++j) {
                    logger1->info("Logger1 thread {} message {}", i, j);
                }
            });
            
            threads.emplace_back([&logger2, i]() {
                for (int j = 0; j < 10; ++j) {
                    logger2->info("Logger2 thread {} message {}", i, j);
                }
            });
            
            threads.emplace_back([&logger3, i]() {
                for (int j = 0; j < 10; ++j) {
                    logger3->info("Logger3 thread {} message {}", i, j);
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        std::cout << "Multiple loggers and threads test completed" << std::endl;
    }
    
    // Test 3: File rotation
    std::cout << "\nTest 3: File rotation" << std::endl;
    const std::string rotation_test_file = "/tmp/test_rotation.log";
    
    // Clean up old rotation test files
    struct stat st3;
    if (stat(rotation_test_file.c_str(), &st3) == 0) {
        std::remove(rotation_test_file.c_str());
    }
    for (int i = 1; i <= 5; ++i) {
        std::string old_file = rotation_test_file + "." + std::to_string(i);
        if (stat(old_file.c_str(), &st3) == 0) {
            std::remove(old_file.c_str());
        }
    }
    
    {

        // Clean up test files
        struct stat st4;
        if (stat(rotation_test_file.c_str(), &st4) == 0) {
            std::remove(rotation_test_file.c_str());
        }
        for (int i = 1; i <= 3; ++i) {
            std::string old_file = rotation_test_file + "." + std::to_string(i);
            if (stat(old_file.c_str(), &st4) == 0) {
                std::remove(old_file.c_str());
            }
        }

        // Create logger with small file size limit (1KB)
        auto logger = slog::make_rotating_file_logger("rotation_test", rotation_test_file, 
                                                    slog::LogLevel::Info, 1024, 3);
        
        // Write enough data to trigger rotation
        for (int i = 0; i < 100; ++i) {
            logger->info("This is a long log message for rotation test - message number {}", i);
        }
        
        std::cout << "Rotation test completed" << std::endl;
    }
    
    // Check if rotated files exist
    int rotated_files = 0;
    struct stat st5;
    for (int i = 1; i <= 3; ++i) {
        std::string old_file = rotation_test_file + "." + std::to_string(i);
        if (stat(old_file.c_str(), &st5) == 0) {
            rotated_files++;
        }
    }
    std::cout << "Found " << rotated_files << " rotated file(s)" << std::endl;
    
    
    std::cout << "File sink tests completed successfully!" << std::endl;
}

void test_global_logger_level_rules() {
    std::cout << "\n=== Test 16: Global Logger Level Rules ===" << std::endl;
    
    // Test 1: Set rule before creating logger (exact match)
    std::cout << "\nTest 1: Set rule before creating logger (exact match):" << std::endl;
    slog::set_logger_level("pre_created_logger", slog::LogLevel::Debug);
    auto logger1 = slog::make_stdout_logger("pre_created_logger", slog::LogLevel::Error);
    std::cout << "Logger created with Error level, but rule sets Debug:" << std::endl;
    std::cout << "  Actual level: " << slog::log_level_name(logger1->get_level()) << std::endl;
    logger1->trace("Trace message (should not appear, level is Debug)");
    logger1->debug("Debug message (should appear)");
    logger1->info("Info message (should appear)");
    
    // Test 2: Set rule after creating logger (exact match)
    std::cout << "\nTest 2: Set rule after creating logger (exact match):" << std::endl;
    auto logger2 = slog::make_stdout_logger("post_created_logger", slog::LogLevel::Error);
    std::cout << "Logger created with Error level:" << std::endl;
    logger2->info("Info message before rule (should not appear)");
    slog::set_logger_level("post_created_logger", slog::LogLevel::Info);
    std::cout << "Rule set to Info, level should change:" << std::endl;
    std::cout << "  Actual level: " << slog::log_level_name(logger2->get_level()) << std::endl;
    logger2->info("Info message after rule (should appear)");
    
    // Test 3: Regex pattern matching - match all loggers ending with "_debug"
    std::cout << "\nTest 3: Regex pattern matching (.*_debug):" << std::endl;
    slog::set_logger_level(".*_debug", slog::LogLevel::Trace);
    auto logger3a = slog::make_stdout_logger("test_debug", slog::LogLevel::Error);
    auto logger3b = slog::make_stdout_logger("another_debug", slog::LogLevel::Warning);
    auto logger3c = slog::make_stdout_logger("normal_logger", slog::LogLevel::Error);
    
    std::cout << "test_debug level: " << slog::log_level_name(logger3a->get_level()) << std::endl;
    std::cout << "another_debug level: " << slog::log_level_name(logger3b->get_level()) << std::endl;
    std::cout << "normal_logger level: " << slog::log_level_name(logger3c->get_level()) << std::endl;
    
    logger3a->trace("Trace message from test_debug (should appear)");
    logger3b->trace("Trace message from another_debug (should appear)");
    logger3c->trace("Trace message from normal_logger (should not appear)");
    
    // Test 4: Regex pattern matching - match all loggers starting with "camera_"
    std::cout << "\nTest 4: Regex pattern matching (^camera_.*):" << std::endl;
    slog::set_logger_level("^camera_.*", slog::LogLevel::Info);
    auto camera_main = slog::make_stdout_logger("camera_main", slog::LogLevel::Error);
    auto camera_sub = slog::make_stdout_logger("camera_sub", slog::LogLevel::Error);
    auto other_logger = slog::make_stdout_logger("other_logger", slog::LogLevel::Error);
    
    std::cout << "camera_main level: " << slog::log_level_name(camera_main->get_level()) << std::endl;
    std::cout << "camera_sub level: " << slog::log_level_name(camera_sub->get_level()) << std::endl;
    std::cout << "other_logger level: " << slog::log_level_name(other_logger->get_level()) << std::endl;
    
    camera_main->debug("Debug message (should not appear)");
    camera_main->info("Info message (should appear)");
    camera_sub->info("Info message from sub (should appear)");
    other_logger->info("Info message from other (should not appear)");
    
    // Test 5: Priority - exact match overrides regex match
    std::cout << "\nTest 5: Priority - exact match overrides regex match:" << std::endl;
    slog::set_logger_level(".*_special", slog::LogLevel::Warning);  // Regex rule
    slog::set_logger_level("test_special", slog::LogLevel::Debug);   // Exact match rule
    auto logger5 = slog::make_stdout_logger("test_special", slog::LogLevel::Error);
    
    std::cout << "test_special level: " << slog::log_level_name(logger5->get_level()) << std::endl;
    std::cout << "Should be Debug (exact match), not Warning (regex match):" << std::endl;
    logger5->debug("Debug message (should appear - exact match wins)");
    logger5->warning("Warning message (should appear)");
    
    // Test 6: Multiple regex rules - first match wins
    std::cout << "\nTest 6: Multiple regex rules - first match wins:" << std::endl;
    slog::set_logger_level(".*_network", slog::LogLevel::Error);      // First regex rule
    slog::set_logger_level(".*_network.*", slog::LogLevel::Debug);   // Second regex rule (broader)
    auto network_logger = slog::make_stdout_logger("test_network", slog::LogLevel::Info);
    
    std::cout << "test_network level: " << slog::log_level_name(network_logger->get_level()) << std::endl;
    std::cout << "Should be Error (first matching rule):" << std::endl;
    network_logger->debug("Debug message (should not appear)");
    network_logger->info("Info message (should not appear)");
    network_logger->error("Error message (should appear)");
    
    // Test 7: Regex pattern with special characters
    std::cout << "\nTest 7: Regex pattern with special characters:" << std::endl;
    slog::set_logger_level(".*module[0-9]+", slog::LogLevel::Trace);
    auto module1 = slog::make_stdout_logger("test_module1", slog::LogLevel::Error);
    auto module2 = slog::make_stdout_logger("test_module2", slog::LogLevel::Error);
    auto module10 = slog::make_stdout_logger("test_module10", slog::LogLevel::Error);
    auto module_abc = slog::make_stdout_logger("test_module_abc", slog::LogLevel::Error);
    
    std::cout << "test_module1 level: " << slog::log_level_name(module1->get_level()) << std::endl;
    std::cout << "test_module2 level: " << slog::log_level_name(module2->get_level()) << std::endl;
    std::cout << "test_module10 level: " << slog::log_level_name(module10->get_level()) << std::endl;
    std::cout << "test_module_abc level: " << slog::log_level_name(module_abc->get_level()) << std::endl;
    
    module1->trace("Trace from module1 (should appear)");
    module2->trace("Trace from module2 (should appear)");
    module10->trace("Trace from module10 (should appear)");
    module_abc->trace("Trace from module_abc (should not appear - no digit)");
    
    // Test 8: Update rule for existing logger
    std::cout << "\nTest 8: Update rule for existing logger:" << std::endl;
    auto logger8 = slog::make_stdout_logger("update_test", slog::LogLevel::Error);
    std::cout << "Initial level: " << slog::log_level_name(logger8->get_level()) << std::endl;
    logger8->info("Info before update (should not appear)");
    
    slog::set_logger_level("update_test", slog::LogLevel::Info);
    std::cout << "After setting rule to Info: " << slog::log_level_name(logger8->get_level()) << std::endl;
    logger8->info("Info after update (should appear)");
    
    slog::set_logger_level("update_test", slog::LogLevel::Debug);
    std::cout << "After updating rule to Debug: " << slog::log_level_name(logger8->get_level()) << std::endl;
    logger8->debug("Debug after update (should appear)");
    
    // Test 9: No matching rule - logger keeps original level
    std::cout << "\nTest 9: No matching rule - logger keeps original level:" << std::endl;
    auto logger9 = slog::make_stdout_logger("no_rule_logger", slog::LogLevel::Warning);
    std::cout << "Logger level: " << slog::log_level_name(logger9->get_level()) << std::endl;
    std::cout << "Should remain Warning (no matching rule):" << std::endl;
    logger9->info("Info message (should not appear)");
    logger9->warning("Warning message (should appear)");
    
    // Test 10: Empty pattern (should be ignored)
    std::cout << "\nTest 10: Empty pattern (should be ignored):" << std::endl;
    auto logger10 = slog::make_stdout_logger("empty_test", slog::LogLevel::Info);
    std::cout << "Before empty rule: " << slog::log_level_name(logger10->get_level()) << std::endl;
    slog::set_logger_level("", slog::LogLevel::Debug);  // Empty pattern
    std::cout << "After empty rule: " << slog::log_level_name(logger10->get_level()) << std::endl;
    std::cout << "Should remain Info (empty pattern ignored):" << std::endl;
    logger10->debug("Debug message (should not appear)");
    logger10->info("Info message (should appear)");
}



int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  slog Library Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;
    
    try {
        test_basic_logging();
        test_log_level_filtering();
        test_formatted_logging();
        test_hex_dump();
        test_logger_registry();
        test_log_level_utilities();
        test_dynamic_level_change();
        test_is_allowed();
        test_string_versions();
        test_multiple_loggers();
        test_logger_name();
        test_default_logger();
        test_none_sink();
        test_global_logging();
        test_log_limiting();
        test_file_sink();
        test_global_logger_level_rules();
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "  All Tests Completed Successfully!" << std::endl;
        std::cout << "========================================" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}

