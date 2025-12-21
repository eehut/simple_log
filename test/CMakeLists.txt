
# Test all
add_executable(test_slog_all test_all.cpp)
target_link_libraries(test_slog_all PRIVATE slog::slog_static)

# test slog with file sink
add_executable(test_slog_file test_file_sink.cpp)
target_link_libraries(test_slog_file PRIVATE slog_static)

# test slog with performance test
add_executable(test_slog_performance test_performance.cpp)
target_link_libraries(test_slog_performance PRIVATE slog_static)

# Spdlog test (only if spdlog is enabled)
if(BUILD_WITH_SPDLOG)
    add_executable(test_spdlog test_spdlog.cpp)
    target_link_libraries(test_spdlog PRIVATE slog_static)
endif()

# Add custom target to run tests
add_custom_target(run_test
    COMMAND test_slog_all
    DEPENDS test_slog_all
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMENT "Running test_slog_all..."
)
