include_guard(GLOBAL)

include("${CMAKE_CURRENT_LIST_DIR}/ssgx-deps.cmake")

# ============================================================================================
# Add a library target called <target> that builds a static library for use within an SGX enclave.
#
# This function simplifies the process of compiling and linking trusted enclave code.
# It sets appropriate compiler and linker flags based on the current build and hardware mode,
# handles EDL processing to generate trusted headers, and links against SGX runtime libraries.
#
# Usage:
#   ssgx_add_trusted_library(<target>
#     SRCS <source1> <source2> ...
#     EDL <edl_file>
#     EDL_SEARCH_PATHS <path1> <path2> ...
#     TRUSTED_LIBS <lib1> <lib2> ...
#     [USE_PREFIX]
#     [USE_SGXSSL]
#     [LDSCRIPT <ld_version_script>]
#   )
#
# Options:
#   USE_PREFIX         : Enable edger8r --use-prefix flag when generating headers.
#   USE_SGXSSL         : Link against Intel SGX SSL libraries.
#
# One-value arguments:
#   EDL                : EDL file describing enclave interface (optional).
#   LDSCRIPT           : Custom linker script for versioning (optional).
#
# Multi-value arguments:
#   SRCS               : List of source files (.cpp/.c) to compile into the enclave.
#   TRUSTED_LIBS       : List of dependent trusted libraries.
#   EDL_SEARCH_PATHS   : Directories to search for EDL files.
#
# Notes:
# - If no EDL is provided, header generation and dependencies are skipped.
# - Internal compile flags are determined automatically via ssgx_get_trusted_env.
# ============================================================================================
function(ssgx_add_trusted_library target)
    message("~~~~~~~~~~~~~~~~~~~~~~~~~~ Configure Target [${target}] ~~~~~~~~~~~~~~~~~~~~~~~~~~")
    ssgx_get_trusted_env(
            SGX_TRTS_LIB
            SGX_TSVC_LIB
            SGX_COMMON_CFLAGS
            ENCLAVE_INC_DIRS
            ENCLAVE_C_FLAGS
            ENCLAVE_CXX_FLAGS)
    # print local env
    foreach(var
            SGX_TRTS_LIB
            SGX_TSVC_LIB
            SGX_COMMON_CFLAGS
            ENCLAVE_INC_DIRS
            ENCLAVE_C_FLAGS
            ENCLAVE_CXX_FLAGS
    )
        message(STATUS "${var} = [${${var}}]")
    endforeach()

    set(optionArgs USE_PREFIX USE_SGXSSL)
    set(oneValueArgs EDL LDSCRIPT)
    set(multiValueArgs SRCS EDL_SEARCH_PATHS TRUSTED_LIBS)
    cmake_parse_arguments("SGX" "${optionArgs}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    if(NOT "${SGX_LDSCRIPT}" STREQUAL "")
        get_filename_component(LDS_ABSPATH ${SGX_LDSCRIPT} ABSOLUTE)
        set(LDSCRIPT_FLAG "-Wl,--version-script=${LDS_ABSPATH}")
    endif()
    if("${SGX_EDL}" STREQUAL "")
        message(STATUS "[${target}]: SGX enclave edl file is not provided; skipping edger8r")
        add_library(${target} STATIC ${SGX_SRCS})
    else()
        if("${SGX_EDL_SEARCH_PATHS}" STREQUAL "")
            message(STATUS "[${target}]: SGX enclave edl file search paths are not provided!")
        endif()
        add_library(${target} STATIC ${SGX_SRCS})
        ssgx_generate_trusted_edl_header(${SGX_EDL} "${SGX_EDL_SEARCH_PATHS}" ${SGX_USE_PREFIX})
        add_dependencies(${target} ${target}-${SGX_EDL}-trusted-headers)
        set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_CURRENT_BINARY_DIR}/${EDL_NAME}_t.h")
        message(STATUS "EDL_SEARCH_PATHS = ${SGX_EDL_SEARCH_PATHS}")
    endif()

    target_compile_options(${target} PRIVATE
            $<$<COMPILE_LANGUAGE:C>:${ENCLAVE_C_FLAGS}>
            $<$<COMPILE_LANGUAGE:CXX>:${ENCLAVE_CXX_FLAGS}>
    )
    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${ENCLAVE_INC_DIRS})

    # Collect direct dependencies
    set(DIRECT_DEPS)
    foreach(ITEM ${SGX_TRUSTED_LIBS})
        list(APPEND DIRECT_DEPS ${ITEM})
    endforeach()
    message(STATUS "DIRECT_DEPS = ${DIRECT_DEPS}")

    # Retrieve all indirect dependencies
    ssgx_get_all_dependencies(INDIRECT_DEPS ${DIRECT_DEPS})
    message(STATUS "INDIRECT_DEPS = ${INDIRECT_DEPS}")

    # Merge direct and indirect dependencies into a complete dependency list
    set(COMPLETE_DEPS ${DIRECT_DEPS})
    list(APPEND COMPLETE_DEPS ${INDIRECT_DEPS})
    list(REMOVE_DUPLICATES COMPLETE_DEPS)  # Ensure uniqueness
    message(STATUS "COMPLETE_DEPS = ${COMPLETE_DEPS}")

    if(${SGX_USE_SGXSSL})
        target_include_directories(${target} PRIVATE ${SSGX_ENV__SGXSSL_INCLUDE_PATH})
        target_link_libraries(${target} PUBLIC ${COMPLETE_DEPS})
        target_link_options(${target} PRIVATE ${SGX_COMMON_CFLAGS}
                -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L${SSGX_ENV__SGXSDK_LIBRARY_DIR} -L${SSGX_ENV__SGXSSL_LIBRARY_DIR}
                -Wl,--whole-archive -l${SGX_TRTS_LIB} -lsgx_tsgxssl -Wl,--no-whole-archive
                -Wl,--start-group -lsgx_dcap_tvl -lsgx_tsgxssl_crypto -lsgx_tstdc -lsgx_pthread -lsgx_tcxx -lsgx_tkey_exchange -lsgx_tcrypto -l${SGX_TSVC_LIB} -lsgx_tprotected_fs -lsgx_protobuf -Wl,--end-group
                -Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined
                -Wl,-pie,-eenclave_entry -Wl,--export-dynamic
                ${LDSCRIPT_FLAG}
                -Wl,--defsym,__ImageBase=0)
    else()
        target_link_libraries(${target} PUBLIC ${COMPLETE_DEPS})
        target_link_options(${target} PRIVATE ${SGX_COMMON_CFLAGS}
                -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L${SSGX_ENV__SGXSSL_LIBRARY_DIR}
                -Wl,--whole-archive -l${SGX_TRTS_LIB} -Wl,--no-whole-archive
                -Wl,--start-group -lsgx_dcap_tvl -lsgx_tstdc -lsgx_pthread -lsgx_tcxx -lsgx_tkey_exchange -lsgx_tcrypto -l${SGX_TSVC_LIB} -lsgx_tprotected_fs -lsgx_protobuf -Wl,--end-group
                -Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined
                -Wl,-pie,-eenclave_entry -Wl,--export-dynamic
                ${LDSCRIPT_FLAG}
                -Wl,--defsym,__ImageBase=0)
    endif()
endfunction()

# --------------------------------------------------------------------------------------------------
# Add an enclave target called <target> built as a shared library.
#
# This function sets up a complete SGX enclave build pipeline, including:
#   - Compiling and linking trusted source files.
#   - Generating EDL-based trusted source code via `sgx_edger8r`.
#   - Applying SGX-specific compiler and linker flags.
#   - Managing optional SGXSSL linkage and enclave link script.
#
# Usage:
#   ssgx_add_enclave_library(<target>
#     SRCS <sources...>
#     EDL <edl_file>
#     EDL_SEARCH_PATHS <paths...>
#     TRUSTED_LIBS <dependencies...>
#     [USE_PREFIX]
#     [USE_SGXSSL]
#     [LDSCRIPT <linker_script>]
#   )
#
# Arguments:
#   <target>               Name of the enclave target.
#
# Options:
#   USE_PREFIX             Enable `--use-prefix` for `sgx_edger8r`.
#   USE_SGXSSL             Link against SGXSSL (tlib + crypto).
#
# One-value arguments:
#   EDL                   Path to the EDL file used to generate trusted code.
#   LDSCRIPT              Path to the enclave linker script (.lds).
#
# Multi-value arguments:
#   SRCS                  Trusted source files (.cpp, .c).
#   EDL_SEARCH_PATHS      Additional paths for resolving EDL imports.
#   TRUSTED_LIBS          Other trusted static libraries to link with this enclave.
#
# Output:
#   <target>              A shared library target representing a signed enclave (.so).
# --------------------------------------------------------------------------------------------------
function(ssgx_add_enclave_library target)
    message("~~~~~~~~~~~~~~~~~~~~~~~~~~ Configure Target [${target}] ~~~~~~~~~~~~~~~~~~~~~~~~~~")
    ssgx_get_trusted_env(
            SGX_TRTS_LIB
            SGX_TSVC_LIB
            SGX_COMMON_CFLAGS
            ENCLAVE_INC_DIRS
            ENCLAVE_C_FLAGS
            ENCLAVE_CXX_FLAGS)
    # print local env
    foreach(var
            SGX_TRTS_LIB
            SGX_TSVC_LIB
            SGX_COMMON_CFLAGS
            ENCLAVE_INC_DIRS
            ENCLAVE_C_FLAGS
            ENCLAVE_CXX_FLAGS
    )
        message(STATUS "${var} = [${${var}}]")
    endforeach()

    set(optionArgs USE_PREFIX USE_SGXSSL)
    set(oneValueArgs EDL LDSCRIPT)
    set(multiValueArgs SRCS TRUSTED_LIBS EDL_SEARCH_PATHS)
    cmake_parse_arguments("SGX" "${optionArgs}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    if("${SGX_EDL}" STREQUAL "")
        message(FATAL_ERROR "${target}: SGX enclave edl file is not provided!")
    endif()
    if("${SGX_EDL_SEARCH_PATHS}" STREQUAL "")
        message(STATUS "[${target}]: SGX enclave edl file search paths are not provided!")
    endif()
    if(NOT "${SGX_LDSCRIPT}" STREQUAL "")
        get_filename_component(LDS_ABSPATH ${SGX_LDSCRIPT} ABSOLUTE)
        set(LDSCRIPT_FLAG "-Wl,--version-script=${LDS_ABSPATH}")
    endif()

    message(STATUS "EDL_SEARCH_PATHS: ${SGX_EDL_SEARCH_PATHS}")
    ssgx_generate_trusted_edl_source(${SGX_EDL} "${SGX_EDL_SEARCH_PATHS}" ${SGX_USE_PREFIX})

    add_library(${target} SHARED ${SGX_SRCS} ${GENERATED_EDL_SOURCES})
    target_compile_options(${target} PRIVATE
            $<$<COMPILE_LANGUAGE:C>:${ENCLAVE_C_FLAGS}>
            $<$<COMPILE_LANGUAGE:CXX>:${ENCLAVE_CXX_FLAGS}>
    )
    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${ENCLAVE_INC_DIRS})

    # Collect direct dependencies
    set(DIRECT_DEPS)
    foreach(ITEM ${SGX_TRUSTED_LIBS})
        list(APPEND DIRECT_DEPS ${ITEM})
    endforeach()
    message(STATUS "DIRECT_DEPS = ${DIRECT_DEPS}")

    # Retrieve all indirect dependencies
    ssgx_get_all_dependencies(INDIRECT_DEPS ${DIRECT_DEPS})
    message(STATUS "INDIRECT_DEPS = ${INDIRECT_DEPS}")

    # Merge direct and indirect dependencies into a complete dependency list
    set(COMPLETE_DEPS ${DIRECT_DEPS})
    list(APPEND COMPLETE_DEPS ${INDIRECT_DEPS})
    list(REMOVE_DUPLICATES COMPLETE_DEPS)  # Ensure uniqueness
    message(STATUS "COMPLETE_DEPS = ${COMPLETE_DEPS}")

    if(${SGX_USE_SGXSSL})
        target_include_directories(${target} PRIVATE ${SSGX_ENV__SGXSSL_INCLUDE_PATH})
        target_link_libraries(${target} PRIVATE ${SGX_COMMON_CFLAGS}
                -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L${SSGX_ENV__SGXSDK_LIBRARY_DIR} -L${SSGX_ENV__SGXSSL_LIBRARY_DIR}
                -Wl,--whole-archive -l${SGX_TRTS_LIB} -lsgx_tsgxssl -Wl,--no-whole-archive
                -Wl,--start-group ${COMPLETE_DEPS} -lsgx_dcap_tvl -lsgx_tsgxssl_crypto -lsgx_tstdc -lsgx_pthread -lsgx_tcxx -lsgx_tkey_exchange -lsgx_tcrypto -l${SGX_TSVC_LIB} -lsgx_tprotected_fs -lsgx_protobuf -Wl,--end-group
                -Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined
                -Wl,-pie,-eenclave_entry -Wl,--export-dynamic
                ${LDSCRIPT_FLAG}
                -Wl,--defsym,__ImageBase=0)
    else()
        target_link_libraries(${target} PRIVATE ${SGX_COMMON_CFLAGS}
                -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L${SSGX_ENV__SGXSSL_LIBRARY_DIR}
                -Wl,--whole-archive -l${SGX_TRTS_LIB} -Wl,--no-whole-archive
                -Wl,--start-group ${COMPLETE_DEPS} -lsgx_dcap_tvl -lsgx_tstdc -lsgx_tcxx -lsgx_tkey_exchange -lsgx_tcrypto -l${SGX_TSVC_LIB} -lsgx_tprotected_fs -lsgx_protobuf -Wl,--end-group
                -Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined
                -Wl,-pie,-eenclave_entry -Wl,--export-dynamic
                ${LDSCRIPT_FLAG}
                -Wl,--defsym,__ImageBase=0)
    endif()
endfunction()