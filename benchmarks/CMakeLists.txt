# FlowForge Benchmarks
if(NOT FLOWFORGE_BUILD_BENCHMARKS)
    return()
endif()

add_executable(flowforge_benchmarks
    bench_dag.cpp
)

target_include_directories(flowforge_benchmarks
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(flowforge_benchmarks
    PRIVATE
        flowforge_lib
        benchmark::benchmark
)

set_target_properties(
    flowforge_benchmarks
    PROPERTIES
        OUTPUT_NAME flowforge_benchmarks
)