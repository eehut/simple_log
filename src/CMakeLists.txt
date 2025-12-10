
# Option: Enable stdout color output (default ON)
option(SLOG_STDOUT_COLOR "Enable color output for stdout sink" ON)

# Source files for the library
set(SLOG_SOURCES
    slog_logger.cpp
    fmt/format.cc
)

# Static library
add_library(slog_static STATIC ${SLOG_SOURCES})
set_target_properties(slog_static PROPERTIES
    OUTPUT_NAME slog
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)
target_include_directories(slog_static 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include/slog>
        $<INSTALL_INTERFACE:include>
)
target_compile_definitions(slog_static PUBLIC 
    SLOG_STDOUT_COLOR=$<IF:$<BOOL:${SLOG_STDOUT_COLOR}>,1,0>
)

# Shared library (default, use simple name 'slog')
add_library(slog SHARED ${SLOG_SOURCES})
set_target_properties(slog PROPERTIES
    OUTPUT_NAME slog
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)
target_include_directories(slog 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include/slog>
        $<INSTALL_INTERFACE:include>
)
target_compile_definitions(slog PUBLIC 
    SLOG_STDOUT_COLOR=$<IF:$<BOOL:${SLOG_STDOUT_COLOR}>,1,0>
)

# Alias targets for convenience
add_library(slog::slog_static ALIAS slog_static)
add_library(slog::slog ALIAS slog)

# Install libraries
install(TARGETS slog_static slog
    EXPORT slogTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# Install export targets
install(EXPORT slogTargets
    FILE slogTargets.cmake
    NAMESPACE slog::
    DESTINATION lib/cmake/slog
)
