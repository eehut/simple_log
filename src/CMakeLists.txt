
# Option: Enable stdout color output (default ON)
option(SLOG_STDOUT_COLOR "Enable color output for stdout sink" ON)

# Source files for the library
set(SLOG_SOURCES
    slog_logger.cpp
    sink_stdout.cpp
    sink_file.cpp
)

# Add bundled fmt if not using system fmt
if(NOT BUILD_WITH_LIBFMT)
    list(APPEND SLOG_SOURCES fmt/format.cc)
endif()

# Add spdlog sources if enabled
if(BUILD_WITH_SPDLOG)
    list(APPEND SLOG_SOURCES
        sink_spdlog.cpp
    )
endif()

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
        $<INSTALL_INTERFACE:include>
)
target_compile_definitions(slog_static PUBLIC 
    SLOG_STDOUT_COLOR=$<IF:$<BOOL:${SLOG_STDOUT_COLOR}>,1,0>
    $<IF:$<BOOL:${BUILD_WITH_SPDLOG}>,BUILD_WITH_SPDLOG=1,>
    $<IF:$<BOOL:${BUILD_WITH_LIBFMT}>,BUILD_WITH_LIBFMT=1,>
)

# Link fmt library and ensure its include directories are used
if(BUILD_WITH_LIBFMT)
    target_link_libraries(slog_static PUBLIC fmt::fmt)
    # fmt::fmt should provide include directories, but ensure they come after our includes
    # This ensures <fmt/format.h> finds system fmt, not slog/fmt/format.h
endif()

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
        $<INSTALL_INTERFACE:include>
)
target_compile_definitions(slog PUBLIC 
    SLOG_STDOUT_COLOR=$<IF:$<BOOL:${SLOG_STDOUT_COLOR}>,1,0>
    $<IF:$<BOOL:${BUILD_WITH_SPDLOG}>,BUILD_WITH_SPDLOG=1,>
    $<IF:$<BOOL:${BUILD_WITH_LIBFMT}>,BUILD_WITH_LIBFMT=1,>
)

# Link fmt library
if(BUILD_WITH_LIBFMT)
    target_link_libraries(slog PUBLIC fmt::fmt)
endif()

# Link spdlog if enabled
if(BUILD_WITH_SPDLOG)
    # Use PUBLIC to propagate spdlog dependency to users of slog
    target_link_libraries(slog_static PUBLIC spdlog::spdlog)
    target_link_libraries(slog PUBLIC spdlog::spdlog)
endif()

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
