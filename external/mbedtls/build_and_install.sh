#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

# Configuration variables
TOP_DIR="$(cd "$(dirname "$(realpath "${0}")")" && pwd)"
REPO_URL="https://github.com/Safeheron/mbedtls-SGX.git"  # URL of the mbedtls-SGX repository
PATCH_FILE="${TOP_DIR}/sgx_mbedtls.patch"  # Path to the patch file (replace with the actual path)
CLONE_DIR="${TOP_DIR}/mbedtls-SGX"  # Absolute path for cloning the repository
BUILD_DIR="${CLONE_DIR}/build"  # Absolute path for the build directory
INSTALL_PREFIX="${INSTALL_PREFIX:-/opt/safeheron/ssgx}"  # Target installation directory, default to /opt/safeheron/ssgx
TARGET_TAG="eab8e36a1e670a2fa66105735143eafa51931bff"  # commit in branch 'main'

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

# Apply the patch file
if [ -f "${PATCH_FILE}" ]; then
    echo "Applying patch: ${PATCH_FILE}..."
    git apply "${PATCH_FILE}"
else
    echo "Error: Patch file '${PATCH_FILE}' not found."
    exit 1
fi

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
