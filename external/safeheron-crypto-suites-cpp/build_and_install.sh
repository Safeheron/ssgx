#!/bin/bash

# Exit immediately if a command fails
set -e

# --- 1. Parameter Parsing ---
# Initialize local variables to store parsed values.
trusted_install_prefix=""
untrusted_install_prefix=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        --trusted-install-prefix)
            if [[ -n "$2" && "$2" != --* ]]; then
                trusted_install_prefix="$2"
                shift 2
            else
                echo "Error: --trusted-install-prefix requires a non-empty value." >&2
                exit 1
            fi
            ;;
        --untrusted-install-prefix)
            if [[ -n "$2" && "$2" != --* ]]; then
                untrusted_install_prefix="$2"
                shift 2
            else
                echo "Error: --untrusted-install-prefix requires a non-empty value." >&2
                exit 1
            fi
            ;;
        *)
            shift
            ;;
    esac
done

# --- 2. Configuration and Directory Setup ---
# Standardized directory structure
SCRIPT_DIR="$(cd "$(dirname "$(realpath "$0")")" && pwd)"
BUILD_ROOT_DIR="$(dirname "$SCRIPT_DIR")/build_tree/safeheron-crypto-suites-cpp"

# Create the root build directory if it doesn't already exist
echo "[INFO] Ensuring build root directory exists at: ${BUILD_ROOT_DIR}"
mkdir -p "$BUILD_ROOT_DIR"

# Source code and build directories
REPO_DIR="${BUILD_ROOT_DIR}/safeheron-crypto-suites-cpp"
SGX_BUILD_DIR="${BUILD_ROOT_DIR}/build_sgx"
UNTRUSTED_BUILD_DIR="${BUILD_ROOT_DIR}/build_untrusted"

# Repository and version tags
REPO_URL="https://github.com/Safeheron/safeheron-crypto-suites-cpp.git"
VERSION_TAG="36f0a0ff352e9f23080bf1de448581deb3d61485" # main branch

# Set install locations with defaults
SGX_INSTALL_PREFIX="${trusted_install_prefix:-/opt/safeheron/ssgx}"
UNTRUSTED_INSTALL_PREFIX="${untrusted_install_prefix:-/usr/local}"

# --- 3. Refactored Installation Function ---
# This function now assumes the repository is already cloned and available.
install_crypto_suites() {
    local VERSION_TAG=$1
    local BUILD_DIR=$2
    local INSTALL_PREFIX=$3
    local BUILD_TYPE=$4

    echo "--------------------------------------------------------"
    echo "[INFO] Starting build for Safeheron Crypto Suites ($BUILD_TYPE)"
    echo "[INFO] Version: ${VERSION_TAG}"
    echo "[INFO] Build Directory: ${BUILD_DIR}"
    echo "[INFO] Install Directory: ${INSTALL_PREFIX}"
    echo "--------------------------------------------------------"

    # In the repo directory, checkout the specified version
    cd "${REPO_DIR}"
    echo "[STEP] Checking out version ${VERSION_TAG}..."
    git checkout "${VERSION_TAG}" || { echo "[ERROR] Failed to checkout ${VERSION_TAG}"; exit 1; }

    # Create the dedicated out-of-source build directory
    echo "[STEP] Ensuring build directory exists: ${BUILD_DIR}..."
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    # Configure the project with CMake, pointing to the source directory
    echo "[STEP] Configuring with CMake..."
    if [[ "$BUILD_TYPE" == "SGX" ]]; then
        cmake "${REPO_DIR}" -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" -DPLATFORM=SGX || {
            echo "[ERROR] CMake configuration failed for SGX"
            exit 1
        }
    else
        cmake "${REPO_DIR}" -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" || {
            echo "[ERROR] CMake configuration failed"
            exit 1
        }
    fi

    # Build and install
    echo "[STEP] Building and installing..."
    sudo make install || { echo "[ERROR] Build and installation failed"; exit 1; }

    echo "[SUCCESS] Safeheron Crypto Suites ($BUILD_TYPE) installed successfully!"
}

# --- 4. Main Execution Logic ---
# Clone or update the repository ONCE at the beginning.
if [ ! -d "$REPO_DIR" ]; then
    echo "[INFO] Cloning Crypto Suites repository into '${REPO_DIR}' from '${REPO_URL}'"
    git clone "$REPO_URL" "$REPO_DIR" || { echo "[ERROR] Failed to clone repository"; exit 1; }
else
    echo "[INFO] Repository exists. Fetching latest updates..."
    cd "$REPO_DIR"
    git fetch origin || { echo "[ERROR] Failed to fetch latest changes"; exit 1; }
fi

# Install the SGX-specific version
install_crypto_suites "${VERSION_TAG}" "${SGX_BUILD_DIR}" "${SGX_INSTALL_PREFIX}" "SGX"

# Install the untrusted version
install_crypto_suites "${VERSION_TAG}" "${UNTRUSTED_BUILD_DIR}" "${UNTRUSTED_INSTALL_PREFIX}" "UNTRUSTED"

echo "--------------------------------------------------------"
echo "[SUCCESS] All versions of Safeheron Crypto Suites have been installed."

# Return to the original script directory for good practice
cd "$SCRIPT_DIR" || exit 1