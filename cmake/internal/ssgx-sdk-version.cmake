# ==============================================================================
# ssgx-sdk-version.cmake
#
# Detects SGX SDK version via sgx_sign and defines SGXSDK_VERSION_MAJOR and
# SGXSDK_VERSION_MINOR as global compile definitions for version-gated code.
#
# Requires: SSGX_ENV__SGX_ENCLAVE_SIGNER must be set before including this file.
# ==============================================================================

execute_process(
    COMMAND ${SSGX_ENV__SGX_ENCLAVE_SIGNER} -version
    OUTPUT_VARIABLE _sgx_sign_ver_output
    ERROR_VARIABLE  _sgx_sign_ver_output
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REGEX MATCH "version ([0-9]+)\\.([0-9]+)" _ "${_sgx_sign_ver_output}")
set(_sgx_major ${CMAKE_MATCH_1})
set(_sgx_minor ${CMAKE_MATCH_2})
if(NOT _sgx_major OR NOT _sgx_minor)
    message(FATAL_ERROR "[SSGX] Failed to detect SGX SDK version from sgx_sign output:\n${_sgx_sign_ver_output}")
endif()
message(STATUS "[SSGX] SGX SDK version: ${_sgx_major}.${_sgx_minor}")
add_compile_definitions(SGXSDK_VERSION_MAJOR=${_sgx_major} SGXSDK_VERSION_MINOR=${_sgx_minor})
unset(_sgx_sign_ver_output)
unset(_sgx_major)
unset(_sgx_minor)
