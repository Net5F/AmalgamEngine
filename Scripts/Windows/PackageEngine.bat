@echo off
setlocal EnableDelayedExpansion

set "argC=0"
for %%x in (%*) do Set /A argC+=1

if not "%argC%"=="1" (
    echo "Packages the client and server binaries alongside necessary resources and shared libs."
    echo "Usage: PackageEngine.bat <Debug/Release>"
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
set "BuildPath=%BasePath%\Build\Windows\%Config%"

set "Result=0"
if not exist %BuildPath%\Server\Server.exe set "Result=1"
if not exist %BuildPath%\Client\Client.exe set "Result=1"
if "%Result%"=="1" (
    echo "Build files for %Config% were not found. Please build before attempting to package."
    goto End
)

rem Make the directories we're going to use.
set "PackagePath=%BasePath%\Output\Windows\%Config%"
if not exist %PackagePath%\Client mkdir %PackagePath%\Client
if not exist %PackagePath%\Server mkdir %PackagePath%\Server

rem Clear out the package directories to prep for the new files.
del /s /f /q %PackagePath%\Client\*.* >nul 2>&1
for /f %%f in ('dir /ad /b %PackagePath%\Client\') do rd /s /q %PackagePath%\Client\%%f

del /s /f /q %PackagePath%\Server\*.* >nul 2>&1
for /f %%f in ('dir /ad /b %PackagePath%\Server\') do rd /s /q %PackagePath%\Server\%%f

echo "Starting package process."

rem Copy the client and server binaries.
robocopy "%BuildPath%\Client" "%PackagePath%\Client" "Client.exe" >nul 2>&1
robocopy "%BuildPath%\Server" "%PackagePath%\Server" "Server.exe" >nul 2>&1

rem Copy the resource files to the client.
robocopy "%BasePath%\Resources" "%PackagePath%\Client\Resources" /E >nul 2>&1

rem Detect and copy dependencies.
cmake -P "%BasePath%\CMake\copy_runtime_deps.cmake" "%BuildPath%\Client\Client.exe" "%PackagePath%\Client"
cmake -P "%BasePath%\CMake\copy_runtime_deps.cmake" "%BuildPath%\Server\Server.exe" "%PackagePath%\Server"


:End
