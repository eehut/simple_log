if(BUILD_WITH_SPDLOG)
    add_executable(spdlog_demo_main spdlog_demo_main.cpp)
    target_link_libraries(spdlog_demo_main PRIVATE slog_static spdlog::spdlog_header_only)
endif()
