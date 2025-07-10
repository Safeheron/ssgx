set(LIB_NAME "ssgx_attestation_u")
set(NAMESPACE "ssgx")

find_package(OpenSSL REQUIRED)
find_package(SafeheronCryptoSuites REQUIRED)

ssgx_add_untrusted_library(${LIB_NAME} SHARED
    SRCS RemoteAttestor.cpp ocall_attestation.cpp auxiliary.cpp
    EDL ssgx_attestation_t.edl
    EDL_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/common/include/
)
add_library(${NAMESPACE}::${LIB_NAME} ALIAS ${LIB_NAME})

target_compile_features(${LIB_NAME} PRIVATE cxx_std_11)

include_directories(
        ${SafeheronCryptoSuites_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../common
)

target_link_libraries(${LIB_NAME} PUBLIC
        -lsgx_dcap_ql -lsgx_quote_ex -lsgx_dcap_quoteverify
        -lcrypto
        SafeheronCryptoSuites
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(Installation)
install_targets_config(
        NAMESPACE ${NAMESPACE}
        LIB_NAME ${LIB_NAME}
)
