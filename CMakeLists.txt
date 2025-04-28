cmake_minimum_required(VERSION 3.19)
project(SSGXLib "C" "CXX")

set(PROJECT_VERSION 1.1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(common/include)

option(ENABLE_TEST "Enable tests" OFF)
if (${ENABLE_TEST})
    enable_testing()
    add_subdirectory(test/BasicTest)
    add_subdirectory(test/HttpTest)

    add_test(NAME basic_test COMMAND basic_test_app ${CMAKE_BINARY_DIR}/test/BasicTest/enc/basic_test_enclave.signed.so)
    add_test(NAME http_test COMMAND http_test_app ${CMAKE_BINARY_DIR}/test/HttpTest/enc/http_test_enclave.signed.so)
else()
    add_subdirectory(ssgx)
    # For IDE(Begin SSGX)
    # Define an explicit build target for SSGX
    add_custom_target(build_ssgx
            COMMAND cmake --build --preset release-ssgx-build
            COMMENT "Building all SSGX libraries..."
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    # Define an explicit install target for SSGX
    add_custom_target(install_ssgx
            COMMAND cmake --install ${CMAKE_BINARY_DIR}
            COMMENT "Installing all SSGX libraries..."
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    set_target_properties(build_ssgx PROPERTIES EXCLUDE_FROM_ALL TRUE)
    set_target_properties(install_ssgx PROPERTIES EXCLUDE_FROM_ALL TRUE)
    # For IDE (End SSGX)
endif()