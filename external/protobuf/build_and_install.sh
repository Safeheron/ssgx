#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

# Define variables
TOP_DIR="$(cd "$(dirname "$(realpath "$0")")" && pwd)"
REPO_URL="https://github.com/protocolbuffers/protobuf.git"
REPO_DIR="${TOP_DIR}/protobuf"
VERSION="v3.20.0"

# Clone the repository if it doesn't already exist
if [ ! -d "$REPO_DIR" ]; then
    echo "Cloning protobuf repository..."
    git clone "$REPO_URL" "$REPO_DIR" || { echo "Failed to clone repository"; exit 1; }
else
    echo "Repository already cloned. Pulling latest changes..."
    cd "$REPO_DIR" || exit 1
    git fetch origin || { echo "Failed to fetch latest changes"; exit 1; }
    cd "$TOP_DIR" || exit 1
fi

# Navigate to the repository
cd "$REPO_DIR" || exit 1

# Check out the specific branch or tag
echo "Checking out version $VERSION..."
git checkout "$VERSION" || { echo "Failed to checkout version $VERSION"; exit 1; }

# Update and initialize submodules
echo "Initializing and updating submodules..."
git submodule update --init --recursive || { echo "Failed to update submodules"; exit 1; }

echo "Run autogen.sh and configure..."
./autogen.sh || { echo "Failed to autogen"; exit 1; }
./configure || { echo "Failed to configure"; exit 1; }

# Build
echo "Building protobuf..."
make -j$(nproc) || { echo "Build failed"; exit 1; }

# Install
echo "Installing protobuf..."
sudo make install || { echo "Install failed"; exit 1; }

echo "Protobuf installation completed successfully."
