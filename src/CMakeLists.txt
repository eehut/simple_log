
include_directories(${CMAKE_SOURCE_DIR}/include/slog)

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
target_compile_definitions(slog_static PUBLIC 
    SLOG_STDOUT_COLOR=$<IF:$<BOOL:${SLOG_STDOUT_COLOR}>,1,0>
)

# Shared library
add_library(slog_shared SHARED ${SLOG_SOURCES})
set_target_properties(slog_shared PROPERTIES
    OUTPUT_NAME slog
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)
target_compile_definitions(slog_shared PUBLIC 
    SLOG_STDOUT_COLOR=$<IF:$<BOOL:${SLOG_STDOUT_COLOR}>,1,0>
)

# Alias targets for convenience
add_library(slog::slog_static ALIAS slog_static)
add_library(slog::slog_shared ALIAS slog_shared)

