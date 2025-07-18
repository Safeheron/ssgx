# ============================================================================================
# SSGX API Interface Description File
# This file provides user-visible functions and global variable definitions
# for SGX SDK/SSL configuration, mode selection, and environment retrieval.
# ============================================================================================

include_guard(GLOBAL)

include("${CMAKE_CURRENT_LIST_DIR}/internal/ssgx-init.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/internal/ssgx-env.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/internal/ssgx-edl.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/internal/ssgx-sign.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/internal/ssgx-trusted.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/internal/ssgx-untrusted.cmake")

# ============================================================================================
# Global Variable Definitions
# ============================================================================================
# SDK related:
#   - SSGX_ENV__SGXSDK
#   - SSGX_ENV__SGX_COMMON_CFLAGS
#   - SSGX_ENV__SGX_ENCLAVE_SIGNER
#   - SSGX_ENV__SGX_EDGER8R
#   - SSGX_ENV__SGXSDK_LIBRARY_DIR
#   - SSGX_ENV__SGXSDK_INCLUDE_DIR
#   - SSGX_ENV__SGXSDK_INCLUDE_DIRS
#   - SSGX_ENV__EDL_SEARCH_PATHS
# SSL related:
#   - SSGX_ENV__SGXSSL
#   - SSGX_ENV__SGXSSL_INCLUDE_DIR
#   - SSGX_ENV__SGXSSL_LIBRARY_DIR
# User-configurable:
#   - SSGX_ENV__BUILD_MODE: (Debug | PreRelease | Release)
#   - SSGX_ENV__HARDWARE_MODE: (ON | OFF)
# Entry path:
#   - SSGX_ENV__CMAKE_ENTRY_PATH

# ============================================================================================
# Functions for configuration and environment retrieval
# ============================================================================================
# Function: ssgx_set_build_mode(mode)
#   - Sets build mode (Debug, PreRelease, Release)
#
# Function: ssgx_set_hardware_mode(enabled)
#   - Enables/disables SGX hardware mode

# ============================================================================================
# Functions for SGX Target Build
# ============================================================================================
# Function: ssgx_add_trusted_library(target [SRCS ...] [EDL <file>] [EDL_SEARCH_PATHS ...] [TRUSTED_LIBS ...] [USE_PREFIX] [USE_SGXSSL] [LDSCRIPT <file>])
#   - Adds a static library for trusted enclave code
#   - Processes EDL headers and dependencies
#
# Function: ssgx_add_enclave_library(target [SRCS ...] [EDL <file>] [EDL_SEARCH_PATHS ...] [TRUSTED_LIBS ...] [USE_PREFIX] [USE_SGXSSL] [LDSCRIPT <file>])
#   - Adds a shared enclave library
#   - Generates EDL source and links all trusted dependencies
#
# Function: ssgx_add_untrusted_library(target mode [SRCS ...] [EDL <file>] [EDL_SEARCH_PATHS ...] [UNTRUSTED_LIBS ...] [USE_PREFIX])
#   - Adds an untrusted host-side library linked to enclave interface
#   - Processes EDL to generate stubs
#
# Function: ssgx_add_untrusted_executable(target [SRCS ...] [EDL <file>] [EDL_SEARCH_PATHS ...] [UNTRUSTED_LIBS ...] [USE_PREFIX])
#   - Builds an untrusted executable that communicates with enclave
#   - Generates and compiles EDL stubs

# ============================================================================================
# Functions for Signing a SGX Enclave
# ============================================================================================
# Function: ssgx_sign_enclave(target [KEY <key.pem>] [CONFIG <enclave.config.xml>] [OUTPUT <name>] [IGNORE_INIT] [IGNORE_REL])
#   - Signs enclave using sgx_sign tool with one-step or two-step mode
#   - Creates a ${target}-signed custom target

# NOTE: These variables/functions are defined in:
#   - cmake/internal/ssgx-env.cmake
#   - cmake/internal/ssgx-edl.cmake
#   - cmake/internal/ssgx-build.cmake
#   - cmake/internal/ssgx-sign.cmake
# This file serves as the unified user-facing interface declaration.
