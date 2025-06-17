include_guard(GLOBAL)
include(CMakeParseArguments) # Required module for cmake_parse_arguments

# ------------------------------------------------------------------------------
# Function: ssgx_check_variable_path
# Description:
#   Check if a given variable is defined and the path it refers to exists.
#
# Parameters:
#   - var  : The name of the variable (not its value)
#   - desc : Human-readable description used in error messages
#
# Example:
#   set(SGX_INCLUDE_DIR "/opt/intel/sgxsdk/include")
#   ssgx_check_variable_path(SGX_INCLUDE_DIR "SGX include directory")
# ------------------------------------------------------------------------------
function(ssgx_check_variable_path var desc)
    if(NOT DEFINED ${var})
        message(FATAL_ERROR "Variable '${var}' is not defined!")
    endif()
    if(NOT EXISTS "${${var}}")
        message(FATAL_ERROR "${desc} not found: ${${var}}")
    endif()
endfunction()


# ============================================================================================
# Function: ssgx_check_path
# Description:
#   Check if the given literal path exists.
#
# Parameters:
#   - path : The full path to check
#   - desc : Human-readable description used in error messages
#
# Example:
#   ssgx_check_path("${SGX_INCLUDE_DIR}/sgx.h" "SGX header file")
# ============================================================================================
function(ssgx_check_path path desc)
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "${desc} not found: ${path}")
    endif()
endfunction()


# ============================================================================================
# Function: ssgx_split_string
# Description:
#   Splits an input string into a list of substrings based on one or more delimiters.
# Parameters:
#   - input_string: The input string to split.
#   - delimiters: A string containing all the delimiters (e.g., ":,;").
#   - output_list: The name of the variable to store the resulting list.
# ============================================================================================
function(ssgx_split_string input_string delimiters output_list)
    set(result "")
    set(work "${input_string}")

    string(REGEX REPLACE "([][.*+?^$(){}|\\])" "\\\\\\1" esc_delims "${delimiters}")

    while(work MATCHES "^([^${esc_delims}]+)[${esc_delims}]*")
        string(REGEX MATCH "^([^${esc_delims}]+)[${esc_delims}]*" match "${work}")
        list(APPEND result "${CMAKE_MATCH_1}")
        string(LENGTH "${match}" match_len)
        string(SUBSTRING "${work}" ${match_len} -1 work)
    endwhile()

    set(${output_list} "${result}" PARENT_SCOPE)
endfunction()

# ============================================================================================
# Define an enum-like global variable in CMake
#
# Usage:
#   ssgx_define_enum(BuildMode Debug PreRelease Release)
#   => Registers a set of allowed values for BuildMode
#
# Parameters:
#   - enum_name: Name of the enum category (e.g., BuildMode)
#   - ...       : List of allowed string values
# ============================================================================================
function(ssgx_define_enum enum_name)
    set(enum_values ${ARGN})
    set_property(GLOBAL PROPERTY SSGX_ENUM_${enum_name} "${enum_values}")
endfunction()

# ============================================================================================
# Set a variable with a value from a defined enum set (with validation)
#
# Usage:
#   ssgx_set_enum_var(my_var BuildMode Debug)
#   => Sets `my_var` to "Debug" if it's a valid value of the BuildMode enum
#
# Parameters:
#   - var_name   : Output variable name (caller-supplied)
#   - enum_name  : Enum category previously defined (e.g., BuildMode)
#   - value      : The value to assign; must be in enum list
#
# Output:
#   - Sets `${var_name}` in the PARENT_SCOPE if value is valid
#   - Fails with `FATAL_ERROR` if value is not allowed
# ============================================================================================
function(ssgx_set_enum_var var_name enum_name value)
    get_property(valid_values GLOBAL PROPERTY SSGX_ENUM_${enum_name})
    list(FIND valid_values "${value}" index)
    if(index EQUAL -1)
        message(FATAL_ERROR "[SSGX] Invalid value '${value}' for enum '${enum_name}'. Expected one of: ${valid_values}")
    endif()
    set(${var_name} "${value}" PARENT_SCOPE)
endfunction()


# ============================================================================================
# Function: ssgx_ensure_rsa_key_exists
# Description: Checks if the specified private key file exists. If not, generates a new one using openssl.
# Arguments:
#   KEY_FILE_PATH (Required): The full path to the private key file to check or create.
#   KEY_SIZE (Optional): The number of bits to use when generating the key (Default: 3072).
#   OPENSSL_EXE (Optional): The path to the openssl executable. If not provided, it will be searched for automatically.
# ============================================================================================
function(ssgx_ensure_rsa_key_exists)
    # Define the arguments accepted by the function
    set(options "") # No boolean options
    set(oneValueArgs KEY_FILE_PATH KEY_SIZE OPENSSL_EXE) # Arguments that take one value
    set(multiValueArgs "") # No arguments that take multiple values

    # Parse the arguments passed to the function (${ARGN} is the list of all arguments)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # --- 1. Validate required arguments ---
    if(NOT DEFINED ARG_KEY_FILE_PATH OR ARG_KEY_FILE_PATH STREQUAL "")
        message(FATAL_ERROR "ssgx_ensure_rsa_key_exists: Error - KEY_FILE_PATH argument is required and cannot be empty.")
    endif()

    # --- 2. Set default values ---
    if(NOT DEFINED ARG_KEY_SIZE OR ARG_KEY_SIZE STREQUAL "")
        set(ARG_KEY_SIZE 3072) # Default key size
        message(VERBOSE "ssgx_ensure_rsa_key_exists: KEY_SIZE not specified, using default value ${ARG_KEY_SIZE}.")
    endif()

    # --- 3. Find or validate the openssl executable ---
    set(LOCAL_OPENSSL_EXE "")
    if(DEFINED ARG_OPENSSL_EXE AND NOT ARG_OPENSSL_EXE STREQUAL "")
        if(EXISTS "${ARG_OPENSSL_EXE}")
            set(LOCAL_OPENSSL_EXE "${ARG_OPENSSL_EXE}")
            message(VERBOSE "ssgx_ensure_rsa_key_exists: Using provided OpenSSL path: ${LOCAL_OPENSSL_EXE}")
        else()
            message(WARNING "ssgx_ensure_rsa_key_exists: Provided OPENSSL_EXE path does not exist: ${ARG_OPENSSL_EXE}. Attempting to find automatically.")
        endif()
    endif()

    # If a valid path was not provided, search automatically
    if(LOCAL_OPENSSL_EXE STREQUAL "")
        find_program(FOUND_OPENSSL_EXE openssl)
        if(NOT FOUND_OPENSSL_EXE)
            message(FATAL_ERROR "ssgx_ensure_rsa_key_exists: Error - Could not find the 'openssl' executable. Please ensure OpenSSL is installed and added to the PATH, or specify the path via the OPENSSL_EXE argument.")
        else()
            set(LOCAL_OPENSSL_EXE ${FOUND_OPENSSL_EXE})
            message(VERBOSE "ssgx_ensure_rsa_key_exists: Found OpenSSL: ${LOCAL_OPENSSL_EXE}")
        endif()
    endif()


    # --- 4. Check if the key file exists, create if not ---
    message(STATUS "Checking for private key file: ${ARG_KEY_FILE_PATH}")

    if(NOT EXISTS "${ARG_KEY_FILE_PATH}")
        message(STATUS "Private key file does not exist. Generating a new ${ARG_KEY_SIZE}-bit RSA key...")

        # Ensure the output directory exists (optional, but recommended)
        get_filename_component(KEY_DIRECTORY "${ARG_KEY_FILE_PATH}" DIRECTORY)
        if(NOT EXISTS "${KEY_DIRECTORY}")
            message(STATUS "Creating key directory: ${KEY_DIRECTORY}")
            file(MAKE_DIRECTORY "${KEY_DIRECTORY}")
        endif()

        # Execute the openssl command
        execute_process(
                COMMAND ${LOCAL_OPENSSL_EXE} genrsa -3 -out "${ARG_KEY_FILE_PATH}" ${ARG_KEY_SIZE}
                RESULT_VARIABLE openssl_result # Store the command exit code
                ERROR_VARIABLE openssl_error   # Store the error output
                OUTPUT_QUIET                   # Do not show standard output
                ERROR_QUIET                    # Do not show standard error (unless command fails)
        )

        # Check if the command executed successfully
        if(NOT openssl_result EQUAL 0)
            # If failed, display error message and stop configuration
            message(FATAL_ERROR "ssgx_ensure_rsa_key_exists: Error - Failed to generate private key '${ARG_KEY_FILE_PATH}'.\n"
                    "OpenSSL command: ${LOCAL_OPENSSL_EXE} genrsa -out \"${ARG_KEY_FILE_PATH}\" ${ARG_KEY_SIZE}\n"
                    "Exit code: ${openssl_result}\n"
                    "Error output: ${openssl_error}")
        else()
            message(STATUS "Successfully generated private key: ${ARG_KEY_FILE_PATH}")
        endif()
    else()
        message(STATUS "Found existing private key file: ${ARG_KEY_FILE_PATH}")
    endif()

endfunction()
