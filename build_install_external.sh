#!/bin/bash
set -e

# --- 0. Usage/Help Function ---
usage() {
    echo "Usage: $0 [OPTIONS]"
    echo "Options:"
    echo "  --trusted-install-prefix <path>   Set the installation prefix for trusted libraries (default: /opt/safeheron/ssgx)."
    echo "  --install_libs_together <true|false>  Install untrusted libs into a sub-directory of the trusted prefix (default: false)."
    echo "  --only <name>               Build and install only a single dependency. Available dependencies are:"
    # Dynamically list available dependencies below
    echo "                                      - protobuf"
    echo "                                      - log4cplus"
    echo "                                      - mbedtls"
    echo "                                      - mpdecimal"
    echo "                                      - nlohmann"
    echo "                                      - safeheron-crypto-suites-cpp"
    echo "                                      - toml11"
    echo "                                      - poco"
    echo "  -h, --help                        Display this help message and exit."
    exit 1
}


# --- 1. All dependencies ---
declare -a dependency_list=(
    "protobuf"
    "log4cplus"
    "mbedtls"
    "mpdecimal"
    "nlohmann"
    "safeheron-crypto-suites-cpp"
    "toml11"
    "poco"
)

declare -A external_dependencies=(
    ["protobuf"]="build_and_install.sh"
    ["log4cplus"]="install.sh"
    ["mbedtls"]="build_and_install_mbedtls_SGX.sh"
    ["mpdecimal"]="build_and_install.sh"
    ["nlohmann"]="apply_patch_and_install.sh"
    ["safeheron-crypto-suites-cpp"]="build_and_install.sh"
    ["toml11"]="install.sh"
    ["poco"]="install.sh"
)



# --- 2. Parse arguments ---
trusted_install_prefix_val="/opt/safeheron/ssgx"
install_libs_together_val="false"
only_dependency=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        --trusted-install-prefix)
            if [[ -n "$2" && "$2" != --* ]]; then
                trusted_install_prefix_val="$2"
                shift 2
            else
                echo "Error: --trusted-install-prefix requires a non-empty value." >&2
                usage
                exit 1
            fi
            ;;
        --install_libs_together)
            if [[ -n "$2" && "$2" != --* ]]; then
                install_libs_together_val="$2"
                if [[ "$install_libs_together_val" != "true" && "$install_libs_together_val" != "false" ]]; then
                    echo "Error: --install_libs_together must be 'true' or 'false'." >&2
                    usage
                    exit 1
                fi
                shift 2
            else
                echo "Error: --install_libs_together requires a non-empty value." >&2
                usage
                exit 1
            fi
            ;;
        --only)
            if [[ -n "$2" && "$2" != --* ]]; then
                only_dependency="$2"

                # Check validity
                found="false"
                for valid in "${dependency_list[@]}"; do
                    if [[ "$only_dependency" == "$valid" ]]; then
                        found="true"
                        break
                    fi
                done

                if [[ "$found" != "true" ]]; then
                    echo "Error: --only argument '$only_dependency' is not a valid dependency name." >&2
                    echo "Valid options are: ${dependency_list[*]}" >&2
                    usage
                    exit 1
                fi

                shift 2
            else
                echo "Error: --only requires a non-empty value." >&2
                usage
                exit 1
            fi
            ;;
        *)
            echo "Error: Unknown argument '$1'" >&2
            usage
            exit 1
            ;;
    esac
done

# --- 3. Setup ---
base_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/external"

log() {
    echo -e "\n[INFO] ========================================================="
    echo "[INFO] $1"
    echo -e "[INFO] ========================================================="
}

execute_script() {
    local script_path=$1
    shift
    local sub_script_args=("$@")

    if [ ! -f "$script_path" ]; then
        echo "[WARNING] Script '$script_path' not found. Skipping."
        return
    fi

    chmod +x "$script_path"

    echo "[CMD] \"$script_path\" \"${sub_script_args[@]}\""
    "$script_path" "${sub_script_args[@]}"
}

# --- 4. Common args ---
sub_script_args=()

echo "--- Configuration Summary ---"
echo "trusted_install_prefix: $trusted_install_prefix_val"

sub_script_args+=(--trusted-install-prefix "$trusted_install_prefix_val")

if [[ "$install_libs_together_val" == "true" ]]; then
    untrusted_path="${trusted_install_prefix_val}/__untrusted_dependencies"

    echo "Install Untrusted Libs Together: YES"
    echo "Path for Untrusted Libs: $untrusted_path"

    sub_script_args+=(--untrusted-install-prefix "$untrusted_path")
else
    echo "Install Untrusted Libs Together: NO"
fi

if [[ -n "$only_dependency" ]]; then
    echo "Only installing: $only_dependency"
fi

echo "---------------------------"


# --- 5. Execute in order ---
for dependency in "${dependency_list[@]}"; do
    # If --only is specified and doesn't match this one, skip
    if [[ -n "$only_dependency" && "$only_dependency" != "$dependency" ]]; then
        continue
    fi

    script_name="${external_dependencies[$dependency]}"
    script_path="${base_dir}/${dependency}/${script_name}"

    log "Processing dependency: $dependency"
    execute_script "$script_path" "${sub_script_args[@]}"
done

log "All requested dependencies have been processed."
