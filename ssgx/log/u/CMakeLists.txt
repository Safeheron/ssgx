set(LIB_NAME "ssgx_log_u")
set(NAMESPACE "ssgx")

find_package(log4cplus CONFIG REQUIRED)

set(LIB_SRC_FILES LogHelper.cpp ocall_log.cpp SSGXLogger.cpp LogMessage.u.cpp)
ssgx_add_untrusted_library(${LIB_NAME} SHARED
        SRCS ${LIB_SRC_FILES}
        EDL ssgx_log_t.edl
        EDL_SEARCH_PATHS ${SSGX_EDL_SEARCH_PATHS}
)
add_library(${NAMESPACE}::${LIB_NAME} ALIAS ${LIB_NAME})

target_compile_features(${LIB_NAME} PRIVATE cxx_std_11)

target_link_libraries(${LIB_NAME} PUBLIC
        log4cplus::log4cplus
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(Installation)
install_targets_config(
        NAMESPACE ${NAMESPACE}
        LIB_NAME ${LIB_NAME}
)
