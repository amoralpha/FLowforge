# FlowForge Tests
if(NOT FLOWFORGE_BUILD_TESTS)
    return()
endif()

if(NOT TARGET Catch2::Catch2WithMain)
    message(WARNING "Catch2 not available. Skipping test build.")
    return()
endif()

add_executable(flowforge_tests
    unit/dag_test.cpp
)

target_include_directories(flowforge_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/fixtures
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(flowforge_tests
    PRIVATE
        flowforge_lib
        Catch2::Catch2WithMain
)
   file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/test_output")
# Register tests with CTest
catch_discover_tests(flowforge_tests
    OUTPUT_DIR ${CMAKE_BINARY_DIR}/test_output
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
