#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

# Parameter Parsing for this Sub-script ---
# Initialize local variables to store parsed values.
trusted_install_prefix=""

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
        *)
            shift
            ;;
    esac
done

# Configuration variables
TOP_DIR="$(cd "$(dirname "$(realpath "${0}")")" && pwd)"
REPO_URL="https://github.com/Safeheron/mbedtls-SGX.git"  # URL of the mbedtls-SGX repository
CLONE_DIR="${TOP_DIR}/mbedtls-SGX"  # Absolute path for cloning the repository
BUILD_DIR="${CLONE_DIR}/build"  # Absolute path for the build directory
INSTALL_PREFIX="${trusted_install_prefix:-/opt/safeheron/ssgx}"  # Target installation directory, default to /opt/safeheron/ssgx
TARGET_TAG="f6901462b8cf809b2e17bcd9688a042d0f7d3b0d"  # commit in branch 'master', tag 'v3.6.3_sgx_2'

# Clone the mbedtls-SGX repository
if [ -d "${CLONE_DIR}" ]; then
    echo "Repository directory already exists."
    echo "Removing directory '${CLONE_DIR}'..."
    rm -rf "${CLONE_DIR}"
fi
echo "Cloning the mbedtls-SGX repository into '${CLONE_DIR}'..."
git clone "${REPO_URL}" "${CLONE_DIR}"

# Navigate to the cloned directory
cd "${CLONE_DIR}"

# Checkout the specified tag
echo "Checking out tag '${TARGET_TAG}'..."
git checkout "${TARGET_TAG}"

# Create and navigate to the build directory
echo "Creating build directory at '${BUILD_DIR}'..."
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Run CMake configuration
echo "Configuring the project with CMake..."
cmake .. -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}"

# Build the project
echo "Building the project..."
make -j$(nproc)

# Install the project
echo "Installing the project to ${INSTALL_PREFIX}..."
sudo make install

# Done
echo "mbedtls-SGX installed successfully to ${INSTALL_PREFIX}."
