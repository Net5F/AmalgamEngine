@echo off
setlocal EnableDelayedExpansion

:: BatchGotAdmin
:-------------------------------------
REM  --> Check for permissions
    IF "%PROCESSOR_ARCHITECTURE%" EQU "amd64" (
>nul 2>&1 "%SYSTEMROOT%\SysWOW64\cacls.exe" "%SYSTEMROOT%\SysWOW64\config\system"
) ELSE (
>nul 2>&1 "%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system"
)

REM --> If error flag set, we do not have admin.
if '%errorlevel%' NEQ '0' (
    echo Requesting administrative privileges...
    goto UACPrompt
) else ( goto gotAdmin )

:UACPrompt
    echo Set UAC = CreateObject^("Shell.Application"^) > "%temp%\getadmin.vbs"
    set params= %*
    echo UAC.ShellExecute "cmd.exe", "/c ""%~s0"" %params:"=""%", "", "runas", 1 >> "%temp%\getadmin.vbs"

    "%temp%\getadmin.vbs"
    del "%temp%\getadmin.vbs"
    exit /B

:gotAdmin
    pushd "%CD%"
    CD /D "%~dp0"
:-------------------------------------- 

rem Library binary URLs:
rem SDL2
set "SDL2File=SDL2-devel-2.0.20-VC.zip"
set "SDL2Folder=SDL2-2.0.20"
set "SDL2URL=https://www.libsdl.org/release/%SDL2File%"
rem SDL2_image
set "SDLImageFile=SDL2_image-devel-2.0.5-VC.zip"
set "SDLImageFolder=SDL2_image-2.0.5"
set "SDLImageURL=https://www.libsdl.org/projects/SDL_image/release/%SDLImageFile%"
rem SDL2_mixer
set "SDLMixerFile=SDL2_mixer-devel-2.0.4-VC.zip"
set "SDLMixerFolder=SDL2_mixer-2.0.4"
set "SDLMixerURL=https://www.libsdl.org/projects/SDL_mixer/release/%SDLMixerFile%"
rem SDL2_ttf
set "SDLTTFFile=SDL2_ttf-devel-2.20.1-VC.zip"
set "SDLTTFFolder=SDL2_ttf-2.20.1"
set "SDLTTFURL=https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.20.1/%SDLTTFFile%"
rem SDL2_gfx
set "SDLGFXFile=SDL2_gfx-devel-1.0.4.zip"
set "SDLGFXFolder=SDL2_gfx-1.0.4"
set "SDLGFXURL=https://github.com/Net5F/SDL_gfx/releases/download/1.0.4/%SDLGFXFile%"

echo "Amalgam Engine Dependency Installer"
echo "Downloads the engine's dependencies to the specified folder."
echo "Sets environment variables, so the build system can find the dependencies."
echo "Adds a system path entry (if one doesn't already exist), so project executables can be ran from the build directory."
echo:
echo "Dependencies to be installed: SDL2, SDL2_Image, SDL2_Mixer, SDL2_TTF, SDL2_GFX"
echo:
echo "Note: This script doesn't delete existing folders (though it may overwrite)."
echo "      If you have conflicting files at the given path, you should probably delete them before running this."
echo:

set /p UserPath=Please enter the path to install to: 

rem ## Download and install the libraries. ##
rem Make the install folders if they don't already exist.
echo "Creating install folders (if they don't already exist)..."
set "InstallPath=%UserPath%\SDL2"
if not exist %InstallPath% mkdir %InstallPath%
if not exist %InstallPath%\bin mkdir %InstallPath%\bin

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
echo "Deleting temporary files..."
del %InstallPath%\%SDL2File%
del %InstallPath%\%SDLImageFile%
del %InstallPath%\%SDLMixerFile%
del %InstallPath%\%SDLTTFFile%
del %InstallPath%\%SDLGFXFile%

rem Copy all the dlls into "bin".
echo "Copying dlls into %BinPath%..."
set "BinPath=%InstallPath%\bin"
for /r %InstallPath% %%x in (x64\*.dll) do copy "%%x" %BinPath% >nul

rem ## Add bin to the system PATH (if it isn't already added). ##
rem Note: We add this so that we don't have to copy the SDL binaries into the 
rem       build folder in order to run the project binaries.

rem Check if it's already in the user or system PATH.
echo %PATH% | findstr "%BinPath%" > nul

rem If our bin path wasn't found in the current session's PATH, add it to the system PATH.
if ERRORLEVEL 1 (
    set Key="HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment"
    for /F "USEBACKQ tokens=2*" %%A in (`reg query %%Key%% /v PATH`) do (
        if not "%%~B" == "" (
            rem Preserve the existing system PATH.
            echo %%B > SystemPATHBackup.txt
      
            rem Update the system PATH.
            setx PATH "%%B;!BinPath!" /M
        )
    )
    echo "Added %BinPath% to the system PATH. Previous PATH has been saved in SystemPATHBackup.txt."
) else (
    echo "Found %BinPath% in user or system path. Proceeding without re-adding."
)

rem ## Add the library paths as environment variables. ##
rem Note: We add these so that the CMake "Find" files can find the libraries.
rem       We don't check if they already exist or back them up like we do with 
rem       the system PATH, because they're less scary.
echo "Setting environment variables..."
setx SDLDIR %InstallPath%\%SDL2Folder% /M
setx SDL2IMAGEDIR %InstallPath%\%SDLImageFolder% /M
setx SDL2MIXERDIR %InstallPath%\%SDLMixerFolder% /M
setx SDL2TTFDIR %InstallPath%\%SDLTTFFolder% /M
setx SDL2GFXDIR %InstallPath%\%SDLGFXFolder% /M

echo:
echo "Successfully installed dependencies to %InstallPath%. You may close this window."
echo:
echo "** Please close and re-open any IDEs or terminals to refresh your environment variables. **"

:End
pause >nul
