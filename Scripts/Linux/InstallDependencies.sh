#!/bin/bash

# Installs the dependencies required to do an engine build.
# Tested on 20.04, 24.04.

sudo apt update

sudo apt install g++ cmake libsdl3-dev libsdl3-image-dev libsdl3-mixer-dev libsdl3-ttf-dev libgtk-3-dev ninja-build
