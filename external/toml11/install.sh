#!/bin/bash

# Exit immediately if a command fails
set -e

# --- 1. Parameter Parsing ---
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

# --- 2. Configuration and Directory Setup ---
# Standardized directory structure
SCRIPT_DIR="$(cd "$(dirname "$(realpath "$0")")" && pwd)"
BUILD_ROOT_DIR="$(dirname "$SCRIPT_DIR")/build_tree/toml11"

# Create the root build directory if it doesn't already exist
echo "[INFO] Ensuring build root directory exists at: ${BUILD_ROOT_DIR}"
mkdir -p "$BUILD_ROOT_DIR"

# Source code and build directories
TOML11_URL="https://github.com/ToruNiina/toml11/archive/refs/tags/v3.8.1.zip"
TOML11_ZIP="${BUILD_ROOT_DIR}/toml11-v3.8.1.zip"
TOML11_DIR="${BUILD_ROOT_DIR}/toml11-3.8.1" # Directory created after unzipping
BUILD_DIR="${BUILD_ROOT_DIR}/build"

# Define expected checksum for the zip file
EXPECTED_CHKSUM="72e956f42002dd1566c5551a693ec0f6fa3bea3a0e7bcea29bcdace98738da74"

# Set install location with a default
INSTALL_PREFIX="${untrusted_install_prefix:-/usr/local}"

# --- 3. Download and Verify ---
# Download the toml11 archive if it doesn't already exist
if [ ! -f "$TOML11_ZIP" ]; then
    echo "[STEP] Downloading toml11 v3.8.1..."
    wget -O "$TOML11_ZIP" "$TOML11_URL" || { echo "[ERROR] Failed to download toml11"; exit 1; }
fi

# Verify the downloaded file's integrity using a more robust method
echo "[STEP] Verifying checksum of the downloaded file..."
# Create checksum line and pipe to sha256sum for verification
echo "${EXPECTED_CHKSUM}  ${TOML11_ZIP}" | sha256sum --check --status
if [ $? -ne 0 ]; then
    echo "[ERROR] Checksum verification failed. The downloaded file might be corrupt or changed."
    # rm -f "$TOML11_ZIP" # Optional: remove the bad file
    exit 1
fi
echo "[SUCCESS] Checksum verified."

# --- 4. Extract ---
# Extract the toml11 archive if the destination directory doesn't exist
if [ ! -d "$TOML11_DIR" ]; then
    echo "[STEP] Extracting toml11 source files..."
    # Extract directly into the build root directory
    unzip -qq "$TOML11_ZIP" -d "$BUILD_ROOT_DIR" || { echo "[ERROR] Failed to extract toml11"; exit 1; }
fi

# --- 5. Build and Install ---
# Create the build directory
echo "[STEP] Ensuring build directory exists..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure, Build, and Install
CMAKE_ARGS="-DCMAKE_CXX_STANDARD=11 -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX}"

echo "[STEP] Configuring and building toml11..."
# Point cmake to the source directory explicitly
cmake "$TOML11_DIR" ${CMAKE_ARGS} || { echo "[ERROR] CMake configuration failed"; exit 1; }

echo "[STEP] Installing toml11 to ${INSTALL_PREFIX}..."
sudo make install || { echo "[ERROR] Build and installation failed"; exit 1; }

echo "[SUCCESS] toml11 installation completed successfully."

# Return to the original script directory for good practice
cd "$SCRIPT_DIR" || exit 1