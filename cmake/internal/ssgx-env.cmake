# ============================================================================================
# SSGX Global Configuration Overview
#
# This file defines all global variables used by the Safeheron SGX CMake build framework.
#
# [1] Intel SGX SDK Related Variables
# -----------------------------------
# - SSGX_ENV__SGXSDK                 : Root path of the SGX SDK
# - SSGX_ENV__SGX_COMMON_CFLAGS      : Base compile flags for SGX targets (-m64 or -m32)
# - SSGX_ENV__SGX_ENCLAVE_SIGNER     : Path to the `sgx_sign` tool
# - SSGX_ENV__SGX_EDGER8R            : Path to the `sgx_edger8r` tool
# - SSGX_ENV__SGXSDK_LIBRARY_DIR     : Path to SDK library directory (lib64 or lib32)
# - SSGX_ENV__SGXSDK_INCLUDE_DIR     : Path to SDK main include directory
# - SSGX_ENV__SGXSDK_INCLUDE_DIRS    : Aggregate include dirs: base + tlibc + libcxx + protobuf
# - SSGX_ENV__EDL_SEARCH_PATHS       : Global EDL lookup paths
#
# [2] Intel SGX SSL Related Variables
# -----------------------------------
# - SSGX_ENV__SGXSSL                 : Root path of the SGX SSL installation
# - SSGX_ENV__SGXSSL_INCLUDE_DIR     : SSL headers
# - SSGX_ENV__SGXSSL_LIBRARY_DIR     : SSL libraries
#
# [3] Build Configuration Variables (User-configurable)
# -----------------------------------------------------
# - SSGX_ENV__BUILD_MODE             : Debug / PreRelease / Release
#     -> Set via `ssgx_set_build_mode(...)`
#
# - SSGX_ENV__HARDWARE_MODE          : ON / OFF
#     -> Set via `ssgx_set_hardware_mode(...)`
# ============================================================================================

include_guard(GLOBAL)

include("${CMAKE_CURRENT_LIST_DIR}/ssgx-utils.cmake")

# ============================================================================================
# Section 1: Detect and set SGX SDK root path
# - SSGX_ENV__SGXSDK
# ============================================================================================
if(DEFINED ENV{SGXSDK_DIR} AND EXISTS "$ENV{SGXSDK_DIR}")
    set(SSGX_ENV__SGXSDK "$ENV{SGXSDK_DIR}" CACHE PATH "SGX SDK path" FORCE)
elseif(EXISTS "/opt/intel/sgxsdk")
    set(SSGX_ENV__SGXSDK "/opt/intel/sgxsdk" CACHE PATH "SGX SDK path" FORCE)
else()
    message(FATAL_ERROR "[SSGX] Intel SGXSDK not found. Set SGXSDK or install to /opt/intel/sgxsdk.")
endif()

# ============================================================================================
# Section 2: Set SGX compile flags and toolchain paths based on architecture
# - SSGX_ENV__SGX_COMMON_CFLAGS
# - SSGX_ENV__SGX_ENCLAVE_SIGNER
# - SSGX_ENV__SGX_EDGER8R
# - SSGX_ENV__SGXSDK_LIBRARY_DIR
# - SSGX_ENV__SGXSDK_INCLUDE_DIR
# ============================================================================================
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(SSGX_ENV__SGX_COMMON_CFLAGS -m32 CACHE STRING "SGX common compile flags" FORCE)
    set(SSGX_ENV__SGX_ENCLAVE_SIGNER ${SSGX_ENV__SGXSDK}/bin/x86/sgx_sign CACHE FILEPATH "sgx_sign tool" FORCE)
    set(SSGX_ENV__SGX_EDGER8R ${SSGX_ENV__SGXSDK}/bin/x86/sgx_edger8r CACHE FILEPATH "sgx_edger8r tool" FORCE)
    set(SSGX_ENV__SGXSDK_LIBRARY_DIR ${SSGX_ENV__SGXSDK}/lib32 CACHE PATH "SGX SDK library directory" FORCE)
else()
    set(SSGX_ENV__SGX_COMMON_CFLAGS -m64 CACHE STRING "SGX common compile flags" FORCE)
    set(SSGX_ENV__SGX_ENCLAVE_SIGNER ${SSGX_ENV__SGXSDK}/bin/x64/sgx_sign CACHE FILEPATH "sgx_sign tool" FORCE)
    set(SSGX_ENV__SGX_EDGER8R ${SSGX_ENV__SGXSDK}/bin/x64/sgx_edger8r CACHE FILEPATH "sgx_edger8r tool" FORCE)
    set(SSGX_ENV__SGXSDK_LIBRARY_DIR ${SSGX_ENV__SGXSDK}/lib64 CACHE PATH "SGX SDK library directory" FORCE)
endif()
set(SSGX_ENV__SGXSDK_INCLUDE_DIR "${SSGX_ENV__SGXSDK}/include" CACHE PATH "SGX SDK include directory" FORCE)

ssgx_check_variable_path(SSGX_ENV__SGX_ENCLAVE_SIGNER "sgx_sign tool")
ssgx_check_variable_path(SSGX_ENV__SGX_EDGER8R "sgx_edger8r tool")
ssgx_check_variable_path(SSGX_ENV__SGXSDK_LIBRARY_DIR "SGX library directory")
ssgx_check_variable_path(SSGX_ENV__SGXSDK_INCLUDE_DIR "SGX include directory")
ssgx_check_path("${SSGX_ENV__SGXSDK_LIBRARY_DIR}/libsgx_urts.so" "SGX SDK library")
ssgx_check_path("${SSGX_ENV__SGXSDK_INCLUDE_DIR}/sgx.h" "SGX SDK header file")

# ============================================================================================
# Section 3: Include directories for SGX SDK  (tlibc, libcxx, protobuf)
# - SSGX_ENV__SGXSDK_INCLUDE_DIRS
# ============================================================================================
set(SSGX_ENV__SGX_TLIBC_INCLUDE_DIR "${SSGX_ENV__SGXSDK_INCLUDE_DIR}/tlibc" CACHE PATH "Intel SGX tlibc include directory" FORCE)
set(SSGX_ENV__SGX_LIBCXX_INCLUDE_DIR "${SSGX_ENV__SGXSDK_INCLUDE_DIR}/libcxx" CACHE PATH "Intel SGX libcxx include directory" FORCE)
set(SSGX_ENV__SGX_PROTOBUF_INCLUDE_DIR "${SSGX_ENV__SGXSDK_INCLUDE_DIR}/tprotobuf" CACHE PATH "Intel SGX protobuf include directory" FORCE)
set(SSGX_ENV__SGXSDK_INCLUDE_DIRS
        ${SSGX_ENV__SGXSDK_INCLUDE_DIR}
        ${SSGX_ENV__SGX_TLIBC_INCLUDE_DIR}
        ${SSGX_ENV__SGX_LIBCXX_INCLUDE_DIR}
        ${SSGX_ENV__SGX_PROTOBUF_INCLUDE_DIR}
        CACHE INTERNAL "SGX SDK include dirs"
)

mark_as_advanced(
        SSGX_ENV__SGXSDK_INCLUDE_DIR
        SSGX_ENV__SGX_TLIBC_INCLUDE_DIR
        SSGX_ENV__SGX_LIBCXX_INCLUDE_DIR
        SSGX_ENV__SGXSDK_LIBRARY_DIR)

ssgx_check_variable_path(SSGX_ENV__SGX_TLIBC_INCLUDE_DIR "Intel SGX tlibc include directory")
ssgx_check_variable_path(SSGX_ENV__SGX_LIBCXX_INCLUDE_DIR "Intel SGX libcxx include directory")
ssgx_check_variable_path(SSGX_ENV__SGX_PROTOBUF_INCLUDE_DIR "Intel SGX protobuf include directory")

# ============================================================================================
# Section 4: SGX SSL (Intel SGX Crypto Library) setup
# - SSGX_ENV__SGXSSL
# ============================================================================================
if(DEFINED ENV{SGXSSL_DIR} AND EXISTS "$ENV{SGXSSL_DIR}")
    set(SSGX_ENV__SGXSSL "$ENV{SGXSSL_DIR}" CACHE PATH "SGX SSL path" FORCE)
elseif(EXISTS "/opt/intel/sgxssl")
    set(SSGX_ENV__SGXSSL "/opt/intel/sgxssl" CACHE PATH "SGX SSL path" FORCE)
else()
    message(FATAL_ERROR "[SSGX] Intel SGXSSL not found. Set SGXSSL_DIR or install to /opt/intel/sgxssl.")
endif()

# ============================================================================================
# Section 5: Setup SGXSSL path and validate its headers/libraries
# - SSGX_ENV__SGXSSL_INCLUDE_DIR
# - SSGX_ENV__SGXSSL_LIBRARY_DIR
# ============================================================================================
set(SSGX_ENV__SGXSSL_INCLUDE_DIR ${SSGX_ENV__SGXSSL}/include CACHE PATH "SGX SSL include directory" FORCE)
set(SSGX_ENV__SGXSSL_LIBRARY_DIR ${SSGX_ENV__SGXSSL}/lib64 CACHE PATH "SGX SSL library directory" FORCE)

ssgx_check_variable_path(SSGX_ENV__SGXSSL_INCLUDE_DIR "Intel SGX SSL include directory")
ssgx_check_variable_path(SSGX_ENV__SGXSSL_LIBRARY_DIR "Intel SGX SSL library directory")
ssgx_check_path("${SSGX_ENV__SGXSSL_INCLUDE_DIR}/tSgxSSL_api.h" "Intel SGX SSL header file")
ssgx_check_path("${SSGX_ENV__SGXSSL_LIBRARY_DIR}/libsgx_tsgxssl.a" "Intel SGX SSL library file")

# ============================================================================================
# Section 6: Define global EDL search paths (used by sgx_edger8r)
# ============================================================================================
set(SSGX_ENV__EDL_SEARCH_PATHS ${SSGX_ENV__SGXSDK}/include /opt/safeheron/ssgx/include/mbedtls CACHE PATH "Search Paths for SSGX EDL files" FORCE)

# ============================================================================================
# Section 7: Enum Setup: BuildMode {Debug, PreRelease, Release}
# ============================================================================================
ssgx_define_enum(BuildMode Debug PreRelease Release)

# ============================================================================================
# Section 8: User Configuration API (build mode, hardware mode)
# ============================================================================================
function(ssgx_set_build_mode build_mode)
    ssgx_set_enum_var(t_build_mode BuildMode ${build_mode})
    set(SSGX_ENV__BUILD_MODE ${t_build_mode} CACHE INTERNAL  "Set SSGX Build Mode: ${t_build_mode}")
endfunction()

function(ssgx_set_hardware_mode enabled)
    string(TOUPPER "${enabled}" _val)
    if(NOT (_val STREQUAL "ON" OR _val STREQUAL "OFF"))
        message(FATAL_ERROR "[SSGX] SSGX_ENV__HARDWARE_MODE must be 'ON' or 'OFF'.")
    endif()

    set(SSGX_ENV__HARDWARE_MODE "${_val}" CACHE BOOL "Enable SSGX hardware mode: ${enabled}")
endfunction()

# ============================================================================================
# Section 9: Trusted Environment Resolver (used in SGX TEE)
# ============================================================================================
function(ssgx_get_trusted_env
    OUT_SGX_TRTS_LIB
    OUT_SGX_TSVC_LIB
    OUT_SGX_COMMON_CFLAGS
    OUT_ENCLAVE_INC_DIRS
    OUT_ENCLAVE_C_FLAGS
    OUT_ENCLAVE_CXX_FLAGS
)
    # Check if SSGX_ENV__HARDWARE_MODE is defined, and if not, set a default value
    if(NOT DEFINED SSGX_ENV__HARDWARE_MODE)
        set(SSGX_ENV__HARDWARE_MODE "ON" CACHE INTERNAL "SSGX_ENV__HARDWARE_MODE is set to default: ON (Enabled)")
    endif()
    if(${SSGX_ENV__HARDWARE_MODE})
        message(STATUS "SSGX_ENV__HARDWARE_MODE: Enabled")
        set(t_SGX_TRTS_LIB sgx_trts)
        set(t_SGX_TSVC_LIB sgx_tservice)
    else()
        message(STATUS "SSGX_ENV__HARDWARE_MODE: Disabled")
        set(t_SGX_TRTS_LIB sgx_trts_sim)
        set(t_SGX_TSVC_LIB sgx_tservice_sim)
    endif()

    if(NOT DEFINED SSGX_ENV__BUILD_MODE)
        set(SSGX_ENV__BUILD_MODE "PreRelease" CACHE INTERNAL "SSGX_ENV__BUILD_MODE is set to default: PreRelease")
    endif()
    if(${SSGX_ENV__BUILD_MODE} STREQUAL "Debug")
        message(STATUS "SSGX_ENV__BUILD_MODE: Debug")
        set(t_SGX_COMMON_CFLAGS ${SSGX_ENV__SGX_COMMON_CFLAGS} -O0 -g -DDEBUG -UNDEBUG -UEDEBUG -fmacro-prefix-map=${CMAKE_SOURCE_DIR}=/safeheron)
    elseif(${SSGX_ENV__BUILD_MODE} STREQUAL "PreRelease")
        message(STATUS "SSGX_ENV__BUILD_MODE: PreRelease")
        set(t_SGX_COMMON_CFLAGS ${SSGX_ENV__SGX_COMMON_CFLAGS} -O2 -UDEBUG -DNDEBUG -DEDEBUG -fmacro-prefix-map=${CMAKE_SOURCE_DIR}=/safeheron)
    elseif(${SSGX_ENV__BUILD_MODE} STREQUAL "Release")
        message(STATUS "SSGX_ENV__BUILD_MODE: Release")
        set(t_SGX_COMMON_CFLAGS ${SSGX_ENV__SGX_COMMON_CFLAGS} -O2 -UDEBUG -DNDEBUG -UEDEBUG -fmacro-prefix-map=${CMAKE_SOURCE_DIR}=/safeheron)
    else()
        message(FATAL_ERROR "Unknown build mode: ${SSGX_BUILD_MODE}")
    endif()

    set(t_ENCLAVE_INC_DIRS ${SSGX_ENV__SGXSDK_INCLUDE_DIRS})
    set(t_ENCLAVE_C_FLAGS ${t_SGX_COMMON_CFLAGS} -nostdinc -fvisibility=hidden -fpie -fstack-protector-strong)
    set(t_ENCLAVE_CXX_FLAGS ${t_ENCLAVE_C_FLAGS} -nostdinc++)

    set(${OUT_SGX_TRTS_LIB} ${t_SGX_TRTS_LIB} PARENT_SCOPE)
    set(${OUT_SGX_TSVC_LIB} ${t_SGX_TSVC_LIB} PARENT_SCOPE)
    set(${OUT_SGX_COMMON_CFLAGS} ${t_SGX_COMMON_CFLAGS} PARENT_SCOPE)
    set(${OUT_ENCLAVE_INC_DIRS} ${t_ENCLAVE_INC_DIRS} PARENT_SCOPE)
    set(${OUT_ENCLAVE_C_FLAGS} ${t_ENCLAVE_C_FLAGS} PARENT_SCOPE)
    set(${OUT_ENCLAVE_CXX_FLAGS} ${t_ENCLAVE_CXX_FLAGS} PARENT_SCOPE)
endfunction()

# ============================================================================================
# Section 10: Untrusted Environment Resolver (used in untrusted env)
# ============================================================================================
function(ssgx_get_untrusted_env
    OUT_SGX_URTS_LIB
    OUT_SGX_USVC_LIB
    OUT_SGX_COMMON_CFLAGS
    OUT_APP_INC_DIRS
    OUT_APP_C_FLAGS
    OUT_APP_CXX_FLAGS
)
    # Check if SSGX_ENV__HARDWARE_MODE is defined, and if not, set a default value
    if(NOT DEFINED SSGX_ENV__HARDWARE_MODE)
        set(SSGX_ENV__HARDWARE_MODE "ON" CACHE INTERNAL)
    endif()
    if(${SSGX_ENV__HARDWARE_MODE})
        message(STATUS "SSGX_ENV__HARDWARE_MODE: Enabled")
        set(t_SGX_URTS_LIB sgx_urts)
        set(t_SGX_USVC_LIB sgx_uae_service)
    else()
        message(STATUS "SSGX_ENV__HARDWARE_MODE: Disabled")
        set(t_SGX_URTS_LIB sgx_urts_sim)
        set(t_SGX_USVC_LIB sgx_uae_service_sim)
    endif()

    if(NOT DEFINED SSGX_ENV__BUILD_MODE)
        set(SSGX_ENV__BUILD_MODE "PreRelease" CACHE INTERNAL)
    endif()
    if(${SSGX_ENV__BUILD_MODE} STREQUAL "Debug")
        message(STATUS "SSGX_ENV__BUILD_MODE: Debug")
        set(t_SGX_COMMON_CFLAGS ${SSGX_ENV__SGX_COMMON_CFLAGS} -O0 -g -DDEBUG -UNDEBUG -UEDEBUG -fmacro-prefix-map=${CMAKE_SOURCE_DIR}=/safeheron)
    elseif(${SSGX_ENV__BUILD_MODE} STREQUAL "PreRelease")
        message(STATUS "SSGX_ENV__BUILD_MODE: PreRelease")
        set(t_SGX_COMMON_CFLAGS ${SSGX_ENV__SGX_COMMON_CFLAGS} -O2 -UDEBUG -DNDEBUG -DEDEBUG -fmacro-prefix-map=${CMAKE_SOURCE_DIR}=/safeheron)
    elseif(${SSGX_ENV__BUILD_MODE} STREQUAL "Release")
        message(STATUS "SSGX_ENV__BUILD_MODE: Release")
        set(t_SGX_COMMON_CFLAGS ${SSGX_ENV__SGX_COMMON_CFLAGS} -O2 -UDEBUG -DNDEBUG -UEDEBUG -fmacro-prefix-map=${CMAKE_SOURCE_DIR}=/safeheron)
    else()
        message(FATAL_ERROR "Unknown build mode: ${SSGX_ENV__BUILD_MODE}")
    endif()

    set(t_APP_INC_DIRS ${SSGX_ENV__SGXSDK_INCLUDE_DIR})
    set(t_APP_C_FLAGS ${SSGX_ENV__SGX_COMMON_CFLAGS} -fPIC -Wno-attributes)
    set(t_APP_CXX_FLAGS ${t_APP_C_FLAGS})

    set(${OUT_SGX_URTS_LIB} ${t_SGX_URTS_LIB} PARENT_SCOPE)
    set(${OUT_SGX_USVC_LIB} ${t_SGX_USVC_LIB} PARENT_SCOPE)
    set(${OUT_SGX_COMMON_CFLAGS} ${t_SGX_COMMON_CFLAGS} PARENT_SCOPE)
    set(${OUT_APP_INC_DIRS} ${t_APP_INC_DIRS} PARENT_SCOPE)
    set(${OUT_APP_C_FLAGS} ${t_APP_C_FLAGS} PARENT_SCOPE)
    set(${OUT_APP_CXX_FLAGS} ${t_APP_CXX_FLAGS} PARENT_SCOPE)
endfunction()
