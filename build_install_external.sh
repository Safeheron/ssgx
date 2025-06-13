#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

# Set the base directory for the external scripts
BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/external"

# Log the installation process
log() {
    echo "[INFO] $1"
}

# Check and execute scripts in a given directory
execute_script() {
    local script_path=$1
    if [ -f "$script_path" ]; then
        log "Executing $script_path..."
        chmod +x "$script_path"
        "$script_path"
        log "Completed: $script_path"
    else
        log "Script $script_path not found. Skipping."
    fi
}

script_proto="${BASE_DIR}/protobuf/build_and_install.sh"
log "Processing dependency: protobuf"
execute_script "$script_proto"

# List of external dependencies with their respective install scripts
declare -A EXTERNAL_DEPENDENCIES=(
    ["log4cplus"]="install.sh"
    ["mbedtls"]="build_and_install_mbedtls_SGX.sh"
    ["mpdecimal"]="build_and_install.sh"
    ["nlohmann"]="apply_patch_and_install.sh"
    ["safeheron-crypto-suites-cpp"]="build_and_install.sh" # Optional, adjust if a script exists
    ["toml11"]="install.sh"
    ["poco"]="install.sh"
)

# Iterate through each dependency and call its script
for dependency in "${!EXTERNAL_DEPENDENCIES[@]}"; do
    script="${BASE_DIR}/${dependency}/${EXTERNAL_DEPENDENCIES[$dependency]}"
    log "Processing dependency: $dependency"
    execute_script "$script"
done

log "All dependencies have been processed successfully."
