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
set "SDLTTFFile=SDL2_ttf-devel-2.0.18-VC.zip"
set "SDLTTFFolder=SDL2_ttf-2.0.18"
set "SDLTTFURL=https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.0.18/%SDLTTFFile%"
rem SDL2_gfx
set "SDLGFXFile=SDL2_gfx-devel-1.0.4.zip"
set "SDLGFXFolder=SDL2_gfx-1.0.4"
set "SDLGFXURL=https://github.com/Net5F/SDL_gfx/releases/download/1.0.4/%SDLGFXFile%"

set "argC=0"
for %%x in (%*) do Set /A argC+=1

if not "%argC%"=="1" (
    echo "Downloads the engine's dependencies to the given folder and adds it to the system path (if it isn't already added)."
    echo "Usage: InstallDependencies.bat <InstallPath>"
    echo "Note: This script doesn't delete existing folders (though it may overwrite). If you have conflicting files at the given path, you should probably delete them before running this."
    goto End
)

rem ## Download and install the libraries. ##
rem Make the install folders if they don't already exist.
echo "Creating install folders (if they don't already exist)..."
set "InstallPath=%~f1\SDL2"
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
set "BinPath=%InstallPath%\bin"
for /r %InstallPath% %%x in (x64\*.dll) do copy "%%x" %BinPath% >nul

rem ## Add bin to the system PATH (if it isn't already added). ##
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
    echo "Found %BinPath% in user or system path. Proceeding without adding it again."
)

echo "Successfully installed dependencies to %InstallPath%. You may close this window."

:End
pause >nul
