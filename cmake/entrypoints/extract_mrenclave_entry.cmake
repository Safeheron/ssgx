# extract_mrenclave_entry.cmake

# ============================================================================================
# Function: extract_mrenclave
# Description: Extracts MRENCLAVE (enclave_hash) from sgx_sign dump file.
#
# Parameters:
#   FILE               - Path to sgx_sign dump output file (e.g., enclave_dump.txt)
#   OUT_MRENCLAVE_VAR  - Name of the CMake variable to receive the extracted MRENCLAVE hex string
#
# Example:
#   extract_mrenclave(FILE <dump_file> OUT_MRENCLAVE_VAR <output_variable>)
#   extract_mrenclave(FILE ${FILE} OUT_MRENCLAVE_VAR __mren_value)
#   file(WRITE "${OUT}" "${__mren_value}")
# ============================================================================================
function(extract_mrenclave)
    cmake_parse_arguments(MREN "" "FILE;OUT_MRENCLAVE_VAR;SHELL_EXECUTABLE;SCRIPT_COLOR_ECHO" "" ${ARGN})
    if(NOT MREN_FILE OR NOT MREN_OUT_MRENCLAVE_VAR)
        message(FATAL_ERROR "Usage: extract_mrenclave(FILE <dump.txt> OUT_MRENCLAVE_VAR <var>)")
    endif()

    file(READ "${MREN_FILE}" _dump_content)
    string(REPLACE "\n" ";" _lines "${_dump_content}")

    set(_found_hash FALSE)
    set(_hash_line_count 0)
    set(_mren_lines "")

    foreach(line IN LISTS _lines)
        if(_found_hash)
            if(_hash_line_count LESS 2)
                string(APPEND _mren_lines "${line}")
                math(EXPR _hash_line_count "${_hash_line_count} + 1")
            endif()
        endif()

        if(line MATCHES "enclave_css\\.body\\.enclave_hash\\.m:")
            set(_found_hash TRUE)
        endif()
    endforeach()

    string(REPLACE "0x" "" _mren_lines "${_mren_lines}")
    string(REPLACE "," "" _mren_lines "${_mren_lines}")
    string(REPLACE " " "" _mren_lines "${_mren_lines}")

    set(${MREN_OUT_MRENCLAVE_VAR} "${_mren_lines}" PARENT_SCOPE)

    if(NOT MREN_SHELL_EXECUTABLE OR NOT MREN_SCRIPT_COLOR_ECHO)
        message(STATUS "[extract_mrenclave] MRENCLAVE: ${_mren_lines}")
    else ()
        execute_process(
                COMMAND ${MREN_SHELL_EXECUTABLE} ${MREN_SCRIPT_COLOR_ECHO} "[extract_mrenclave] MRENCLAVE: ${_mren_lines}" "GREEN"
        )
    endif()

endfunction()

if(DEFINED FILE AND DEFINED OUT_MRENCLAVE_VAR)
    extract_mrenclave(FILE ${FILE}
            OUT_MRENCLAVE_VAR ${OUT_MRENCLAVE_VAR}
            SHELL_EXECUTABLE ${SHELL_EXECUTABLE}
            SCRIPT_COLOR_ECHO ${SCRIPT_COLOR_ECHO}
    )
else()
    message(FATAL_ERROR "FILE and OUT_MRENCLAVE_VAR must be defined.")
endif()