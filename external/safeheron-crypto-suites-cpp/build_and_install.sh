#!/bin/bash

# Exit immediately if a command fails
set -e

# Parameter Parsing for this Sub-script ---
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

# Define global variables
TOP_DIR="$(cd "$(dirname "$(realpath "$0")")" && pwd)"
REPO_URL="https://github.com/Safeheron/safeheron-crypto-suites-cpp.git"
REPO_DIR="${TOP_DIR}/safeheron-crypto-suites-cpp"
SGX_VERSION_TAG="291613e138cbb9cb8e7b646ee4767d123a5198b9"   # SGX-specific version(sgx_dev branch)
UNTRUSTED_VERSION_TAG="d193dd7ba29e02a8f4705b9698bbb49f0939016a" # Untrusted version (main branch)

# Install locations
SGX_INSTALL_PREFIX="/opt/safeheron/ssgx"   # default
UNTRUSTED_INSTALL_PREFIX="/usr/local"      # default

if [ -n "$trusted_install_prefix" ]; then
    SGX_INSTALL_PREFIX="$trusted_install_prefix"
    echo "Overriding SGX Install Prefix with: $SGX_INSTALL_PREFIX"
fi
if [ -n "$untrusted_install_prefix" ]; then
    UNTRUSTED_INSTALL_PREFIX="$untrusted_install_prefix"
    echo "Overriding Untrusted Install Prefix with: $UNTRUSTED_INSTALL_PREFIX"
fi

# Build directories
SGX_BUILD_DIR="${REPO_DIR}/sgx_build"
UNTRUSTED_BUILD_DIR="${REPO_DIR}/untrusted_build"

# Function to install a specific version of Safeheron Crypto Suites
install_crypto_suites() {
    local VERSION_TAG=$1  # Commit hash to install
    local BUILD_DIR=$2     # Compilation directory
    local INSTALL_PREFIX=$3 # Installation directory
    local BUILD_TYPE=$4     # SGX or UNTRUSTED

    echo "==============================="
    echo "🚀 Installing Safeheron Crypto Suites ($BUILD_TYPE)"
    echo "==============================="
    echo "🔹 Version: ${VERSION_TAG}"
    echo "🔹 Build Directory: ${BUILD_DIR}"
    echo "🔹 Install Directory: ${INSTALL_PREFIX}"
    echo "==============================="

    # Clone the repository if not already present
    if [ ! -d "$REPO_DIR" ]; then
        echo "🔹 Cloning Safeheron Crypto Suites repository..."
        git clone "$REPO_URL" "$REPO_DIR" || { echo "❌ Failed to clone repository"; exit 1; }
    else
        echo "🔹 Repository exists. Fetching latest updates..."
        cd "$REPO_DIR"
        git fetch origin || { echo "❌ Failed to fetch latest changes"; exit 1; }
    fi

    # Navigate to the repository
    cd "$REPO_DIR"

    # Checkout the specified version
    echo "🔹 Checking out version ${VERSION_TAG}..."
    git checkout "${VERSION_TAG}" || { echo "❌ Failed to checkout ${VERSION_TAG}"; exit 1; }

    # Create build directory if it doesn't exist
    if [ ! -d "$BUILD_DIR" ]; then
        echo "🔹 Creating build directory: ${BUILD_DIR}..."
        mkdir -p "$BUILD_DIR"
    fi
    cd "$BUILD_DIR"

    # Configure the project with CMake
    echo "🔹 Configuring Safeheron Crypto Suites with CMake..."
    cmake .. -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" || { echo "❌ CMake configuration failed"; exit 1; }

    # Build and install
    echo "🔹 Building and installing Safeheron Crypto Suites..."
    sudo make install || { echo "❌ Build and installation failed"; exit 1; }

    echo "✅ Safeheron Crypto Suites ($BUILD_TYPE) installed successfully in ${INSTALL_PREFIX}!"
}

# Install the SGX-specific version
install_crypto_suites "${SGX_VERSION_TAG}" "${SGX_BUILD_DIR}" "${SGX_INSTALL_PREFIX}" "SGX"

# Install the untrusted version
install_crypto_suites "${UNTRUSTED_VERSION_TAG}" "${UNTRUSTED_BUILD_DIR}" "${UNTRUSTED_INSTALL_PREFIX}" "UNTRUSTED"

echo "✅ Both SGX and UNTRUSTED versions of Safeheron Crypto Suites have been installed successfully!"
