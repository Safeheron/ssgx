set(LIB_NAME "ssgx_http_u")
set(NAMESPACE "ssgx")

find_package(mbedtls_SGX REQUIRED)
find_package(Poco REQUIRED COMPONENTS Foundation Net Util)

file(GLOB http_u_server_SOURCE
        server/RequestHandler.cpp
        server/ocall_http_server.cpp
        server/http_u_server_imp.cpp
        server/SSGXHttpServer.cpp
)

ssgx_add_untrusted_library(${LIB_NAME} SHARED
    SRCS
        ${http_u_server_SOURCE}
    UNTRUSTED_LIBS
        mbedtls_SGX_u
        Poco::Net
        Poco::Util
        Poco::Foundation
    EDL
        ssgx_http_t.edl
    EDL_SEARCH_PATHS
        ${SSGX_EDL_SEARCH_PATHS}
)
add_library(${NAMESPACE}::${LIB_NAME} ALIAS ${LIB_NAME})
target_compile_features(${LIB_NAME} PRIVATE cxx_std_17)

#include_directories(
#        ${CMAKE_SOURCE_DIR}/common/include/
#)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(Installation)
install_targets_config(
        NAMESPACE ${NAMESPACE}
        LIB_NAME ${LIB_NAME}
)