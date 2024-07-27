#!/bin/bash

# Installs the dependencies required to do an engine build.
# Tested on 20.04, 24.04.

sudo apt update

sudo apt install g++ cmake libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev libsdl2-gfx-dev libgtk-3-dev ninja-build
