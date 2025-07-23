cmake_minimum_required(VERSION 3.24)
project(http_client "C" "CXX")

find_package(ssgx REQUIRED)
ssgx_set_build_mode(Release)  # Options: Debug, PreRelease, Release
ssgx_set_hardware_mode(ON)    # Options: ON, OFF

ssgx_ensure_rsa_key_exists(
        KEY_FILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/enc/Enclave_private_test.pem"
        KEY_SIZE 3072
)
add_subdirectory(enc)
add_subdirectory(host)
