
/**
 * @file test_get_logger.cpp    
 * @author LiuChuansen (179712066@qq.com)
 * @brief 测试get_logger函数
 * @version 0.1
 * @date 2026-01-02
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <iostream>
#include <slog/slog.hpp>


void test_case1()
{
    auto logger = slog::get_logger("test");
    logger->info("logger name should be test");
    slog::info("Logger name should be test");
}

void test_case2()
{
    auto logger = slog::default_logger()->clone("test2");
    logger->info("logger name should be test2");
    slog::info("Logger name should be default");
}

void test_case3()
{
    // 在创建logger前输出日志，创建一个default logger，并且为默认logger
    slog::info("This is log before logger created");
    // 使用get_logger函数，如果logger不存在，就从默认logger clone一个出来，重命名为新的名称。
    auto logger = slog::get_logger("test3");
    logger->info("logger name should be test3");
    slog::info("This logger name should be default");
}


int main(int argc, const char *argv[]) {

    int test_case = 1;
    if (argc > 1){
        test_case = std::atoi(argv[1]);
    }

    if (test_case == 1){
        test_case1();
    } else if (test_case == 2){
        test_case2();
    }else if (test_case == 3){
        test_case3();
    } else {
        std::cout << "Invalid test case" << std::endl;
        return 1;
    }

    return 0;
}



