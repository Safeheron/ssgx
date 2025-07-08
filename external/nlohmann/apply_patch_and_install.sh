#!/bin/bash

# Exit immediately if a command exits with a non-zero status
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

# Configuration variables
TOP_DIR="$(cd "$(dirname "$(realpath "${0}")")" && pwd)"
NLOHMANN_REPO_URL="https://github.com/Safeheron/json.git"  # URL of the nlohmann/json repository
NLOHMANN_VERSION="v3.11.2"  # Target tag to apply the patch
PATCH_FILE="${TOP_DIR}/sgx_json.patch"  # Path to the patch file
CLONE_DIR="${TOP_DIR}/nlohmann_json"  # Absolute path for the cloned repository
INSTALL_PREFIX="${trusted_install_prefix:-/opt/safeheron/ssgx}"  # Target installation directory, default to /opt/safeheron/ssgx
INSTALL_DIR="${INSTALL_PREFIX}/include"  # Target installation directory

# Clone the nlohmann/json repository
if [ -d "${CLONE_DIR}" ]; then
    echo "Repository directory already exists."
    echo "Removing directory '${CLONE_DIR}'..."
    rm -rf "${CLONE_DIR}"
fi
echo "Cloning the nlohmann/json repository..."
git clone "${NLOHMANN_REPO_URL}" "${CLONE_DIR}"

# Navigate to the repository directory
cd "${CLONE_DIR}"

# Checkout the specific tag
echo "Checking out version ${NLOHMANN_VERSION}..."
git checkout "${NLOHMANN_VERSION}"

# Install the unpatched library to the system default directory
UNTRUSTED_INSTALL_DIR="${untrusted_install_prefix:-/usr/local}/include"  # Target installation directory, default to /opt/safeheron/ssgx

echo "Installing unpatched library headers to ${UNTRUSTED_INSTALL_DIR}..."
sudo mkdir -p "${UNTRUSTED_INSTALL_DIR}"
sudo cp -r "${CLONE_DIR}/include/nlohmann" "${UNTRUSTED_INSTALL_DIR}/"

# Apply the patch file
if [ -f "${PATCH_FILE}" ]; then
    echo "Applying patch: ${PATCH_FILE}..."
    git apply "${PATCH_FILE}"
else
    echo "Error: Patch file '${PATCH_FILE}' not found."
    exit 1
fi

# Check if the target directory exists, create it if not
if [ ! -d "${INSTALL_DIR}" ]; then
    echo "Creating the installation directory: ${INSTALL_DIR}..."
    sudo mkdir -p "${INSTALL_DIR}"
fi

# Install header files to the target directory
echo "Installing header files to ${INSTALL_DIR}..."
sudo cp -r "${CLONE_DIR}/include/nlohmann" "${INSTALL_DIR}/"

# Done
echo "nlohmann/json headers installed successfully to ${INSTALL_DIR}."
