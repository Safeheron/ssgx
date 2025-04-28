set(LIB_NAME "ssgx_attestation_t")
set(NAMESPACE "ssgx")

find_package(SafeheronCryptoSuitesSgx REQUIRED)

ssgx_add_trusted_library(${LIB_NAME}
        USE_SGXSSL ON
        SRCS RemoteAttestor.cpp
        EDL ssgx_attestation_t.edl
        EDL_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/common/include/
        TRUSTED_LIBS ssgx::ssgx_utils_t SafeheronCryptoSuitesSgx
)
add_library(${NAMESPACE}::${LIB_NAME} ALIAS ${LIB_NAME})

target_compile_features(${LIB_NAME} PRIVATE cxx_std_17)

include_directories(
        ${SafeheronCryptoSuitesSgx_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../common
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(Installation)
install_targets_config(
        NAMESPACE ${NAMESPACE}
        LIB_NAME ${LIB_NAME})
