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
REPO_URL="https://github.com/Safeheron/mpdecimal.git"  # URL of the mpdecimal repository
PATCH_FILE="${TOP_DIR}/sgx_mpdecimal.patch"  # Path to the patch file (replace with the actual path)
BUILD_DIR="${TOP_DIR}/build"  # Build directory
CLONE_DIR="${TOP_DIR}/mpdecimal"  # Absolute path for the cloned repository
INSTALL_PREFIX="${trusted_install_prefix:-/opt/safeheron/ssgx}"  # Target installation directory, default to /opt/safeheron/ssgx
TARGET_TAG="78956e91f32d21843fb77be50ba060c196c718af"  # mpdecimal v4.0.0

# Clone the mpdecimal repository
if [ -d "${CLONE_DIR}" ]; then
    echo "Repository directory already exists."
    echo "Removing directory '${CLONE_DIR}'..."
    rm -rf "${CLONE_DIR}"
fi
echo "Cloning the mpdecimal repository..."
git clone "${REPO_URL}" "${CLONE_DIR}"

# Navigate to the repository directory
cd "${CLONE_DIR}"

# Checkout the specified tag
echo "Checking out tag: ${TARGET_TAG}..."
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
cmake "${CLONE_DIR}" -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}"

# Build the project
echo "Building the project..."
make -j$(nproc)

# Install the project
echo "Installing the project to ${INSTALL_PREFIX}..."
sudo make install

# Done
echo "mpdecimal installed successfully to ${INSTALL_PREFIX}."
