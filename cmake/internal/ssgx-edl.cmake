# ============================================================================================
# ssgx-edl.cmake
# Functions to process SGX EDL files and generate corresponding headers and source files
# ============================================================================================
# Global variables used in this file:
# - SSGX_ENV__SGX_EDGER8R
# - SSGX_ENV__SGXSDK_INCLUDE_DIR
# ============================================================================================

include_guard(GLOBAL)

include("${CMAKE_CURRENT_LIST_DIR}/ssgx-utils.cmake")

# --------------------------------------------------------------------------------------------------
# Function: ssgx_generate_trusted_edl_header
# Description:
#   Generate only the trusted header (*.t.h) from the EDL file.
# Parameters:
#   - edl: The name of the EDL file.
#   - edl_search_paths: Additional search directories to find the EDL file.
#   - use_prefix: Boolean flag ("ON"/"OFF") to enable --use-prefix for edger8r.
# Outputs:
#   - Generates <name>_t.h in the binary dir.
#   - Declares custom target: <target>-<edl>-trusted-headers
# --------------------------------------------------------------------------------------------------
function(ssgx_generate_trusted_edl_header edl edl_search_paths use_prefix)
    get_filename_component(EDL_NAME ${edl} NAME_WE)
    set(SEARCH_PATHS "")
    foreach(path ${SSGX_ENV__EDL_SEARCH_PATHS})
        get_filename_component(ABSPATH ${path} ABSOLUTE)
        list(APPEND SEARCH_PATHS "${ABSPATH}")
    endforeach()
    foreach(path ${edl_search_paths})
        get_filename_component(ABSPATH ${path} ABSOLUTE)
        list(APPEND SEARCH_PATHS "${ABSPATH}")
    endforeach()
    string(REPLACE ";" ":" SEARCH_PATHS "${SEARCH_PATHS}")
    if(${use_prefix})
        set(USE_PREFIX "--use-prefix")
    endif()

    ssgx_split_string("${SEARCH_PATHS}" ":,;" SEARCH_PATH_LIST)

    set(EDL_FULL_PATH "")
    foreach(SEARCH_PATH ${SEARCH_PATH_LIST})
        if (EXISTS "${SEARCH_PATH}/${edl}")
            set(EDL_FULL_PATH "${SEARCH_PATH}/${edl}")
            break()
        endif()
    endforeach()

    if (NOT EDL_FULL_PATH)
        message(FATAL_ERROR "File '${edl}' not found in SEARCH_PATHS: ${SEARCH_PATHS}")
    endif()

    add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${EDL_NAME}_t.h
            COMMENT "Generating ${EDL_NAME}_t.h in directory ${CMAKE_CURRENT_BINARY_DIR}"
            COMMAND ${CMAKE_COMMAND} -E echo "    -- Generating ${EDL_NAME}_t.h in directory ${CMAKE_CURRENT_BINARY_DIR}"  # For Ninja
            COMMAND ${SSGX_ENV__SGX_EDGER8R} ${USE_PREFIX} --header-only --trusted ${edl} --search-path ${SEARCH_PATHS}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS ${EDL_FULL_PATH}
    )

    add_custom_target(${target}-${edl}-trusted-headers
            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${EDL_NAME}_t.h
            )
    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_CURRENT_BINARY_DIR}/${EDL_NAME}_t.h")
endfunction()

# --------------------------------------------------------------------------------------------------
# Function: ssgx_generate_untrusted_edl_header
# Description:
#   Generate only the untrusted header (*.u.h) from the EDL file.
# Parameters:
#   - edl: EDL filename.
#   - edl_search_paths: Paths to search for the EDL file.
#   - use_prefix: Enables `--use-prefix` if ON.
# Outputs:
#   - Generates <name>_u.h in the binary dir.
#   - Declares custom target: <target>-<edl>-untrusted-headers
# --------------------------------------------------------------------------------------------------
function(ssgx_generate_untrusted_edl_header edl edl_search_paths use_prefix)
    get_filename_component(EDL_NAME ${edl} NAME_WE)
    set(SEARCH_PATHS "")
    foreach(path ${SSGX_ENV__EDL_SEARCH_PATHS})
        get_filename_component(ABSPATH ${path} ABSOLUTE)
        list(APPEND SEARCH_PATHS "${ABSPATH}")
    endforeach()
    foreach(path ${edl_search_paths})
        get_filename_component(ABSPATH ${path} ABSOLUTE)
        list(APPEND SEARCH_PATHS "${ABSPATH}")
    endforeach()
    list(APPEND SEARCH_PATHS "${SGX_PATH}/include")
    string(REPLACE ";" ":" SEARCH_PATHS "${SEARCH_PATHS}")
    if(${use_prefix})
        set(USE_PREFIX "--use-prefix")
    endif()

    ssgx_split_string("${SEARCH_PATHS}" ":,;" SEARCH_PATH_LIST)

    set(EDL_FULL_PATH "")
    foreach(SEARCH_PATH ${SEARCH_PATH_LIST})
        if (EXISTS "${SEARCH_PATH}/${edl}")
            set(EDL_FULL_PATH "${SEARCH_PATH}/${edl}")
            break()
        endif()
    endforeach()

    if (NOT EDL_FULL_PATH)
        message(FATAL_ERROR "File '${edl}' not found in SEARCH_PATHS: ${SEARCH_PATHS}")
    endif()

    add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${EDL_NAME}_u.h
            COMMENT "Generating ${EDL_NAME}_u.h in directory ${CMAKE_CURRENT_BINARY_DIR}"
            COMMAND ${CMAKE_COMMAND} -E echo "    -- Generating ${EDL_NAME}_u.h in directory ${CMAKE_CURRENT_BINARY_DIR}"  # For Ninja
            COMMAND ${SSGX_ENV__SGX_EDGER8R} ${USE_PREFIX} --header-only --untrusted ${edl} --search-path ${SEARCH_PATHS}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS ${EDL_FULL_PATH}
            COMMENT "Generating ${EDL_NAME}_u.h"
    )

    add_custom_target(${target}-${edl}-untrusted-headers
            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${EDL_NAME}_u.h
            )
    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_CURRENT_BINARY_DIR}/${EDL_NAME}_u.h")
endfunction()


# --------------------------------------------------------------------------------------------------
# Function: ssgx_generate_trusted_edl_source
# Description:
#   Generate both header (*.t.h) and source (*.t.c) files for trusted side from EDL.
# Parameters:
#   - edl: EDL filename.
#   - edl_search_paths: EDL lookup paths.
#   - use_prefix: Enables --use-prefix.
# Outputs:
#   - Sets GENERATED_EDL_SOURCES and GENERATED_EDL_HEADERS for use in target declaration.
# --------------------------------------------------------------------------------------------------
function(ssgx_generate_trusted_edl_source edl edl_search_paths use_prefix)
    get_filename_component(EDL_NAME ${edl} NAME_WE)
    # Find the absolute path of the EDL file in `edl_search_paths`
    set(SEARCH_PATHS "")
    foreach(path ${SSGX_ENV__EDL_SEARCH_PATHS})
        get_filename_component(ABSPATH ${path} ABSOLUTE)
        list(APPEND SEARCH_PATHS "${ABSPATH}")
    endforeach()
    foreach(path ${edl_search_paths})
        get_filename_component(ABSPATH ${path} ABSOLUTE)
        list(APPEND SEARCH_PATHS "${ABSPATH}")
    endforeach()
    #    list(APPEND SEARCH_PATHS "${SSGX_ENV__SGXSDK_INCLUDE_DIR}")
    string(REPLACE ";" ":" SEARCH_PATHS "${SEARCH_PATHS}")
    if(${use_prefix})
        set(USE_PREFIX "--use-prefix")
    endif()

    ssgx_split_string("${SEARCH_PATHS}" ":,;" SEARCH_PATH_LIST)

    set(EDL_FULL_PATH "")
    foreach(SEARCH_PATH ${SEARCH_PATH_LIST})
        if (EXISTS "${SEARCH_PATH}/${edl}")
            set(EDL_FULL_PATH "${SEARCH_PATH}/${edl}")
            break()
        endif()
    endforeach()

    if (NOT EDL_FULL_PATH)
        message(FATAL_ERROR "File '${edl}' not found in SEARCH_PATHS: ${SEARCH_PATHS}")
    endif()

    set(EDL_C "${CMAKE_CURRENT_BINARY_DIR}/${EDL_NAME}_t.c")
    set(EDL_H "${CMAKE_CURRENT_BINARY_DIR}/${EDL_NAME}_t.h")

    add_custom_command(
            OUTPUT ${EDL_C} ${EDL_H}
            COMMENT "Generating ${EDL_NAME}_t.h, ${EDL_NAME}_t.c in directory ${CMAKE_CURRENT_BINARY_DIR}"
            COMMAND ${CMAKE_COMMAND} -E echo "    -- Generating ${EDL_NAME}_t.h, ${EDL_NAME}_t.c in directory ${CMAKE_CURRENT_BINARY_DIR}"  # For Ninja
            COMMAND ${SSGX_ENV__SGX_EDGER8R} ${USE_PREFIX} --trusted ${edl} --search-path ${SEARCH_PATHS}
            MAIN_DEPENDENCY ${EDL_FULL_PATH} # Track changes to the EDL file
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    set(GENERATED_EDL_SOURCES ${EDL_C} PARENT_SCOPE)
    set(GENERATED_EDL_HEADERS ${EDL_H} PARENT_SCOPE)
endfunction()


# --------------------------------------------------------------------------------------------------
# Function: ssgx_generate_untrusted_edl_source
# Description:
#   Generate both header (*.u.h) and source (*.u.c) files for untrusted side from EDL.
# Parameters:
#   - edl: EDL filename.
#   - edl_search_paths: EDL lookup paths.
#   - use_prefix: Enables --use-prefix.
# Outputs:
#   - Sets GENERATED_EDL_SOURCES and GENERATED_EDL_HEADERS
# --------------------------------------------------------------------------------------------------
function(ssgx_generate_untrusted_edl_source edl edl_search_paths use_prefix)
    get_filename_component(EDL_NAME ${edl} NAME_WE)
    # Find the absolute path of the EDL file in `edl_search_paths`
    set(SEARCH_PATHS "")
    foreach(path ${SSGX_ENV__EDL_SEARCH_PATHS})
        get_filename_component(ABSPATH ${path} ABSOLUTE)
        list(APPEND SEARCH_PATHS "${ABSPATH}")
    endforeach()
    foreach(path ${edl_search_paths})
        get_filename_component(ABSPATH ${path} ABSOLUTE)
        list(APPEND SEARCH_PATHS "${ABSPATH}")
    endforeach()
    #    list(APPEND SEARCH_PATHS "${SSGX_ENV__SGXSDK_INCLUDE_DIR}")
    string(REPLACE ";" ":" SEARCH_PATHS "${SEARCH_PATHS}")
    if(${use_prefix})
        set(USE_PREFIX "--use-prefix")
    endif()

    ssgx_split_string("${SEARCH_PATHS}" ":,;" SEARCH_PATH_LIST)

    set(EDL_FULL_PATH "")
    foreach(SEARCH_PATH ${SEARCH_PATH_LIST})
        if (EXISTS "${SEARCH_PATH}/${edl}")
            set(EDL_FULL_PATH "${SEARCH_PATH}/${edl}")
            break()
        endif()
    endforeach()

    if (NOT EDL_FULL_PATH)
        message(FATAL_ERROR "File '${edl}' not found in SEARCH_PATHS: ${SEARCH_PATHS}")
    endif()

    set(EDL_C "${CMAKE_CURRENT_BINARY_DIR}/${EDL_NAME}_u.c")
    set(EDL_H "${CMAKE_CURRENT_BINARY_DIR}/${EDL_NAME}_u.h")

    add_custom_command(
            OUTPUT ${EDL_C} ${EDL_H}
            COMMENT "Generating ${EDL_NAME}_u.h, ${EDL_NAME}_u.c in directory ${CMAKE_CURRENT_BINARY_DIR}"
            COMMAND ${CMAKE_COMMAND} -E echo "    -- Generating ${EDL_NAME}_u.h, ${EDL_NAME}_u.c in directory ${CMAKE_CURRENT_BINARY_DIR}"  # For Ninja
            COMMAND ${SSGX_ENV__SGX_EDGER8R} ${USE_PREFIX} --untrusted ${edl} --search-path ${SEARCH_PATHS}
            MAIN_DEPENDENCY ${EDL_FULL_PATH} # Track changes to the EDL file
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    set(GENERATED_EDL_SOURCES ${EDL_C} PARENT_SCOPE)
    set(GENERATED_EDL_HEADERS ${EDL_H} PARENT_SCOPE)
endfunction()
