
## The Amalgam Engine - An engine for easily creating virtual worlds

For more information, visit [worlds.place](https://worlds.place/).

If you'd like to get involved, please join the Discord: https://discord.gg/EA2Sg3ar74

## Vision
(Not all implemented yet, see [Roadmap](https://worlds.place/roadmap.html))
* Easily create your own isometric, sprite-based virtual world.
* Start from a template and have a full working world, including client, server, text chat, and account management.
* All needed networking is built-in, and adding new messages for your custom features is extremely easy.
* Supports 1000+ users in groups of 10, or 150+ users in 1 area, all being very active.
* Targeted for use on relatively low-spec hardware (tested on a $30/mo rented server).
* Live, in-world map editing. Use permissions to let players build things, or restrict it to your developers.

## Template Projects
### Repose
Repose is our first template project. If you'd like to make a world, you can fork Repose and use it as a fully-functioning starting point.

[Check out the project and download the latest playable release here.](https://github.com/Net5F/Repose)

## Building
Note: You rarely need to build the engine by itself, this section just provides canonical instructions. Instead, see the Template Projects section.

### Windows
#### Visual Studio (MSVC)
1. Open CMakeLists.txt in Visual Studio (`Open` -> `CMake`).
1. (Optional) Open CMakeSettings.json (in this repo) and enable `AM_BUILD_SPRITE_EDITOR` to build the sprite editor.
1. `Project` -> `Generate CMake cache` (or just let it run if you have auto-config on).
1. `Build` -> `Build All`

Note: The Sprite Editor should be built within your project, since it relies on config values from your project's Override/SharedConfig.h.

#### MinGW
For MSYS2/MinGW, we don't have a dependency install script. Here's the list:

    pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja mingw-w64-x86_64-gdb mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_image mingw-w64-x86_64-SDL2_mixer mingw-w64-x86_64-SDL2_ttf mingw-w64-x86_64-SDL2_gfx mingw-w64-x86_64-SDL2_net mingw-w64-x86_64-catch
    
Then, build through the Eclipse project or follow the Linux instructions for a command line build.

### Linux
Note: This is only tested on Ubuntu 20.04. If you have experience in multi-distro builds, please get involved!

1. Run `Scripts/Linux/InstallDependencies.sh`, then build through the Eclipse project, or:
1. (From the base of the repo) `mkdir -p Build/Linux/Release`
1. `cd Build/Linux/Release`
1. `cmake -DCMAKE_BUILD_TYPE=Release -G Ninja ../../../`
   1. You can optionally add `-DAM_BUILD_SPRITE_EDITOR` to build the sprite editor.
1. `ninja all`

### macOS

Note: AmalgamEngine requires GCC to build on macOS. Clang / Apple Clang does not yet support C++ features required by this project (as of Apple Clang 14 / Clang 15).

1. Use the Homebrew package manager to install dependencies: `brew install gcc make cmake ninja sdl2 sdl2_image sdl2_mixer sdl2_ttf sdl2_gfx sdl2_net`
1. (From the base of the repo) `mkdir -p Build/macOS/Release`
1. `cd Build/macOS/Release`
1. `CC=gcc-NN CXX=g++-NN cmake -DCMAKE_BUILD_TYPE=Release -G Ninja ../../../`. Replace the `NN` in `gcc-NN` and `g++-NN` with the version of GCC that you installed with Homebrew. This is important, as using `gcc` without a version number will alias to `clang`.
   1. ~~You can optionally add `-DAM_BUILD_SPRITE_EDITOR` to build the sprite editor.~~ Sprite editor doesn't currently build on macOS due to GCC not being able to build with Apple SDK headers which use certain Objective C extensions.
1. `ninja all`

## Packaging
Note: You rarely need to package the engine by itself, this section just provides canonical instructions. Instead, see the Template Projects section.

To package the applications in a way that can be shared, first run the desired build. Then, run:
```
// Assuming you're at the base of the repo.
cmake --install Build/Windows/Release --prefix Packages/Windows
```
where 'Build/Windows/Release' is your desired build to package, and 'Packages/Windows' is your desired output directory.

On Windows, you can use Visual Studio's developer terminal (`Tools` -> `Command Line` -> `Developer Command Prompt`) for easy access to CMake.

## Contributing
Contributions are very welcome! Feel free to work on features or bugfixes and submit PRs, they'll all be promptly reviewed.

There's a fairly solid vision for the near-future of this engine. If you would like to work on a larger feature, please make sure it's on [the roadmap](https://worlds.place/roadmap.html), or [join the discord](https://discord.gg/EA2Sg3ar74) and discuss it before getting started.
