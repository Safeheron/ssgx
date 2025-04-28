include(CMakeFindDependencyMacro)

include(${CMAKE_CURRENT_LIST_DIR}/ssgx-build.cmake)

include(${CMAKE_CURRENT_LIST_DIR}/../ssgx_log_t/ssgx_log_tTargets.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../ssgx_log_u/ssgx_log_uTargets.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../ssgx_json_t/ssgx_json_tTargets.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../ssgx_utils_t/ssgx_utils_tTargets.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../ssgx_utils_u/ssgx_utils_uTargets.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../ssgx_attestation_t/ssgx_attestation_tTargets.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../ssgx_attestation_u/ssgx_attestation_uTargets.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../ssgx_config_t/ssgx_config_tTargets.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../ssgx_config_u/ssgx_config_uTargets.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../ssgx_decimal_t/ssgx_decimal_tTargets.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../ssgx_filesystem_t/ssgx_filesystem_tTargets.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../ssgx_filesystem_u/ssgx_filesystem_uTargets.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../ssgx_http_t/ssgx_http_tTargets.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../ssgx_http_u/ssgx_http_uTargets.cmake)

find_dependency(log4cplus)
find_dependency(ssgx_log_t)
find_dependency(ssgx_log_u)
find_dependency(ssgx_json_t)
find_dependency(ssgx_utils_t)
find_dependency(ssgx_utils_u)
find_dependency(ssgx_attestation_t)
find_dependency(ssgx_attestation_u)
find_dependency(ssgx_config_t)
find_dependency(ssgx_config_u)
find_dependency(ssgx_decimal_t)
find_dependency(ssgx_filesystem_t)
find_dependency(ssgx_filesystem_u)
find_dependency(ssgx_http_t)
find_dependency(ssgx_http_u)

#if (ssgx_log_t_FOUND AND ssgx_log_u_FOUND AND ssgx_json_t_FOUND)
if (ssgx_log_t_FOUND AND ssgx_log_u_FOUND)
    set(ssgx_FOUND TRUE)
    set(ssgx_INCLUDE_DIR
            ${ssgx_log_t_INCLUDE_DIR}
    )
    set(ssgx_LIBRARY_DIRS
            ${ssgx_log_t_LIBRARY_DIR}
    )
    set(ssgx_EDL_DIRS
            ${ssgx_log_t_INCLUDE_DIR}
            ${ssgx_log_t_INCLUDE_DIR}/mbedtls
    )
    set(ssgx_LIBRARIES
            ssgx::ssgx_utils_t
            ssgx::ssgx_utils_u
            ssgx::ssgx_log_t
            ssgx::ssgx_log_u
            ssgx::ssgx_json_t
            ssgx::ssgx_database_t
            ssgx::ssgx_database_u
            ssgx::ssgx_decimal_t
            ssgx::ssgx_http_t
            ssgx::ssgx_http_u
    )
else ()
    set(ssgx_FOUND FALSE)
endif ()

MESSAGE(STATUS "Found ssgx.")