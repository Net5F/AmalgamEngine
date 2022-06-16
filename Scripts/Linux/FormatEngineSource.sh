#!/bin/bash

REQUIRED_PKG="clang-format"
PKG_OK=$(dpkg-query -W --showformat='${Status}\n' $REQUIRED_PKG|grep "install ok installed")
if [ "" = "$PKG_OK" ]; then
  echo "$REQUIRED_PKG not found. Installing."
  sudo apt --yes install $REQUIRED_PKG 
fi

# Grab the full path to the base of the repo (assuming this script is in Scripts/Linux)
BasePath="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../.." >/dev/null 2>&1 && pwd )"

echo "Running clang-format on engine source code."
find $BasePath/Source -iname '*.h' -o -iname '*.cpp' | xargs clang-format -i
echo "Done."
    
