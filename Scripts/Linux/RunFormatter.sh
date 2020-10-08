#!/bin/bash

# Grab the full path to the base of the repo (assuming this script is in Scripts/Linux)
BasePath="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../.." >/dev/null 2>&1 && pwd )"

echo "Running clang-format."
find $BasePath/Source -iname '*.h' -o -iname '*.cpp' | xargs clang-format -i
echo "Done."
    
