# FlowForge Examples
if(NOT FLOWFORGE_BUILD_EXAMPLES)
    return()
endif()

add_executable(dag_example
    dag_example.cpp
)
target_link_libraries(dag_example PRIVATE flowforge_lib)
