set(LIB_NAME "ssgx_json_t")
set(NAMESPACE "ssgx")

ssgx_add_trusted_library(${LIB_NAME}
        SRCS ssgx_json_t.cpp
)
add_library(${NAMESPACE}::${LIB_NAME} ALIAS ${LIB_NAME})

target_compile_features(${LIB_NAME} PRIVATE cxx_std_11)

target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_INSTALL_PREFIX}/include)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(Installation)
install_targets_config(
        NAMESPACE ${NAMESPACE}
        LIB_NAME ${LIB_NAME})
