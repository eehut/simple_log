# Test executable - link against static library
add_executable(test_slog_all
    test_all.cpp
)
target_link_libraries(test_slog_all PRIVATE slog_static)

add_executable(test_slog_file
    test_file_sink.cpp
)
target_link_libraries(test_slog_file PRIVATE slog_static)

# Performance test executable
add_executable(test_slog_performance
    test_performance.cpp
)
target_link_libraries(test_slog_performance PRIVATE slog_static)

# Add custom target to run tests
add_custom_target(run_test
    COMMAND test_slog_all
    DEPENDS test_slog_all
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMENT "Running test_slog_all..."
)

