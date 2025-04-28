cmake_minimum_required(VERSION 3.19)
project(basic_test "C" "CXX")

set(PROJECT_VERSION 0.0.1)

find_package(ssgx REQUIRED)

# For Test Data
add_definitions(-DTEST_DATA_DIR="${CMAKE_SOURCE_DIR}/test/data")

ssgx_ensure_rsa_key_exists(
        KEY_FILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/enc/Enclave_private_test.pem"
        KEY_SIZE 3072
)

add_subdirectory(enc)
add_subdirectory(host)
