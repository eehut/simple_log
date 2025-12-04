# Test executable - link against static library
add_executable(test_all
    test_all.cpp
)
target_link_libraries(test_all PRIVATE slog_static)

# Add custom target to run tests
add_custom_target(run_test
    COMMAND test_all
    DEPENDS test_all
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMENT "Running test_all..."
)

