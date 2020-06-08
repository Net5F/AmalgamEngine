#!/bin/bash

if [[ $# -eq 0 ]] ; then
    echo "Builds the engine targets selected in Source/CMakeLists.txt"
    echo "Usage: ./BuildEngine <Debug/Release> <(Optional) Clobber>"
    echo "Clobber as the second argument will clear the build directory before building."
    echo "Uses relative paths, so make sure to run this from Scripts/Linux"
    exit 1
fi

# Check if we're in the right directory.
if ! [ -f "BuildEngine.sh" ] ; then
    echo "BuildEngine.sh was not found in the current directory. Please run from Scripts/Linux."
fi

# Get into the right directory.
Config=$1
cd ../..
mkdir -p Build/Linux/$Config
cd Build/Linux/$Config

if [ $# -eq 2 ] && [ $2 = "Clobber" ] ; then
    # Check if we're in the right directory.
    if ! [ -f "CMakeCache.txt" ] ; then
        echo "Clobber requested, but CMakeCache.txt was not found in the current directory. Aborting for safety."
        exit 1
    fi
    
    rm -r ./*
    echo "Clobbered the directory."
fi

echo "Starting build."
cmake -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON -G Ninja ../../../Source/
ninja all
    
