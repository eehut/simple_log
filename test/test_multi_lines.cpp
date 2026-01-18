/**
 * @file test_multi_lines.cpp
 * @author Liu Chuansen (179712066@qq.com)
 * @brief Test multi-line logging functionality
 * @version 0.1
 * @date 2025-12-04
 * 
 * @copyright Copyright (c) 2025 Liu Chuansen
 * 
 */

#include <iostream>
#include <slog/slog.hpp>

// Test multi-line logging with \n
void test_newline_separator() {
    std::cout << "\n=== Test 1: Multi-line with \\n separator ===" << std::endl;
    
    auto logger = slog::make_stdout_logger("test_newline", slog::LogLevel::Info);
    
    std::string msg = "Line 1\nLine 2\nLine 3";
    std::cout << "Input message: " << std::endl;
    std::cout << msg << std::endl;
    std::cout << "Output:" << std::endl;
    logger->log_lines(slog::LogLevel::Info, msg);
}

// Test multi-line logging with \r\n
void test_crlf_separator() {
    std::cout << "\n=== Test 2: Multi-line with \\r\\n separator ===" << std::endl;
    
    auto logger = slog::make_stdout_logger("test_crlf", slog::LogLevel::Info);
    
    std::string msg = "Line 1\r\nLine 2\r\nLine 3";
    std::cout << "Input message: " << std::endl;
    std::cout << msg << std::endl;
    std::cout << "Output:" << std::endl;
    logger->log_lines(slog::LogLevel::Info, msg);
}

// Test multi-line logging with \r
void test_cr_separator() {
    std::cout << "\n=== Test 3: Multi-line with \\r separator ===" << std::endl;
    
    auto logger = slog::make_stdout_logger("test_cr", slog::LogLevel::Info);
    
    std::string msg = "Line 1\rLine 2\rLine 3";
    std::cout << "Input message: " << std::endl;
    std::cout << msg << std::endl;
    std::cout << "Output:" << std::endl;
    logger->log_lines(slog::LogLevel::Info, msg);
}

// Test mixed line separators
void test_mixed_separators() {
    std::cout << "\n=== Test 4: Mixed line separators ===" << std::endl;
    
    auto logger = slog::make_stdout_logger("test_mixed", slog::LogLevel::Info);
    
    std::string msg = "Line 1\nLine 2\r\nLine 3\rLine 4\nLine 5";
    std::cout << "Input message: " << std::endl;
    std::cout << msg << std::endl;
    std::cout << "Output:" << std::endl;
    logger->log_lines(slog::LogLevel::Info, msg);
}

// Test empty lines
void test_empty_lines() {
    std::cout << "\n=== Test 5: Empty lines ===" << std::endl;
    
    auto logger = slog::make_stdout_logger("test_empty", slog::LogLevel::Info);
    
    std::string msg = "Line 1\n\nLine 3\n\n\nLine 6";
    std::cout << "Input message: " << std::endl;
    std::cout << msg << std::endl;
    std::cout << "Output:" << std::endl;
    logger->log_lines(slog::LogLevel::Info, msg);
}

// Test single line (no line separator)
void test_single_line() {
    std::cout << "\n=== Test 6: Single line (no separator) ===" << std::endl;
    
    auto logger = slog::make_stdout_logger("test_single", slog::LogLevel::Info);
    
    std::string msg = "This is a single line without any line separator";
    std::cout << "Input message: " << std::endl;
    std::cout << msg << std::endl;
    std::cout << "Output:" << std::endl;
    logger->log_lines(slog::LogLevel::Info, msg);
}

// Test line starting/ending with newlines
void test_leading_trailing_newlines() {
    std::cout << "\n=== Test 7: Leading and trailing newlines ===" << std::endl;
    
    auto logger = slog::make_stdout_logger("test_leading", slog::LogLevel::Info);
    
    std::string msg = "\nLine 2\nLine 3\n";
    std::cout << "Input message: " << std::endl;
    std::cout << msg << std::endl;
    std::cout << "Output:" << std::endl;
    logger->log_lines(slog::LogLevel::Info, msg);
}

// Test global log_lines function
void test_global_function() {
    std::cout << "\n=== Test 8: Global log_lines function ===" << std::endl;
    
    std::string msg = "Global Line 1\nGlobal Line 2\nGlobal Line 3";
    std::cout << "Input message: " << std::endl;
    std::cout << msg << std::endl;
    std::cout << "Output:" << std::endl;
    slog::log_lines(slog::LogLevel::Info, msg);
}

// Test different log levels
void test_different_levels() {
    std::cout << "\n=== Test 9: Different log levels ===" << std::endl;
    
    auto logger = slog::make_stdout_logger("test_levels", slog::LogLevel::Trace);
    
    std::string msg = "Trace line 1\nTrace line 2";
    std::cout << "Trace level:" << std::endl;
    logger->log_lines(slog::LogLevel::Trace, msg);
    
    msg = "Debug line 1\nDebug line 2";
    std::cout << "\nDebug level:" << std::endl;
    logger->log_lines(slog::LogLevel::Debug, msg);
    
    msg = "Info line 1\nInfo line 2";
    std::cout << "\nInfo level:" << std::endl;
    logger->log_lines(slog::LogLevel::Info, msg);
    
    msg = "Warning line 1\nWarning line 2";
    std::cout << "\nWarning level:" << std::endl;
    logger->log_lines(slog::LogLevel::Warning, msg);
    
    msg = "Error line 1\nError line 2";
    std::cout << "\nError level:" << std::endl;
    logger->log_lines(slog::LogLevel::Error, msg);
}

// Test long multi-line text
void test_long_multiline() {
    std::cout << "\n=== Test 10: Long multi-line text ===" << std::endl;
    
    auto logger = slog::make_stdout_logger("test_long", slog::LogLevel::Info);
    
    std::string msg = 
        "This is a very long multi-line text that spans multiple lines.\n"
        "Each line should be logged separately.\n"
        "This demonstrates the streaming output capability.\n"
        "The output should appear line by line as it's being processed.\n"
        "This is the last line of the long text.";
    
    std::cout << "Input message (5 lines):" << std::endl;
    std::cout << msg << std::endl;
    std::cout << "\nOutput (should appear line by line):" << std::endl;
    logger->log_lines(slog::LogLevel::Info, msg);
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Multi-line Logging Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;
    
    test_newline_separator();
    test_crlf_separator();
    test_cr_separator();
    test_mixed_separators();
    test_empty_lines();
    test_single_line();
    test_leading_trailing_newlines();
    test_global_function();
    test_different_levels();
    test_long_multiline();
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "All tests completed!" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}
