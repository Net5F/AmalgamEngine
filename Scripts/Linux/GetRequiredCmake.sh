#!/bin/bash

# This script builds a new enough cmake from source to use for this project.
# If your OS version is new enough, you won't need to use this.

version=3.17.3

sudo apt install build-essential libssl-dev
sudo apt purge --auto-remove cmake

mkdir temp
cd temp

wget https://github.com/Kitware/CMake/releases/download/v$version/cmake-$version.tar.gz --no-check-certificate

tar -xzvf cmake-$version.tar.gz

cd CMake-$version

# Build and install
./bootstrap
make -j$(nproc)
sudo make install

cd ../..
rm -r temp

