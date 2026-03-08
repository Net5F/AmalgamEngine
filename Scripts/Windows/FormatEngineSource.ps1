# Grab the full path to the repo root (assuming this script is in Scripts/Windows)
$BasePath = Resolve-Path "$PSScriptRoot\..\.."

# Find clang-format shipped with Visual Studio
$ClangFormat = Get-ChildItem -Path "${env:ProgramFiles}\Microsoft Visual Studio" `
    -Filter "clang-format.exe" -Recurse -ErrorAction SilentlyContinue |
    Select-Object -First 1 -ExpandProperty FullName

if (-not $ClangFormat) {
    Write-Error "clang-format not found. Install the 'C++ Clang tools for Windows' component via the VS Installer."
    exit 1
}

Write-Host "Running clang-format on engine source code."
Get-ChildItem -Path "$BasePath\Source" -Recurse -Include "*.h", "*.cpp" |
    ForEach-Object { & $ClangFormat -i $_.FullName }
Write-Host "Done."
