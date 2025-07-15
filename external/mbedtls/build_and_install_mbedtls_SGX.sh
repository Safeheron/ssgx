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

# Define variables based on the new directory structure
# The directory where this script is located (i.e., external/mbedtls)
SCRIPT_DIR="$(cd "$(dirname "$(realpath "$0")")" && pwd)"
# All intermediate products go into external/build_tree/mbedtls
BUILD_ROOT_DIR="$(dirname "$SCRIPT_DIR")/build_tree/mbedtls"

# Create the root build directory if it doesn't already exist
echo "Ensuring build root directory exists at: ${BUILD_ROOT_DIR}"
mkdir -p "$BUILD_ROOT_DIR"

REPO_URL="https://github.com/Safeheron/mbedtls-SGX.git"
# Source code will be cloned into a subdirectory of the build root
REPO_DIR="${BUILD_ROOT_DIR}/mbedtls-SGX"
# Build artifacts will be generated in a separate 'build' subdirectory
BUILD_DIR="${BUILD_ROOT_DIR}/build"

# Configuration variables
INSTALL_PREFIX="${trusted_install_prefix:-/opt/safeheron/ssgx}"  # Target installation directory, default to /opt/safeheron/ssgx
TARGET_TAG="a2935a151d9abd904f433d845e6e43d745e3b531"  # commit in branch 'master', tag 'v3.6.3_sgx_2'


# --- MODIFIED SECTION: Clone or Update Repository ---

# If the repository directory doesn't exist, clone it.
if [ ! -d "${REPO_DIR}" ]; then
    echo "Cloning the mbedtls-SGX repository into '${REPO_DIR}'..."
    git clone "${REPO_URL}" "${REPO_DIR}"
# If it exists, fetch the latest changes instead of deleting.
else
    echo "Repository directory already exists. Fetching latest changes..."
    cd "${REPO_DIR}"
    git fetch origin
fi

# Navigate to the cloned directory
cd "${REPO_DIR}"

# Checkout the specified tag or commit
echo "Checking out tag/commit '${TARGET_TAG}'..."
git checkout "${TARGET_TAG}"


# --- END OF MODIFIED SECTION ---


# Create and navigate to the build directory
echo "Creating build directory at '${BUILD_DIR}'..."
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Run CMake configuration for an out-of-source build
echo "Configuring the project with CMake..."
# Point cmake to the source directory explicitly
cmake "${REPO_DIR}" -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}"

# Build the project
echo "Building the project..."
make -j$(nproc)

# Install the project
echo "Installing the project to ${INSTALL_PREFIX}..."
sudo make install

# Done
echo "mbedtls-SGX installed successfully to ${INSTALL_PREFIX}."

# Return to the original script directory for good practice
cd "$SCRIPT_DIR" || exit 1