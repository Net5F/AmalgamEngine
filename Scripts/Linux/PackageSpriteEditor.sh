#!/bin/bash

if [[ $# -eq 0 ]] ; then
    echo "Packages the sprite editor binary alongside necessary resources and shared libs."
    echo "Usage: ./PackageSpriteEditor <Debug/Release>"
    exit 1
fi

# Grab the full path to the base of the repo (assuming this script is in Scripts/Linux)
BasePath="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../.." >/dev/null 2>&1 && pwd )"

# Check that the build files exist.
Config=$1
BuildPath=$BasePath/Build/Linux/$Config/Source
if ! [ -f "$BuildPath/SpriteEditor/SpriteEditor" ] ; then
    echo "Build files for $Config config were not found. Please build before attempting to package."
    exit 1
fi

# Make the directories that we're going to use.
PackagePath=$BasePath/Output/Linux/$Config
mkdir -p $PackagePath/SpriteEditor

# Clear out the package directories to prep for the new files.
rm -r $PackagePath/SpriteEditor/* >/dev/null 2>&1

echo "Starting packaging process."

# Copy the binaries.
cp $BuildPath/SpriteEditor/SpriteEditor $PackagePath/SpriteEditor/

# Copy the resource files.
cp -r $BasePath/Resources/SpriteEditor/Common/* $PackagePath/SpriteEditor/
cp -r $BasePath/Resources/SpriteEditor/Linux/* $PackagePath/SpriteEditor/

echo "Packaging complete. Package can be found at $PackagePath"
