# Windows Development Setup

## Goal

Chuẩn hóa môi trường dev cho `Project Phoenix v1 beta`.

Target stack:

* `MSVC 2019` hoặc `MSVC 2022`
* `CMake >= 3.21`
* `Qt 6`
* `OpenGL`
* `Assimp`

---

## Required Tools

## 1. Visual Studio

Cài:

* `Visual Studio 2019` hoặc `Visual Studio 2022`
* workload `Desktop development with C++`

Cần có:

* MSVC compiler
* Windows SDK
* CMake tools

## 2. CMake

Yêu cầu:

* `CMake 3.21+`

Kiểm tra:

```powershell
cmake --version
```

Nếu shell không nhận `cmake`, thêm vào `PATH` hoặc cài bản standalone.

## 3. Qt 6

Yêu cầu:

* `Qt 6`
* bản `MSVC 64-bit` phù hợp compiler đang dùng

Ví dụ:

* `Qt 6.5.x msvc2019_64`
* `Qt 6.6.x msvc2019_64`
* `Qt 6.6.x msvc2022_64`

Kiểm tra:

* `Qt6Config.cmake` phải tồn tại trong:

```text
<QtRoot>/lib/cmake/Qt6
```

## 4. Assimp

Yêu cầu:

* `Assimp` cho import `FBX`

Tạm thời có thể:

* build từ source
* dùng package manager
* hoặc vendor vào `third_party/`

---

## Configure Build

Ví dụ:

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:\Qt\6.6.3\msvc2019_64"
cmake --build build --config Debug
```

## Deploy Qt Runtime for Local Launch

Nếu chạy `.exe` trực tiếp ngoài `Developer Command Prompt`, cần deploy Qt runtime:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\deploy_qt_runtime.ps1
```

Script này dùng `windeployqt` để copy các Qt DLL cần thiết cạnh file `.exe`.

Nếu `Assimp` được install ở vị trí riêng, có thể cần thêm:

```powershell
-DCMAKE_PREFIX_PATH="C:\Qt\6.6.3\msvc2019_64;C:\deps\assimp"
```

---

## Current Local Machine Findings

Trong môi trường hiện tại, đã kiểm tra:

* chưa thấy `cmake` trên `PATH`
* chưa thấy `git` trên `PATH`
* có `qmake` từ `Autodesk Maya 2024`
* Qt đi kèm Maya là `Qt 5.15.2`, không phải `Qt 6`

Kết luận:

* repo đã scaffold xong
* build thật chưa verify được trên máy hiện tại
* cần cài hoặc expose `CMake` + `Qt 6` trước khi chạy bước build

---

## Recommended Standard

Chuẩn team nên chốt:

* `Visual Studio 2022`
* `Qt 6.6.x msvc2022_64`
* `CMake 3.27+`

Lý do:

* mới hơn
* ít friction hơn cho `Qt 6`
* hợp hướng project dài hạn
