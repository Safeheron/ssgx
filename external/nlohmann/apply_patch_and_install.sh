#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

# --- 1. Parameter Parsing ---
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

# --- 2. Configuration and Directory Setup ---
# Standardized directory structure
SCRIPT_DIR="$(cd "$(dirname "$(realpath "$0")")" && pwd)"
BUILD_ROOT_DIR="$(dirname "$SCRIPT_DIR")/build_tree/nlohmann"

# Create the root build directory if it doesn't already exist
echo "[INFO] Ensuring build root directory exists at: ${BUILD_ROOT_DIR}"
mkdir -p "$BUILD_ROOT_DIR"

# Source code and build directories
REPO_DIR="${BUILD_ROOT_DIR}/nlohmann_json"
UNTRUSTED_BUILD_DIR="${BUILD_ROOT_DIR}/untrusted_build"
PATCH_FILE="${SCRIPT_DIR}/sgx_json.patch"

# Repository and version tags
NLOHMANN_REPO_URL="https://github.com/Safeheron/json.git"
NLOHMANN_VERSION="v3.11.2"

# Set install locations with defaults
TRUSTED_INSTALL_DIR="${trusted_install_prefix:-/opt/safeheron/ssgx}/include"
UNTRUSTED_INSTALL_DIR="${untrusted_install_prefix:-/usr/local}"

# --- 3. Git Clone/Fetch ---
# Clone the repository if it doesn't exist, otherwise fetch latest changes.
if [ ! -d "${REPO_DIR}" ]; then
    echo "[INFO] Cloning the nlohmann/json repository into '${REPO_DIR}'..."
    git clone "${NLOHMANN_REPO_URL}" "${REPO_DIR}"
else
    echo "[INFO] Repository directory already exists. Fetching latest changes..."
    cd "${REPO_DIR}"
    git fetch origin
fi

# Navigate to the repository directory
cd "${REPO_DIR}"

# --- 4. Install Unpatched (Untrusted) Version ---
echo "--------------------------------------------------------"
echo "[INFO] Starting install for UNTRUSTED nlohmann/json"
# Checkout the specific tag
echo "[STEP] Checking out version ${NLOHMANN_VERSION}..."
git checkout "${NLOHMANN_VERSION}"

# Reset to a clean state to ensure we are installing the original, unpatched version.
echo "[STEP] Resetting repository to a clean state before untrusted build..."
git reset --hard HEAD

echo "[STEP] Building and installing unpatched library to ${UNTRUSTED_INSTALL_DIR}..."
# Configure using an out-of-source build directory
cmake -S "${REPO_DIR}" -B "${UNTRUSTED_BUILD_DIR}" \
      -DCMAKE_INSTALL_PREFIX="${UNTRUSTED_INSTALL_DIR}" \
      -DJSON_BuildTests=OFF \
      -DJSON_BuildExamples=OFF

# Build and install the untrusted version
cmake --build "${UNTRUSTED_BUILD_DIR}"
sudo cmake --install "${UNTRUSTED_BUILD_DIR}"
echo "[SUCCESS] Untrusted version installed."

# --- 5. Apply Patch and Install Patched (Trusted) Version ---
echo "--------------------------------------------------------"
echo "[INFO] Starting install for TRUSTED nlohmann/json"
# Apply the patch file. The previous reset ensures this step is safe to re-run.
if [ ! -f "${PATCH_FILE}" ]; then
    echo "[ERROR] Patch file '${PATCH_FILE}' not found."
    exit 1
fi
echo "[STEP] Applying patch: ${PATCH_FILE}..."
git apply "${PATCH_FILE}"

# Install patched header files to the target directory using simple copy
echo "[STEP] Installing patched header files to ${TRUSTED_INSTALL_DIR}..."
sudo mkdir -p "${TRUSTED_INSTALL_DIR}"
sudo cp -r "${REPO_DIR}/include/nlohmann" "${TRUSTED_INSTALL_DIR}/"
echo "[SUCCESS] Trusted version installed."

# --- 6. Completion ---
echo "--------------------------------------------------------"
echo "[SUCCESS] nlohmann/json untrusted and trusted headers installed."

# Return to the original script directory for good practice
cd "$SCRIPT_DIR" || exit 1