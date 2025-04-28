set(LIB_NAME "ssgx_filesystem_u")
set(NAMESPACE "ssgx")

ssgx_add_untrusted_library(${LIB_NAME} SHARED
        SRCS fs_auxiliary.cpp ocall_filesystem.cpp
        EDL ssgx_filesystem_t.edl
        EDL_SEARCH_PATHS ${SSGX_EDL_SEARCH_PATHS}
)
add_library(${NAMESPACE}::${LIB_NAME} ALIAS ${LIB_NAME})

target_compile_features(${LIB_NAME} PRIVATE cxx_std_11)

target_include_directories(${LIB_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/common/include/
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(Installation)
install_targets_config(
        NAMESPACE ${NAMESPACE}
        LIB_NAME ${LIB_NAME}
)