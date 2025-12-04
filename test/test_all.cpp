
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

#include <slog/slog.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

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
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "  All Tests Completed Successfully!" << std::endl;
        std::cout << "========================================" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}

