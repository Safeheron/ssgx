#!/bin/bash

# Define variables
top_dir="$(cd "$(dirname "$(realpath "${0}")")" && pwd)"
toml11_url="https://github.com/ToruNiina/toml11/archive/refs/tags/v3.8.1.zip"
toml11_zip="$top_dir/toml11-v3.8.1.zip"
toml11_dir="$top_dir/toml11-3.8.1"
build_dir="$toml11_dir/build"

# Define expected checksum (replace with the actual value if needed)
expected_chksum="72e956f42002dd1566c5551a693ec0f6fa3bea3a0e7bcea29bcdace98738da74"
checksum_file="$top_dir/checksum_toml11.txt"

# Remove old checksum file if it exists
rm -f $checksum_file

# Download the toml11 archive if it doesn't already exist
if [ ! -f "$toml11_zip" ]; then
    echo "Downloading toml11 v3.8.1..."
    wget -O "$toml11_zip" "$toml11_url" || { echo "Failed to download toml11"; exit 1; }
fi

# Verify the downloaded file's integrity if a checksum is provided
if [ -n "$expected_chksum" ]; then
    echo "Verifying checksum of the downloaded file..."
    sha256sum $toml11_zip > $checksum_file
    grep $expected_chksum $checksum_file
    if [ $? -ne 0 ]; then
        echo "Checksum verification failed."
        rm -f $toml11_zip
        exit 1
    fi
fi

# Extract the toml11 archive if it hasn't been extracted yet
if [ ! -d "$toml11_dir" ]; then
    echo "Extracting toml11 source files..."
    unzip -qq $toml11_zip -d $top_dir || { echo "Failed to extract toml11"; exit 1; }
fi

# Create the build directory if it doesn't exist
if [ ! -d "$build_dir" ]; then
    echo "Creating the build directory..."
    mkdir -p $build_dir || { echo "Failed to create build directory"; exit 1; }
fi

# Navigate to the build directory, configure, and build
cd $build_dir || exit 1
echo "Configuring and building toml11..."
cmake .. -DCMAKE_CXX_STANDARD=11 || { echo "CMake configuration failed"; exit 1; }
sudo make install || { echo "Build and installation failed"; exit 1; }

echo "toml11 installation completed successfully."
