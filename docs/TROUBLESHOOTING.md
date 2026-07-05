# Troubleshooting

## Qt DLL Missing at Launch

Symptom:

* app báo thiếu `Qt6Core.dll` hoặc Qt DLL khác

Fix:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\deploy_qt_runtime.ps1
```

## CMake Config Fails Because Compiler Missing

Symptom:

* `No CMAKE_CXX_COMPILER could be found`
* không thấy `cl.exe`

Fix:

* mở `Visual Studio Installer`
* modify `Visual Studio 2022`
* cài `Desktop development with C++`
* đảm bảo có `MSVC v143` + `Windows SDK`

## Build Works Only in Dev Env

Nguyên nhân:

* route build hiện dùng `VsDevCmd + Ninja`

Fix:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build_debug.ps1
```

## Qt Kit Wrong

Symptom:

* cài nhầm `mingw_64`
* project cần `msvc2022_64`

Fix:

* mở `C:\Qt\MaintenanceTool.exe`
* `Add or remove components`
* cài `Qt 6.x -> MSVC 2022 64-bit`

