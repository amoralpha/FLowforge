# FlowForge External Dependencies
# Use find_package with CONFIG mode preferred. Fall back to FetchContent for
# header-only or small libraries.

option(FLOWFORGE_SKIP_FETCHCONTENT "Skip FetchContent if find_package fails (air-gapped builds)" OFF)

# ------------------------------------------------------------------------------
# Threads (required)
# ------------------------------------------------------------------------------
find_package(Threads REQUIRED)

# ------------------------------------------------------------------------------
# nlohmann/json (header-only, preferred for JSON parsing)
# ------------------------------------------------------------------------------
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    if(FLOWFORGE_SKIP_FETCHCONTENT)
        message(WARNING "nlohmann/json not found and FetchContent is skipped")
    else()
        message(STATUS "nlohmann/json not found, will use FetchContent")
        include(FetchContent)
        FetchContent_Declare(
            nlohmann_json
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG v3.11.3
            GIT_SHALLOW ON
        )
        FetchContent_MakeAvailable(nlohmann_json)
    endif()
else()
    message(STATUS "Found nlohmann/json via find_package")
endif()

# ------------------------------------------------------------------------------
# CLI11 (header-only, preferred for CLI parsing)
# ------------------------------------------------------------------------------
find_package(CLI11 QUIET)
if(NOT CLI11_FOUND)
    if(FLOWFORGE_SKIP_FETCHCONTENT)
        message(WARNING "CLI11 not found and FetchContent is skipped")
    else()
        message(STATUS "CLI11 not found, will use FetchContent")
        include(FetchContent)
        FetchContent_Declare(
            CLI11
            GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
            GIT_TAG v2.4.2
            GIT_SHALLOW ON
        )
        FetchContent_MakeAvailable(CLI11)
    endif()
else()
    message(STATUS "Found CLI11 via find_package")
endif()

# ------------------------------------------------------------------------------
# BLAKE3 (C library, required for content hashing)
# ------------------------------------------------------------------------------
find_package(blake3 QUIET)
if(NOT blake3_FOUND)
    if(FLOWFORGE_SKIP_FETCHCONTENT)
        message(WARNING "BLAKE3 not found and FetchContent is skipped")
    else()
        message(STATUS "BLAKE3 not found, will use FetchContent")
        include(FetchContent)
        FetchContent_Declare(
            blake3
            GIT_REPOSITORY https://github.com/BLAKE3-team/BLAKE3.git
            GIT_TAG 1.5.5
            GIT_SHALLOW ON
        )
        FetchContent_MakeAvailable(blake3)
    endif()
else()
    message(STATUS "Found BLAKE3 via find_package")
endif()

# ------------------------------------------------------------------------------
# libpqxx (PostgreSQL C++ client, required for Storage in v0.3+)
# ------------------------------------------------------------------------------
# Not required for v0.1. Uncomment when Storage module is implemented.
# find_package(libpqxx QUIET)
# if(NOT libpqxx_FOUND)
#     message(FATAL_ERROR "libpqxx is required for the Storage module")
# endif()

# ------------------------------------------------------------------------------
# fmt (header-only, preferred for string formatting)
# C++23 has std::format, but fmt is used for compatibility and extended features.
# ------------------------------------------------------------------------------
find_package(fmt QUIET)
if(NOT fmt_FOUND)
    if(FLOWFORGE_SKIP_FETCHCONTENT)
        message(WARNING "fmt not found and FetchContent is skipped")
    else()
        message(STATUS "fmt not found, will use FetchContent")
        include(FetchContent)
        FetchContent_Declare(
            fmt
            GIT_REPOSITORY https://github.com/fmtlib/fmt.git
            GIT_TAG 10.2.1
            GIT_SHALLOW ON
        )
        FetchContent_MakeAvailable(fmt)
    endif()
else()
    message(STATUS "Found fmt via find_package")
endif()

# ------------------------------------------------------------------------------
# Google Benchmark (required for benchmarks, if enabled)
# ------------------------------------------------------------------------------
if(FLOWFORGE_BUILD_BENCHMARKS)
    find_package(benchmark QUIET)
    if(NOT benchmark_FOUND)
        if(FLOWFORGE_SKIP_FETCHCONTENT)
            message(WARNING "Google Benchmark not found and FetchContent is skipped")
        else()
            message(STATUS "Google Benchmark not found, will use FetchContent")
            include(FetchContent)
            FetchContent_Declare(
                benchmark
                GIT_REPOSITORY https://github.com/google/benchmark.git
                GIT_TAG v1.9.1
                GIT_SHALLOW ON
            )
            FetchContent_MakeAvailable(benchmark)
        endif()
    else()
        message(STATUS "Found Google Benchmark via find_package")
    endif()
endif()

# ------------------------------------------------------------------------------
# Catch2 (required for tests, if enabled)
# ------------------------------------------------------------------------------
if(FLOWFORGE_BUILD_TESTS)
    find_package(Catch2 QUIET)
    if(NOT Catch2_FOUND)
        if(FLOWFORGE_SKIP_FETCHCONTENT)
            message(WARNING "Catch2 not found and FetchContent is skipped")
        else()
            message(STATUS "Catch2 not found, will use FetchContent")
            include(FetchContent)
            FetchContent_Declare(
                Catch2
                GIT_REPOSITORY https://github.com/catchorg/Catch2.git
                GIT_TAG v3.6.0
                GIT_SHALLOW ON
            )
            FetchContent_MakeAvailable(Catch2)
        endif()
    else()
        message(STATUS "Found Catch2 via find_package")
    endif()
    if(DEFINED catch2_SOURCE_DIR)
        list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    endif()
endif()
