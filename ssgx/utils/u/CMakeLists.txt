set(LIB_NAME "ssgx_utils_u")
set(NAMESPACE "ssgx")

ssgx_add_untrusted_library(${LIB_NAME} SHARED
        SRCS
            ocall_utils.cpp
        EDL
            ssgx_utils_t.edl
        EDL_SEARCH_PATHS
            ${SSGX_EDL_SEARCH_PATHS}
)
add_library(${NAMESPACE}::${LIB_NAME} ALIAS ${LIB_NAME})

target_compile_features(${LIB_NAME} PRIVATE cxx_std_11)



list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(Installation)
install_targets_config(
        NAMESPACE ${NAMESPACE}
        LIB_NAME ${LIB_NAME}
)
