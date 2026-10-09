cmake_minimum_required(VERSION 3.28 FATAL_ERROR)
project(FlowForge VERSION 0.1.0 LANGUAGES CXX)

# ------------------------------------------------------------------------------
# Build options
# ------------------------------------------------------------------------------
option(FLOWFORGE_BUILD_TESTS "Build unit and integration tests" ON)
option(FLOWFORGE_BUILD_BENCHMARKS "Build performance benchmarks" ON)
option(FLOWFORGE_BUILD_EXAMPLES "Build example programs" ON)
option(FLOWFORGE_BUILD_CLI "Build the flowforge CLI executable" OFF)

# ------------------------------------------------------------------------------
# Global C++ standard and compile options
# ------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/CompilerSettings.cmake)

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------
include(cmake/Dependencies.cmake)

# ------------------------------------------------------------------------------
# Subdirectories
# ------------------------------------------------------------------------------
add_subdirectory(src)

if(FLOWFORGE_BUILD_TESTS AND TARGET Catch2::Catch2WithMain)
    include(cmake/Testing.cmake)
    add_subdirectory(tests)
endif()

if(FLOWFORGE_BUILD_BENCHMARKS AND TARGET benchmark::benchmark)
    include(cmake/Benchmarking.cmake)
    add_subdirectory(benchmarks)
endif()

if(FLOWFORGE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ------------------------------------------------------------------------------
# Install and export configuration
# ------------------------------------------------------------------------------
include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FlowForgeConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/FlowForgeConfig.cmake"
    INSTALL_DESTINATION lib/cmake/FlowForge
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/FlowForgeConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(TARGETS flowforge_lib
    EXPORT FlowForgeTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)
install(DIRECTORY include/flowforge DESTINATION include)
install(EXPORT FlowForgeTargets
    FILE FlowForgeTargets.cmake
    NAMESPACE FlowForge::
    DESTINATION lib/cmake/FlowForge
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/FlowForgeConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/FlowForgeConfigVersion.cmake"
    DESTINATION lib/cmake/FlowForge
)
