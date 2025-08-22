#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

# Parameter Parsing for this Sub-script ---
# Initialize local variables to store parsed values.
untrusted_install_prefix=""

while [[ $# -gt 0 ]]; do
    case "$1" in
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

# --- MODIFIED SECTION START ---

# Define variables based on the new directory structure
# The directory where this script is located (i.e., external/protobuf)
SCRIPT_DIR="$(cd "$(dirname "$(realpath "$0")")" && pwd)"
# All intermediate products go into external/build_tree/protobuf
BUILD_ROOT_DIR="$(dirname "$SCRIPT_DIR")/build_tree/protobuf"

# Create the root build directory if it doesn't already exist
echo "Ensuring build root directory exists at: ${BUILD_ROOT_DIR}"
mkdir -p "$BUILD_ROOT_DIR"

# Source code will be cloned into a subdirectory of the build root
REPO_DIR="${BUILD_ROOT_DIR}/protobuf"
# Build artifacts will be generated in a separate 'build' subdirectory
BUILD_DIR="${BUILD_ROOT_DIR}/build"

# --- MODIFIED SECTION END ---

# Configuration variables
REPO_URL="https://github.com/protocolbuffers/protobuf.git"
VERSION="v3.20.0"
# Set default install prefix if not provided
INSTALL_PREFIX="${untrusted_install_prefix:-/usr/local}"


# Clone the repository if it doesn't exist, otherwise fetch latest changes.
if [ ! -d "${REPO_DIR}" ]; then
    echo "Cloning protobuf repository into '${REPO_DIR}' from '${REPO_URL}'"
    git clone "${REPO_URL}" "${REPO_DIR}"
else
    echo "Repository directory already exists. Fetching latest changes..."
    cd "${REPO_DIR}"
    git fetch origin
fi

# Navigate to the repository directory
cd "${REPO_DIR}"

# Check out the specific branch or tag
echo "Checking out version $VERSION..."
git checkout "$VERSION"

# Update and initialize submodules
echo "Initializing and updating submodules..."
git submodule update --init --recursive

# Create the build directory
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Configure the project with CMake for an out-of-source build
# Note that protobuf's CMakeLists.txt is in a 'cmake' subdirectory
echo "Configuring protobuf with CMake..."
cmake "${REPO_DIR}/cmake" \
  -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" \
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
	-DCMAKE_BUILD_TYPE=Release \
  -Dprotobuf_BUILD_SHARED_LIBS=OFF \
  -Dprotobuf_BUILD_TESTS=OFF \
  -Dprotobuf_BUILD_EXAMPLES=OFF \
  -Dprotobuf_WITH_ZLIB=OFF \
  -Dprotobuf_BUILD_PROTOC_BINARIES=ON

# Build the project
echo "Building protobuf..."
make -j$(nproc)

# Install the project
echo "Installing protobuf to: ${INSTALL_PREFIX}"
sudo make install

echo "Protobuf installation completed successfully."

# Return to the original script directory for good practice
cd "$SCRIPT_DIR" || exit 1