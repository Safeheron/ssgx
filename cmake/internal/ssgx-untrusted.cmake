include_guard(GLOBAL)

include("${CMAKE_CURRENT_LIST_DIR}/ssgx-deps.cmake")

# --------------------------------------------------------------------------------------------------
# Add a library target called <target> that builds an SGX untrusted library.
#
# This function compiles and links the untrusted part of an SGX application,
# optionally generating EDL headers if provided. It resolves and links trusted/untrusted dependencies.
#
# Arguments:
#   <target>           - Name of the untrusted library target to create.
#   mode               - CMake library mode: STATIC, SHARED, etc.
#
# Options:
#   USE_PREFIX         - Pass --use-prefix to edger8r.
#
# Multi-value Arguments:
#   SRCS               - Source files to compile into the library.
#   EDL                - Path(s) to EDL file(s) (only one expected in most cases).
#   EDL_SEARCH_PATHS   - Search paths for resolving the EDL files.
#   UNTRUSTED_LIBS     - Other untrusted libraries this one depends on.
# --------------------------------------------------------------------------------------------------
function(ssgx_add_untrusted_library target mode)
    message("~~~~~~~~~~~~~~~~~~~~~~~~~~ Configure Target [${target}] ~~~~~~~~~~~~~~~~~~~~~~~~~~")
    ssgx_get_untrusted_env(
            SGX_URTS_LIB
            SGX_USVC_LIB
            SGX_COMMON_CFLAGS
            APP_INC_DIRS
            APP_C_FLAGS
            APP_CXX_FLAGS)
    # print local env
    foreach(var
            SGX_URTS_LIB
            SGX_USVC_LIB
            SGX_COMMON_CFLAGS
            APP_INC_DIRS
            APP_C_FLAGS
            APP_CXX_FLAGS
    )
        message(STATUS "${var} = [${${var}}]")
    endforeach()

    set(optionArgs USE_PREFIX)
    set(multiValueArgs SRCS UNTRUSTED_LIBS EDL EDL_SEARCH_PATHS)
    cmake_parse_arguments("SGX" "${optionArgs}" "" "${multiValueArgs}" ${ARGN})
    if("${SGX_EDL}" STREQUAL "")
        message(STATUS "${target}: SGX enclave edl file is not provided!")
    endif()
    if("${SGX_EDL_SEARCH_PATHS}" STREQUAL "")
        message(STATUS "[${target}]: SGX enclave edl file search paths are not provided!")
    endif()

    # Collect direct dependencies
    set(DIRECT_DEPS)
    foreach(ITEM ${SGX_UNTRUSTED_LIBS})
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

    add_library(${target} ${mode} ${SGX_SRCS})

    get_filename_component(EDL_NAME ${SGX_EDL} NAME_WE)
    ssgx_generate_untrusted_edl_header(${SGX_EDL} "${SGX_EDL_SEARCH_PATHS}" ${SGX_USE_PREFIX})
    add_dependencies(${target} ${target}-${SGX_EDL}-untrusted-headers)

    target_compile_options(${target} PRIVATE
            $<$<COMPILE_LANGUAGE:C>:${APP_C_FLAGS}>
            $<$<COMPILE_LANGUAGE:CXX>:${APP_CXX_FLAGS}>
    )
    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${APP_INC_DIRS})

    target_link_libraries(${target} PUBLIC ${COMPLETE_DEPS})

    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_CURRENT_BINARY_DIR}/${EDL_NAME}_u.h")
endfunction()


# --------------------------------------------------------------------------------------------------
# Add an executable target called <target> for an SGX untrusted application.
#
# This function compiles a host-side executable that communicates with one or more enclaves.
# It processes one or more EDL files to generate the required *_u.c/h files and links untrusted dependencies.
#
# Arguments:
#   <target>           - Name of the host-side executable to create.
#
# Options:
#   USE_PREFIX         - Pass --use-prefix to edger8r.
#
# Multi-value Arguments:
#   SRCS               - Source files to compile into the executable.
#   EDL                - List of EDL files required to interface with enclave(s).
#   EDL_SEARCH_PATHS   - Directories to search for EDL files.
#   UNTRUSTED_LIBS     - Libraries to link against (e.g., Poco::Foundation, etc.)
# --------------------------------------------------------------------------------------------------
function(ssgx_add_untrusted_executable target)
    message("~~~~~~~~~~~~~~~~~~~~~~~~~~ Configure Target [${target}] ~~~~~~~~~~~~~~~~~~~~~~~~~~")
    ssgx_get_untrusted_env(
            SGX_URTS_LIB
            SGX_USVC_LIB
            SGX_COMMON_CFLAGS
            APP_INC_DIRS
            APP_C_FLAGS
            APP_CXX_FLAGS)
    # print local env
    foreach(var
            SGX_URTS_LIB
            SGX_USVC_LIB
            SGX_COMMON_CFLAGS
            APP_INC_DIRS
            APP_C_FLAGS
            APP_CXX_FLAGS
    )
        message(STATUS "${var} = [${${var}}]")
    endforeach()

    set(optionArgs USE_PREFIX)
    set(multiValueArgs SRCS UNTRUSTED_LIBS EDL EDL_SEARCH_PATHS)
    cmake_parse_arguments("SGX" "${optionArgs}" "" "${multiValueArgs}" ${ARGN})
    if("${SGX_EDL}" STREQUAL "")
        message(FATAL_ERROR "${target}: SGX enclave edl file is not provided!")
    endif()
    if("${SGX_EDL_SEARCH_PATHS}" STREQUAL "")
        message(STATUS "[${target}]: SGX enclave edl file search paths are not provided!")
    endif()
    message(STATUS "EDL_SEARCH_PATHS: ${SGX_EDL_SEARCH_PATHS}")

    set(EDL_U_SRCS "")
    foreach(EDL ${SGX_EDL})
        get_filename_component(EDL_NAME ${EDL} NAME_WE)
        ssgx_generate_untrusted_edl_source(${EDL} "${SGX_EDL_SEARCH_PATHS}" ${SGX_USE_PREFIX})
#        add_dependencies(${target} ${target}-${SGX_EDL}-untrusted-headers)
        list(APPEND EDL_U_SRCS ${GENERATED_EDL_SOURCES})
    endforeach()

    # Collect direct dependencies
    set(DIRECT_DEPS)
    foreach(ITEM ${SGX_UNTRUSTED_LIBS})
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

    add_executable(${target} ${SGX_SRCS} ${EDL_U_SRCS})
    target_compile_options(${target} PRIVATE
            $<$<COMPILE_LANGUAGE:C>:${APP_C_FLAGS}>
            $<$<COMPILE_LANGUAGE:CXX>:${APP_CXX_FLAGS}>
    )
    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${APP_INC_DIRS})
    target_link_libraries(${target} PRIVATE
            -L${SSGX_ENV__SGXSDK_LIBRARY_DIR}
            -l${SGX_URTS_LIB}
            -l${SGX_USVC_LIB}
            -lsgx_ukey_exchange
            -lsgx_uprotected_fs
            -lpthread
            -lsgx_dcap_ql
            -lsgx_quote_ex
            -lsgx_dcap_quoteverify
            ${COMPLETE_DEPS}
    )
    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${EDL_U_HDRS})
endfunction()
