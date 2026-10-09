# FlowForge Testing Setup
# Requires Catch2 (fetched or found via find_package)

if(NOT FLOWFORGE_BUILD_TESTS)
    return()
endif()

if(TARGET Catch2::Catch2WithMain)

    include(CTest)
    enable_testing()

    # Catch2 provides Catch.cmake.
    # Dependencies.cmake adds Catch2's extras/ directory
    # to CMAKE_MODULE_PATH when Catch2 is fetched.
    include(Catch)

    set(CATCH2_TEST_PREFIX "flowforge:")

else()

    message(WARNING
        "Catch2 not available. Tests will not be built."
    )

    return()

endif()