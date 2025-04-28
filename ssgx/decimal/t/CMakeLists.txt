set(LIB_NAME "ssgx_decimal_t")
set(NAMESPACE "ssgx")

find_package(mpdecimal_SGX REQUIRED)

ssgx_add_trusted_library(${LIB_NAME}
        SRCS
            ssgx_decimal_t.cpp
        TRUSTED_LIBS
            mpdecimal_SGX
)
add_library(${NAMESPACE}::${LIB_NAME} ALIAS ${LIB_NAME})

include_directories(
        ${CMAKE_SOURCE_DIR}/common/include/
        ${CMAKE_SOURCE_DIR}/ssgx/common/
)


target_compile_features(${LIB_NAME} PRIVATE cxx_std_11)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(Installation)
install_targets_config(
        NAMESPACE ${NAMESPACE}
        LIB_NAME ${LIB_NAME})
