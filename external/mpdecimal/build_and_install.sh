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

# --- MODIFIED SECTION START ---

# Define variables based on the new directory structure
# The directory where this script is located (i.e., external/mpdecimal)
SCRIPT_DIR="$(cd "$(dirname "$(realpath "$0")")" && pwd)"
# All intermediate products go into external/build_tree/mpdecimal
BUILD_ROOT_DIR="$(dirname "$SCRIPT_DIR")/build_tree/mpdecimal"

# Create the root build directory if it doesn't already exist
echo "Ensuring build root directory exists at: ${BUILD_ROOT_DIR}"
mkdir -p "$BUILD_ROOT_DIR"

# Source code will be cloned into a subdirectory of the build root
REPO_DIR="${BUILD_ROOT_DIR}/mpdecimal"
# Build artifacts will be generated in a separate 'build' subdirectory
BUILD_DIR="${BUILD_ROOT_DIR}/build"
# Patch file path relative to the script's location
PATCH_FILE="${SCRIPT_DIR}/sgx_mpdecimal.patch"

# --- MODIFIED SECTION END ---

# Configuration variables
REPO_URL="https://github.com/Safeheron/mpdecimal.git"
INSTALL_PREFIX="${trusted_install_prefix:-/opt/safeheron/ssgx}"
TARGET_TAG="78956e91f32d21843fb77be50ba060c196c718af"  # mpdecimal v4.0.0

# Clone the repository if it doesn't exist, otherwise fetch latest changes.
if [ ! -d "${REPO_DIR}" ]; then
    echo "Cloning the mpdecimal repository into '${REPO_DIR}'..."
    git clone "${REPO_URL}" "${REPO_DIR}"
else
    echo "Repository directory already exists. Fetching latest changes..."
    cd "${REPO_DIR}"
    git fetch origin
fi

# Navigate to the repository directory
cd "${REPO_DIR}"

# Checkout the specified tag
echo "Checking out tag: ${TARGET_TAG}..."
git checkout "${TARGET_TAG}"

# IMPORTANT: Reset to a clean state before applying the patch.
# This makes the script idempotent (safe to run multiple times).
echo "Resetting repository to a clean state..."
git reset --hard HEAD

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

# Run CMake configuration for an out-of-source build
echo "Configuring the project with CMake..."
cmake "${REPO_DIR}" -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}"

# Build the project
echo "Building the project..."
make -j$(nproc)

# Install the project
echo "Installing the project to ${INSTALL_PREFIX}..."
sudo make install

# Done
echo "mpdecimal installed successfully to ${INSTALL_PREFIX}."

# Return to the original script directory for good practice
cd "$SCRIPT_DIR" || exit 1