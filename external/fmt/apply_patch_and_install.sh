#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

# --- 1. Parameter Parsing ---
# --untrusted-install-prefix is accepted but ignored — host-side code does not
# currently consume fmt, so no untrusted (unpatched) install is produced.
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
        --untrusted-install-prefix)
            # Accepted for caller-convention parity with sibling scripts; ignored.
            if [[ -n "$2" && "$2" != --* ]]; then
                shift 2
            else
                shift
            fi
            ;;
        *)
            shift
            ;;
    esac
done

# --- 2. Configuration and Directory Setup ---
SCRIPT_DIR="$(cd "$(dirname "$(realpath "$0")")" && pwd)"
BUILD_ROOT_DIR="$(dirname "$SCRIPT_DIR")/build_tree/fmt"
REPO_DIR="${BUILD_ROOT_DIR}/fmt"
PATCH_FILE="${SCRIPT_DIR}/sgx_fmt.patch"

# Upstream fmtlib repository and pinned version
FMT_REPO_URL="https://github.com/fmtlib/fmt.git"
FMT_VERSION="12.1.0"

TRUSTED_INSTALL_DIR="${trusted_install_prefix:-/opt/safeheron/ssgx}/include"

echo "[INFO] Ensuring build root directory exists at: ${BUILD_ROOT_DIR}"
mkdir -p "$BUILD_ROOT_DIR"

# --- 3. Git Clone / Fetch (idempotent) ---
if [ ! -d "${REPO_DIR}" ]; then
    echo "[INFO] Cloning fmt repository into '${REPO_DIR}' from '${FMT_REPO_URL}'"
    git clone "${FMT_REPO_URL}" "${REPO_DIR}"
else
    echo "[INFO] Repository directory already exists. Fetching latest tags..."
    cd "${REPO_DIR}"
    git fetch --tags origin
fi

cd "${REPO_DIR}"

# --- 4. Checkout and Reset (so re-runs are safe) ---
echo "[STEP] Checking out tag ${FMT_VERSION}..."
git checkout "${FMT_VERSION}"
echo "[STEP] Resetting repository to a clean state..."
git reset --hard HEAD

# --- 5. Apply SGX Patch ---
if [ ! -f "${PATCH_FILE}" ]; then
    echo "[ERROR] Patch file '${PATCH_FILE}' not found." >&2
    exit 1
fi
echo "[STEP] Applying patch: ${PATCH_FILE}..."
git apply "${PATCH_FILE}"

# --- 6. Install Patched Headers (trusted side only) ---
echo "[STEP] Installing patched fmt headers to ${TRUSTED_INSTALL_DIR}/fmt..."
sudo mkdir -p "${TRUSTED_INSTALL_DIR}"
sudo cp -r "${REPO_DIR}/include/fmt" "${TRUSTED_INSTALL_DIR}/"

# --- 7. Completion ---
echo "--------------------------------------------------------"
echo "[SUCCESS] fmt v${FMT_VERSION} (SGX-patched) trusted headers installed."
echo "          Headers at: ${TRUSTED_INSTALL_DIR}/fmt/"

cd "$SCRIPT_DIR" || exit 1
