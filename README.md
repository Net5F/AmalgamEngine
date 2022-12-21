
## The Amalgam Engine - An engine for easily creating virtual worlds

If you'd like to get involved, please join the Discord: https://discord.gg/EA2Sg3ar74

## Vision
(Not all implemented yet, see Status section)
* Easily create your own isometric, sprite-based virtual world.
* Start from a template and have a full working world, including client, server, text chat, and account management.
* All needed networking is built-in, and adding new messages for your custom features is extremely easy.
* Supports 1000+ users in groups of 10, or 150+ users in 1 area, all being very active.
* Targeted for use on relatively low-spec hardware (tested on a $30/mo rented server).
* Live, in-world map editing. Use permissions to let players build things, or restrict it to your developers.

## Worlds
### Repose
TODO: Add title screen and in-world screenshots.

Repose is our first template project. If you'd like to make a world, you can fork Repose and use it as a fully-functioning starting point.

[Check out the project and download the latest playable release here.](https://github.com/Net5F/Repose)

## Status
### Current
- [x] Implement, load test, and optimize the netcode.
- [x] Implement tile maps, iso rendering, and figure out architecture between the sim/rendering/UI.
- [x] Build minimal UI library.
- [x] Build sprite editor. (lets you add 3d bounding boxes to sprites for draw order and collision calcs)
- [x] Import sprite bounds data into engine.
- [x] Implement world map persistence.
- [x] Implement chunk and tile update streaming (live, in-world map editing).
- [x] Implement spatial partitioning grid for entities.
- [x] Implement interest management system.
- [x] Refactor UI library (spatial grid, nice event propagation).
- [x] Implement world editing UI.
- [x] Implement collision (simple placeholder).
- [x] Further netcode load testing (get ready for users).
- [x] Split single repo into engine and project repos.
- [x] Build simple template project.
- [x] Simplify build/packaging process.
- [ ] MVP features are done, template project is finished.

### Future
- [ ] Implement world interactions, decide on whether scripting is in-world or offline.
- [ ] Implement object database, inventory, picking up objects.
- [ ] Implement NPCs, potential in-world AI scripting.
- [ ] Implement text chat, chat server.
- [ ] Implement sprite animation.
- [ ] Implement login server, account db, account validation.
- [ ] Implement web server, account creation.

## Building
Note: You rarely need to build the engine by itself, this section just provides canonical instructions. Instead, see the Projects section.

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

## Packaging
Note: You rarely need to package the engine by itself, this section just provides canonical instructions. Instead, see the Projects section.

To package the applications in a way that can be shared, first run the desired build. Then, run:
```
// Assuming you're at the base of the repo.
cmake --install Build/Windows/Release --prefix Packages/Windows
```
where 'Build/Windows/Release' is your desired build to package, and 'Packages/Windows' is your desired output directory.

This is easily done on Windows using Visual Studio's developer terminal (`Tools` -> `Command Line` -> `Developer Command Prompt`).

## Contributing
### Bugs
Bug reports and fixes are always welcome. Feel free to open an issue or submit a PR.

### Features
**Unsolicited feature PRs will not be reviewed. Please ask about the feature plan before working on a feature.**

Collaboration is very welcome! That being said, there is a fairly solid vision for the near-future of this engine. If you would like to contribute expertise or take ownership over a feature on the roadmap, please make a post [on the discussion board](https://github.com/Net5F/AmalgamEngine/discussions/categories/feature-development).
