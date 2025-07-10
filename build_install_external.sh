#!/bin/bash
#
# Main build orchestrator for external dependencies.
# This version exclusively uses command-line arguments for inter-script communication.
#

set -e

# --- 1. Parameter Parsing for the Main Script ---
# Internal variables remain lowercase.
trusted_install_prefix_val="/opt/safeheron/ssgx"
install_libs_together_val="false"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --trusted-install-prefix)
            if [[ -n "$2" && "$2" != --* ]]; then
                trusted_install_prefix_val="$2"
                shift 2
            else
                echo "Error: --trusted-install-prefix requires a non-empty value." >&2
                exit 1
            fi
            ;;
        --install_libs_together)
            if [[ -n "$2" && "$2" != --* ]]; then
                install_libs_together_val="$2"
                shift 2
            else
                echo "Error: --install_libs_together requires a non-empty value." >&2
                exit 1
            fi
            ;;
        *)
            shift
            ;;
    esac
done

# --- 2. Setup Variables and Functions ---
base_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/external"

log() {
    echo -e "\n[INFO] ========================================================="
    echo "[INFO] $1"
    echo -e "[INFO] ========================================================="
}

# MODIFIED execute_script function
# It now accepts a path and a list of arguments to pass along.
# $1: Path to the script to execute.
# $@: All subsequent arguments are passed directly to the sub-script.
execute_script() {
    local script_path=$1
    shift # Remove script_path, the rest are the arguments for the sub-script.
    local sub_script_args=("$@")

    if [ ! -f "$script_path" ]; then
        echo "[WARNING] Script '$script_path' not found. Skipping."
        return
    fi

    chmod +x "$script_path"

    # Pass the arguments to the sub-script. The quotes are crucial for handling spaces.
    echo "[CMD] \"$script_path\" \"${sub_script_args[@]}\""
    "$script_path" "${sub_script_args[@]}"
}

# --- 3. Prepare Command-Line Arguments for Sub-scripts ---
# We use an array to safely build the list of arguments.
sub_script_args=()

echo "--- Configuration Summary ---"
if [ -n "$trusted_install_prefix_val" ]; then
    echo "trusted Install Prefix: $trusted_install_prefix_val"
    # Add the argument and its value to the array.
    sub_script_args+=(--trusted-install-prefix "$trusted_install_prefix_val")

    if [[ "$install_libs_together_val" == "true" ]]; then
        untrusted_path="${trusted_install_prefix_val}/__untrusted_dependencies"
        echo "Install Untrusted Libs Together: YES"
        echo "Path for Untrusted Libs: $untrusted_path"
        # Add the second argument and its value to the array.
        sub_script_args+=(--untrusted-install-prefix "$untrusted_path")
    else
        echo "Install Untrusted Libs Together: NO"
    fi
else
    echo "No custom prefix provided. All libraries will be installed to their default system paths."
fi
echo "---------------------------"

# --- 4. Define Dependencies and Execute Installation ---
script_proto="${base_dir}/protobuf/build_and_install.sh"
log "Processing dependency: protobuf"
execute_script "$script_proto" "${sub_script_args[@]}"

declare -A external_dependencies=(
    ["log4cplus"]="install.sh"
    ["mbedtls"]="build_and_install_mbedtls_SGX.sh"
    ["mpdecimal"]="build_and_install.sh"
    ["nlohmann"]="apply_patch_and_install.sh"
    ["safeheron-crypto-suites-cpp"]="build_and_install.sh" # Optional, adjust if a script exists
    ["toml11"]="install.sh"
    ["poco"]="install.sh"
)

# Iterate and call each script, passing the constructed argument list.
for dependency in "${!external_dependencies[@]}"; do
    script_name="${external_dependencies[$dependency]}"
    script_path="${base_dir}/${dependency}/${script_name}"

    log "Processing dependency: $dependency"
    # The expansion "${sub_script_args[@]}" correctly passes all elements of the array as separate arguments.
    execute_script "$script_path" "${sub_script_args[@]}"
done

log "All dependencies have been processed successfully."