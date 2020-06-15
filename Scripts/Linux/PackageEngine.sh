#!/bin/bash

if [[ $# -eq 0 ]] ; then
    echo "Packages the client and server binaries alongside necessary resources and shared libs."
    echo "Usage: ./PackageEngine <Debug/Release>"
    exit 1
fi

# Grab the full path to the base of the repo (assuming this script is in Scripts/Linux)
BasePath="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../.." >/dev/null 2>&1 && pwd )"

# Check that the build files exist.
Config=$1
BuildPath=$BasePath/Build/Linux/$Config
if ! [ -f "$BuildPath/Server/Server" ] || ! [ -f "$BuildPath/Client/Client" ] ; then
    echo "Build files for $Config config were not found. Please build before attempting to package."
fi

# Make the directories we're going to use.
PackagePath=$BasePath/Output/Linux/$Config
mkdir -p $PackagePath/Client
mkdir -p $PackagePath/Server

# Clear out the package directories to prep for the new files.
rm -r $PackagePath/Client/* >/dev/null 2>&1
rm -r $PackagePath/Server/* >/dev/null 2>&1

echo "Starting package process."

# Copy the client and server binaries
cp $BuildPath/Client/Client $PackagePath/Client/
cp $BuildPath/Server/Server $PackagePath/Server/

# Copy the resource files to the client.
cp -r $BasePath/Resources $PackagePath/Client/

# Detect and copy dependencies.
cmake -P $BasePath/CMake/copy_runtime_deps.cmake $BuildPath/Client/Client $PackagePath/Client/
cmake -P $BasePath/CMake/copy_runtime_deps.cmake $BuildPath/Server/Server $PackagePath/Server/
    
