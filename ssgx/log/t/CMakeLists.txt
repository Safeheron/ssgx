set(LIB_NAME "ssgx_log_t")
set(NAMESPACE "ssgx")

ssgx_add_trusted_library(${LIB_NAME}
        SRCS LogMessage.t.cpp SSGXLogger.cpp
        EDL ssgx_log_t.edl
        EDL_SEARCH_PATHS ${SSGX_EDL_SEARCH_PATHS}
)
add_library(${NAMESPACE}::${LIB_NAME} ALIAS ${LIB_NAME})

target_compile_features(${LIB_NAME} PRIVATE cxx_std_11)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(Installation)
install_targets_config(
        NAMESPACE ${NAMESPACE}
        LIB_NAME ${LIB_NAME})
