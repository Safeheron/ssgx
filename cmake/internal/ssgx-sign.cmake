# ============================================================================================
# ssgx-sign.cmake
# Sign an SGX enclave with sgx_sign, supporting one-step and two-step workflows
#
# Global variables required:
# - SSGX_ENV__SGX_ENCLAVE_SIGNER : path to sgx_sign (e.g., /opt/intel/sgxsdk/bin/x64/sgx_sign)
# ============================================================================================

include_guard(GLOBAL)

# ============================================================================================
# Function: ssgx_sign_enclave
#
# Description:
#   Sign an enclave binary to produce a `.signed.so` file using Intel's sgx_sign tool.
#   Supports one-step and two-step signing, with optional config and ignore flags.
#
# Parameters:
#   - target:          The enclave target to sign (must be already compiled)
#   - KEY:             Path to the PEM private key file (required)
#   - CONFIG:          (optional) XML config file for enclave metadata
#   - OUTPUT:          (optional) Name of the signed output (default: <target>.signed.so)
#   - IGNORE_INIT:     (flag) Ignore init section mismatch errors
#   - IGNORE_REL:      (flag) Ignore relocation errors
# ============================================================================================
function(ssgx_sign_enclave target)
    set(optionArgs IGNORE_INIT IGNORE_REL)
    set(oneValueArgs KEY CONFIG OUTPUT)
    cmake_parse_arguments("SGX" "${optionArgs}" "${oneValueArgs}" "" ${ARGN})

    if("${SGX_CONFIG}" STREQUAL "")
        message(STATUS "[${target}]: SGX enclave config is not provided!")
    else()
        get_filename_component(CONFIG_ABSPATH ${SGX_CONFIG} ABSOLUTE)
    endif()

    if("${SGX_KEY}" STREQUAL "")
       message(FATAL_ERROR "${target}: Private key used to sign enclave is not provided!")
    else()
        get_filename_component(KEY_ABSPATH ${SGX_KEY} ABSOLUTE)
    endif()

    if("${SGX_OUTPUT}" STREQUAL "")
        set(OUTPUT_NAME "${target}.signed.so")
    else()
        set(OUTPUT_NAME ${SGX_OUTPUT})
    endif()

    if(${SGX_IGNORE_INIT})
        set(IGN_INIT "-ignore-init-sec-error")
    endif()
    if(${SGX_IGNORE_REL})
        set(IGN_REL "-ignore-rel-error")
    endif()

    add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_NAME}
            COMMAND ${SSGX_ENV__SGX_ENCLAVE_SIGNER} sign -key ${KEY_ABSPATH}
            $<$<NOT:$<STREQUAL:${SGX_CONFIG},>>:-config> $<$<NOT:$<STREQUAL:${SGX_CONFIG},>>:${CONFIG_ABSPATH}>
            -enclave $<TARGET_FILE:${target}>
            -out $<TARGET_FILE_DIR:${target}>/${OUTPUT_NAME}
            ${IGN_INIT} ${IGN_REL}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS ${KEY_ABSPATH} ${CONFIG_ABSPATH} $<TARGET_FILE:${target}>
            COMMENT "Generating ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_NAME}"
    )

    add_custom_target(${target}-signed ALL
            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_NAME}
    )

    set(CLEAN_FILES "$<TARGET_FILE_DIR:${target}>/${OUTPUT_NAME};$<TARGET_FILE_DIR:${target}>/${target}_hash.hex")
    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${CLEAN_FILES}")

    add_dependencies(${target}-signed ${target})
endfunction()
