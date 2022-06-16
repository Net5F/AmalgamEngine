@echo off
setlocal EnableDelayedExpansion

set "argC=0"
for %%x in (%*) do Set /A argC+=1

if not "%argC%"=="1" (
    echo "Packages the sprite editor binary alongside necessary resources and shared libs."
    echo "Usage: PackageEngine.bat <Debug/Release>"
    goto End
)

rem Check that CMake exists.
WHERE /q cmake
if %ERRORLEVEL% NEQ 0 (
    echo "CMake not found. Either install it, or run this script from the Visual Studio command line (Tools -> Command Line -> Developer Command Prompt)."
    goto End
)

rem Grab the full path to the base of the repo (assuming this script is in Scripts/Windows).
set "ScriptPath=%~dp0"
pushd %ScriptPath%
cd ../..
set "BasePath=!cd!"
popd

rem Check that the build files exist.
set "Config=%1"
set "BuildPath=%BasePath%\Build\Windows\%Config%\Source"

set "Result=0"
if not exist %BuildPath%\SpriteEditor\SpriteEditor.exe set "Result=1"
if "%Result%"=="1" (
    echo "Build files for %Config% were not found. Please build before attempting to package."
    goto End
)

rem Make the directories that we're going to use.
set "PackagePath=%BasePath%\Output\Windows\%Config%"
if not exist %PackagePath%\SpriteEditor mkdir %PackagePath%\SpriteEditor

rem Clear out the package directories to prep for the new files.
del /s /f /q %PackagePath%\SpriteEditor\*.* >nul 2>&1
for /f %%f in ('dir /ad /b %PackagePath%\SpriteEditor\') do rd /s /q %PackagePath%\SpriteEditor\%%f

echo "Starting packaging process."

rem Copy the binaries.
robocopy "%BuildPath%\SpriteEditor" "%PackagePath%\SpriteEditor" "SpriteEditor.exe" >nul 2>&1

rem Copy the resource files.
robocopy "%BasePath%\Resources\SpriteEditor\Common" "%PackagePath%\SpriteEditor" /E >nul 2>&1
robocopy "%BasePath%\Resources\SpriteEditor\Windows" "%PackagePath%\SpriteEditor" /E >nul 2>&1

rem Detect and copy dependencies.
cmake -P "%BasePath%\CMake\copy_windows_deps.cmake" "%PackagePath%\SpriteEditor" false

echo "Packaging complete. Package can be found at %PackagePath%"

:End
