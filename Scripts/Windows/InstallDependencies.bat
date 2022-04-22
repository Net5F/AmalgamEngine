@echo off
setlocal EnableDelayedExpansion

rem Library binary URLs:
rem SDL2
set "SDL2File=SDL2-devel-2.0.20-VC.zip"
set "SDL2URL=https://www.libsdl.org/release/%SDL2File%"
rem SDL2_image
set "SDLImageFile=SDL2_image-devel-2.0.5-VC.zip"
set "SDLImageURL=https://www.libsdl.org/projects/SDL_image/release/%SDLImageFile%"
rem SDL2_mixer
set "SDLMixerFile=SDL2_mixer-devel-2.0.4-VC.zip"
set "SDLMixerURL=https://www.libsdl.org/projects/SDL_mixer/release/%SDLMixerFile%"
rem SDL2_ttf
set "SDLTTFFile=SDL2_ttf-devel-2.0.18-VC.zip"
set "SDLTTFURL=https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.0.18/%SDLTTFFile%"
rem SDL2_gfx
set "SDLGFXFile=SDL2_gfx-devel-1.0.4.zip"
set "SDLGFXURL=https://github.com/Net5F/SDL_gfx/releases/download/1.0.4/%SDLGFXFile%"

set "argC=0"
for %%x in (%*) do Set /A argC+=1

if not "%argC%"=="1" (
    echo "Downloads the engine's dependencies to the given folder."
    echo "Usage: InstallDependencies.bat <InstallPath>"
    echo "Note: This script doesn't delete existing folders (though it may overwrite). If you have existing SDL2 libraries at the given path, you should probably delete them before running this."
    goto End
)

rem Make the install directory if it doesn't already exist.
set "InstallPath=%~f1"
for /f "tokens=1,2 delims=d" %%A in ("-%~a1") do if "%%B" neq "" (
    rem The given install directory exists and is a folder.
) else if "%%A" neq "-" (
    echo "The given path is a file. Must be empty or a folder."
    goto End
) else (
    echo "Created folder at %InstallPath%."
    mkdir %InstallPath%
)

rem Download the library binaries.
echo "Downloading library binaries..."

curl %SDL2URL% --output %InstallPath%\%SDL2File%
curl %SDLImageURL% --output %InstallPath%\%SDLImageFile%
curl %SDLMixerURL% --output %InstallPath%\%SDLMixerFile%
curl -L %SDLTTFURL% --output %InstallPath%\%SDLTTFFile%
curl -L %SDLGFXURL% --output %InstallPath%\%SDLGFXFile%

rem Unzip the binaries.
echo "Unzipping..."
tar -xf %InstallPath%\%SDL2File% --directory %InstallPath%\
tar -xf %InstallPath%\%SDLImageFile% --directory %InstallPath%\
tar -xf %InstallPath%\%SDLMixerFile% --directory %InstallPath%\
tar -xf %InstallPath%\%SDLTTFFile% --directory %InstallPath%\
tar -xf %InstallPath%\%SDLGFXFile% --directory %InstallPath%\

rem Delete the zip files.
echo "Deleting zip files..."
del %InstallPath%\%SDL2File%
del %InstallPath%\%SDLImageFile%
del %InstallPath%\%SDLMixerFile%
del %InstallPath%\%SDLTTFFile%
del %InstallPath%\%SDLGFXFile%

echo "The engine's dependencies have been downloaded. You now must update the SDL2-related include and library paths in CMakeSettings.json to point at the equivalent folders in %InstallPath%."

:End
