param(
    [string]$QtRoot = "C:\Qt\6.11.1\msvc2022_64",
    [string]$BuildDir = "E:\Animation Software\build\ninja-msvc-debug",
    [string]$ExePath = "E:\Animation Software\build\ninja-msvc-debug\apps\editor\phoenix_editor.exe",
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug"
)

$windeployqt = Join-Path $QtRoot "bin\windeployqt.exe"
$vcpkgBinDir = if ($Configuration -eq "Debug") {
    "E:\Animation Software\vcpkg_installed\x64-windows\debug\bin"
} else {
    "E:\Animation Software\vcpkg_installed\x64-windows\bin"
}

if (-not (Test-Path $windeployqt)) {
    throw "windeployqt not found: $windeployqt"
}

if (-not (Test-Path $ExePath)) {
    throw "Executable not found: $ExePath"
}

& $windeployqt `
    --dir (Split-Path $ExePath -Parent) `
    --no-translations `
    --no-system-d3d-compiler `
    --no-compiler-runtime `
    --verbose 0 `
    $ExePath

if ($LASTEXITCODE -ne 0) {
    throw "windeployqt failed with exit code $LASTEXITCODE"
}

if (Test-Path $vcpkgBinDir) {
    Get-ChildItem -Path $vcpkgBinDir -Filter *.dll | ForEach-Object {
        Copy-Item -LiteralPath $_.FullName -Destination (Split-Path $ExePath -Parent) -Force
    }
}

Write-Output "Qt runtime deployed to $(Split-Path $ExePath -Parent)"
