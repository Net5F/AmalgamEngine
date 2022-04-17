#!/bin/bash

if [[ $# -eq 0 ]] ; then
    echo "Builds the engine targets selected in Source/CMakeLists.txt"
    echo "Usage: ./BuildEngine <Debug/Release> <(Optional) Clobber>"
    echo "Clobber as the second argument will clear the build directory before building."
    exit 1
fi

# Grab the full path to the base of the repo (assuming this script is in Scripts/Linux)
BasePath="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../.." >/dev/null 2>&1 && pwd )"

# Make the directory we're going to use.
Config=$1
BuildPath=$BasePath/Build/Linux/$Config
mkdir -p $BuildPath

# If a clobber was requested, clobber the old files.
if [ $# -eq 2 ] && [ $2 = "Clobber" ] ; then
    # Check if we're in the right directory.
    if ! [ -f "$BuildPath/CMakeCache.txt" ] ; then
        echo "Clobber requested, but CMakeCache.txt was not found in the build directory. Aborting for safety."
        exit 1
    fi
    
    rm -r $BuildPath/*
    echo "Clobbered $BuildPath"
fi

echo "Starting build."
cd $BuildPath
cmake -DCMAKE_BUILD_TYPE:STRING=$Config -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON -G Ninja $BasePath/
ninja all
    
