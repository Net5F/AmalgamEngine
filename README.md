## The Amalgam Engine - An engine for easily creating virtual worlds
[![Watch 150 clients in 1 area load test](https://img.youtube.com/vi/N7eCfXj04cE/maxresdefault.jpg)](https://youtu.be/N7eCfXj04cE)
<p align="center">
   <i>One of our single-area load tests - 150 clients constantly moving. Click to watch.</i>
</p>
&nbsp;

![BuildMode](https://user-images.githubusercontent.com/17211746/173250545-7b5b66fb-b8b6-46ba-a301-69f4c8d818e4.jpg)
<p align="center">
   <i>Our in-world "Build Mode".</i>
</p>
&nbsp;

![SpriteEditor](https://user-images.githubusercontent.com/17211746/173250552-2cc9db87-f748-4417-83c9-258c8720bf44.jpg)
<p align="center">
   <i>Our Sprite Editor, used to draw 3D bounding boxes on sprites.</i>
</p>

## Vision
(Not all implemented yet, see Status section)
* Easily create isometric, sprite-based virtual worlds.
* Start from a template and have a full working world, including client, server, text chat, and account management.
* All needed networking is built-in, and adding new messages for your custom features is extremely easy.
* Load tested with 150 clients in 1 area or 1000 clients in groups of 10, all sending 4 inputs/second.
* Targeted for use on relatively low-spec hardware (tested on a $30/mo rented server).
* Live, in-world map editing. Use permissions to let players build things, or restrict it to your developers.

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
- [ ] Split single repo into engine and project repos.

### Future
- [ ] Implement text chat, chat server.
- [ ] Implement sprite animation.
- [ ] Implement login server, account db, account validation.
- [ ] Implement web server, account creation.
- [ ] MVP is done.

## Joining the demo world
To join the demo world and play with other people, you can [download the latest release](https://github.com/Net5F/Amalgam/releases/latest) and follow the instructions in the README.

Currently, the client application is hardcoded to connect to a server ran by Net_.

## Building
### Windows
#### Visual Studio (MSVC)
1. Run `Scripts/Windows/InstallDependencies.bat`, passing it the path you want to install the dependencies to.
1. Open CMakeLists.txt in Visual Studio (`Open` -> `CMake`).
1. Update CMakeSettings.json (in this repo) to point at the various SDL folders in your installation path.
   1. You'll leave this file dirty. Don't try to commit it back upstream with your personal paths.
   1. You can optionally enable `AM_BUILD_SPRITE_EDITOR` to build the sprite editor.
1. `Project` -> `Generate CMake cache` (or just let it run if you have auto-config on).
1. `Build` -> `Build All`

#### MinGW
For MSYS2/MinGW, we don't have a dependency install script. Here's the list:

    pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja mingw-w64-x86_64-gdb mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_image mingw-w64-x86_64-SDL2_mixer mingw-w64-x86_64-SDL2_ttf mingw-w64-x86_64-SDL2_gfx mingw-w64-x86_64-SDL2_net mingw-w64-x86_64-catch
    
Then, build through the Eclipse project or follow the linux instructions for a command line build.

### Linux
Had to update to Ubuntu 21.04 for g++ 10.3.0 (older versions didn't have support for some C++20 features I needed.)
I haven't yet locked down an SDL2 or CMake version, we'll see where it ends up.

1. Run `Scripts/Linux/InstallDependencies.sh`, then build through the Eclipse project, or:
1. (From the base of the repo) `mkdir -p Build/Linux/Release`
1. `cd Build/Linux/Release`
1. `cmake -DCMAKE_BUILD_TYPE=Release -G Ninja ../../../`
   1. You can optionally add `-DAM_BUILD_SPRITE_EDITOR` to build the sprite editor.
1. `ninja all`

## Contributing
### Bugs
Bug reports and fixes are always welcome. Feel free to open an issue or submit a PR.

### Features
**Unsolicited feature PRs will not be reviewed. Please ask about the feature plan before working on a feature.**

Collaboration is very welcome! That being said, there is a fairly solid vision for the near-future of this engine. If you would like to contribute expertise or take ownership over a feature on the roadmap, please make a post [on the discussion board](https://github.com/Net5F/AmalgamEngine/discussions/categories/feature-development).
