set(LIB_NAME "ssgx_config_t")
set(NAMESPACE "ssgx")

ssgx_add_trusted_library(${LIB_NAME}
        SRCS ssgx_config_t.cpp
        EDL ssgx_config_t.edl
        EDL_SEARCH_PATHS ${SSGX_EDL_SEARCH_PATHS}
)
add_library(${NAMESPACE}::${LIB_NAME} ALIAS ${LIB_NAME})

target_compile_features(${LIB_NAME} PRIVATE cxx_std_17)

target_include_directories(${LIB_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/common/include/
        ${CMAKE_SOURCE_DIR}/ssgx/common
        ${CMAKE_INSTALL_PREFIX}/include
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(Installation)
install_targets_config(
        NAMESPACE ${NAMESPACE}
        LIB_NAME ${LIB_NAME})
