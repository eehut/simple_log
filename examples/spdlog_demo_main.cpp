

#include "slog_spdlog_demo.hpp"

int main()
{
    auto logger = slog::make_spdlog_demo_logger("test", slog::LogLevel::Debug, "/tmp", true);

    logger->trace("This is a trace message");
    logger->debug("This is a debug message");
    logger->info("This is an info message");
    logger->warning("This is a warning message");
    logger->error("This is an error message");

    std::cout << "test clone" << std::endl;

    auto new_logger = logger->clone("clone");
    new_logger->trace("This is a trace message");
    new_logger->debug("This is a debug message");
    new_logger->info("This is an info message");
    new_logger->warning("This is a warning message");
    new_logger->error("This is an error message");

    return 0;
}
