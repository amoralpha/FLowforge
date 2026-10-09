# FlowForge Benchmarking Setup
# Requires Google Benchmark (fetched or found via find_package)

if(NOT FLOWFORGE_BUILD_BENCHMARKS)
    return()
endif()

if(NOT TARGET benchmark::benchmark)
    message(WARNING "Google Benchmark not available. Skipping benchmark build.")
    return()
endif()