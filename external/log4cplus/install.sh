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

# Define variables
# The directory where this script is located (i.e., external/log4plus)
SCRIPT_DIR="$(cd "$(dirname "$(realpath "$0")")" && pwd)"
# The root directory for build outputs, relative to the script's location (i.e., external/build_tree/log4plus)
BUILD_ROOT_DIR="$(dirname "$SCRIPT_DIR")/build_tree/log4plus"

# Create the root build directory if it doesn't already exist
echo "Ensuring build root directory exists at: ${BUILD_ROOT_DIR}"
mkdir -p "$BUILD_ROOT_DIR"

REPO_URL="https://github.com/log4cplus/log4cplus.git"
# Source code will be cloned into a subdirectory of the build root
REPO_DIR="${BUILD_ROOT_DIR}/log4cplus"
VERSION="REL_2_0_8"
# Build artifacts will be generated in a separate 'build' subdirectory
BUILD_DIR="${BUILD_ROOT_DIR}/build"

# Clone the repository if it doesn't already exist
if [ ! -d "$REPO_DIR" ]; then
    echo "Cloning log4cplus repository into ${REPO_DIR} from ${REPO_URL}"
    git clone "$REPO_URL" "$REPO_DIR" || { echo "Failed to clone repository"; exit 1; }
else
    echo "Repository already cloned. Pulling latest changes..."
    cd "$REPO_DIR" || exit 1
    git fetch origin || { echo "Failed to fetch latest changes"; exit 1; }
fi

# Navigate to the repository
cd "$REPO_DIR" || exit 1

# Check out the specific branch or tag
echo "Checking out version $VERSION..."
git checkout "$VERSION" || { echo "Failed to checkout version $VERSION"; exit 1; }

# Update and initialize submodules
echo "Initializing and updating submodules..."
git submodule update --init --recursive || { echo "Failed to update submodules"; exit 1; }

# Create the build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating the build directory at ${BUILD_DIR}..."
    mkdir -p "$BUILD_DIR" || { echo "Failed to create build directory"; exit 1; }
fi

# Navigate to the build directory
cd "$BUILD_DIR" || exit 1

# Configure, Build, and Install (CORRECTED SECTION) ---
CMAKE_ARGS=""

# Use the 'untrusted_install_prefix' variable parsed from command-line arguments.
if [ -n "$untrusted_install_prefix" ]; then
    echo ">>> Custom installation prefix for untrusted library detected: $untrusted_install_prefix"
    CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=$untrusted_install_prefix"
fi

# Configure the project with CMake
# The source directory is now explicitly passed as REPO_DIR
echo "Configuring log4cplus with CMake..."
cmake "${REPO_DIR}" ${CMAKE_ARGS} || { echo "CMake configuration failed"; exit 1; }

# Build and install
echo "Building and installing log4cplus..."
sudo make install || { echo "Build and installation failed"; exit 1; }

echo "log4cplus installation completed successfully."

# Return to the original script directory for good practice
cd "$SCRIPT_DIR" || exit 1