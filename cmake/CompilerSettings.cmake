# FlowForge Compiler Settings
# C++23 standard, warnings, sanitizers, and LTO

# Ensure C++23 is enforced
if(NOT CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 23)
    message(FATAL_ERROR "FlowForge requires C++23 support. Please use GCC 13+, Clang 17+, or MSVC 19.35+.")
endif()

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Warnings
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wshadow
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        -Wimplicit-fallthrough
        -Wmissing-include-dirs
        -Wswitch-enum
        -Wswitch-default
        -Wunused-parameter
        -Wunused-variable
        -Wunused-value
        -Wunused-function
        -Wredundant-decls
        -Werror=return-type
    )

    # Debug sanitizers
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address,undefined)
    endif()

    # LTO for release builds
    if(CMAKE_BUILD_TYPE MATCHES "Release|RelWithDebInfo")
        add_compile_options(-flto=auto)
        add_link_options(-flto=auto)
    endif()

    # Position-independent code (required for shared libs)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(
        /W4
        /permissive-
        /utf-8
        /Zc:__cplusplus
        /Zc:preprocessor
        /Zc:referenceBinding
        /Zc:throwingNew
    )

    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        add_compile_options(/fsanitize=address /INCREMENTAL:NO)
    endif()

    if(CMAKE_BUILD_TYPE MATCHES "Release|RelWithDebInfo")
        add_compile_options(/GL)
        add_link_options(/LTCG)
    endif()
endif()

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Multi-config generators (Visual Studio, Xcode)
foreach(OUTPUT_CONFIG_TYPE ARCHIVE LIBRARY RUNTIME)
    set(CMAKE_${OUTPUT_CONFIG_TYPE}_OUTPUT_DIRECTORY_${CMAKE_MATCH_1} ${CMAKE_BINARY_DIR}/bin)
endforeach()
