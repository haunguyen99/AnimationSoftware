param(
    [string]$QtRoot = "C:\Qt\6.11.1\msvc2022_64",
    [string]$BuildDir = "E:\Animation Software\build\ninja-msvc-debug"
)

$vsDevCmd = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
$cmake = "C:\Program Files\CMake\bin\cmake.exe"
$ninja = "C:\Qt\Tools\Ninja\ninja.exe"
$vcpkgToolchain = "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\vcpkg\scripts\buildsystems\vcpkg.cmake"

if (-not (Test-Path $vsDevCmd)) {
    throw "VsDevCmd not found: $vsDevCmd"
}

if (-not (Test-Path $cmake)) {
    throw "cmake not found: $cmake"
}

if (-not (Test-Path $ninja)) {
    throw "ninja not found: $ninja"
}

$prefixPath = "$QtRoot;E:/Animation Software/vcpkg_installed/x64-windows"
$configure = "`"$vsDevCmd`" -arch=x64 && `"$cmake`" -S . -B `"$BuildDir`" -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=`"$prefixPath`" -DCMAKE_MAKE_PROGRAM=$ninja -DCMAKE_TOOLCHAIN_FILE=`"$vcpkgToolchain`" -DVCPKG_TARGET_TRIPLET=x64-windows"
$build = "`"$vsDevCmd`" -arch=x64 && `"$cmake`" --build `"$BuildDir`""

cmd.exe /c $configure
if ($LASTEXITCODE -ne 0) {
    throw "Configure failed with exit code $LASTEXITCODE"
}

cmd.exe /c $build
if ($LASTEXITCODE -ne 0) {
    throw "Build failed with exit code $LASTEXITCODE"
}

Write-Output "Build completed: $BuildDir"
