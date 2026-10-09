# FlowForge Library and Tools
# This CMakeLists.txt builds:
#   - flowforge_lib (static library)
#   - flowforge (CLI executable, if FLOWFORGE_BUILD_CLI=ON)

# ------------------------------------------------------------------------------
# Library: flowforge_lib
# ------------------------------------------------------------------------------
add_library(flowforge_lib STATIC
    # DAG (v0.1)
    flowforge/dag/DAG.cpp
)

target_include_directories(flowforge_lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(flowforge_lib
    PUBLIC
        Threads::Threads
)

target_compile_features(flowforge_lib PUBLIC cxx_std_23)

# ------------------------------------------------------------------------------
# CLI Executable
# ------------------------------------------------------------------------------
if(FLOWFORGE_BUILD_CLI)
    add_executable(flowforge
        tools/flowforge.cpp
    )

    target_link_libraries(flowforge PRIVATE flowforge_lib)
    set_target_properties(flowforge PROPERTIES OUTPUT_NAME flowforge)
endif()
