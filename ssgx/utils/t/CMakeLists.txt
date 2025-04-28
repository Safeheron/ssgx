set(LIB_NAME "ssgx_utils_t")
set(NAMESPACE "ssgx")

ssgx_add_trusted_library(${LIB_NAME}
        SRCS
            ssgx_utils_t.cpp
            ecall_utils.cpp
            dummy_utils.c
            uuid/uuid4/uuid4.c
            uuid/UUIDGenerator.cpp
            time/musl/__month_to_secs.c
            time/musl/__secs_to_tm.c
            time/musl/__tm_to_secs.c
            time/musl/__year_to_secs.c
            time/DateTime.cpp
            time/TimeSpan.cpp
            time/PreciseTime.cpp
            time/TimeVerifier.cpp
            ecall_utils.cpp
            EnclaveInfo.cpp
            seal/SealHandler.cpp
        EDL
            ssgx_utils_t.edl
        EDL_SEARCH_PATHS
            ${SSGX_EDL_SEARCH_PATHS}
)
add_library(${NAMESPACE}::${LIB_NAME} ALIAS ${LIB_NAME})
target_compile_features(${LIB_NAME} PRIVATE cxx_std_17)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(Installation)
install_targets_config(
        NAMESPACE ${NAMESPACE}
        LIB_NAME ${LIB_NAME})
