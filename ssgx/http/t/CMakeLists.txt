set(LIB_NAME "ssgx_http_t")
set(NAMESPACE "ssgx")

find_package(mbedtls_SGX REQUIRED)

file(GLOB http_t_common_SOURCE
        common/Request.cpp
        common/Response.cpp
)

file(GLOB http_t_client_SOURCE
        client/Client.cpp
        client/mbedtls_client/HttpClient.cpp
        client/mbedtls_client/HttpUrl.cpp
)

file(GLOB http_t_server_SOURCE
        server/Server.cpp
        server/ecall_http_server.cpp
)

ssgx_add_trusted_library(${LIB_NAME}
        SRCS
            ${http_t_common_SOURCE}
            ${http_t_client_SOURCE}
            ${http_t_server_SOURCE}
        TRUSTED_LIBS
            mbedtls_SGX_t
            ssgx::ssgx_utils_t
        EDL
            ssgx_http_t.edl
        EDL_SEARCH_PATHS
            ${SSGX_EDL_SEARCH_PATHS}
)
add_library(${NAMESPACE}::${LIB_NAME} ALIAS ${LIB_NAME})
target_compile_features(${LIB_NAME} PRIVATE cxx_std_17)

target_compile_features(${LIB_NAME} PRIVATE cxx_std_17)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(Installation)
install_targets_config(
        NAMESPACE ${NAMESPACE}
        LIB_NAME ${LIB_NAME})
