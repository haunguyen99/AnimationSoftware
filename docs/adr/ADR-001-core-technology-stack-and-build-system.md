# ADR-001: Core Technology Stack and Build System

* Status: `Accepted`
* Date: `2026-07-04`

## Context

`Project Phoenix` cần nền kỹ thuật đủ mạnh cho editor desktop 3D, có đường dài cho rigging, animation, plugin, AI.

Beta gần nhất cần:

* app desktop
* viewport 3D
* import `FBX`
* scene nội bộ
* mở rộng về sau không vỡ nền

Ta cần chốt stack sớm để:

* tránh đổi toolchain giữa chừng
* thống nhất build/dev env
* làm nền cho module boundary

## Decision

Chọn stack lõi:

* `C++20/23` cho core application
* `CMake` cho build system
* `Qt 6` cho desktop application framework
* `OpenGL` cho rendering backend đầu tiên
* `GLM` cho math
* `spdlog` cho logging
* `GoogleTest` cho unit/integration test mức native

## Consequences

Tốt:

* `C++` phù hợp editor native 3D, memory/perf control tốt
* `CMake` phổ biến, hợp toolchain đa nền tảng
* `Qt 6` mạnh cho desktop UI, input, file dialog, event loop
* `OpenGL` đưa beta lên nhanh hơn `Vulkan`
* `GLM` giảm chi phí tự viết math lib

Đổi lại:

* build native phức tạp hơn app script-based
* `Qt` thêm chi phí học + deploy
* `OpenGL` không phải backend cuối cùng dài hạn

## Alternatives Considered

### C# + WPF/WinUI

Không chọn:

* nhanh cho desktop UI
* yếu hơn cho portable 3D/editor stack mục tiêu
* lệch định hướng cross-platform

### C++ + Dear ImGui

Không chọn làm shell chính:

* nhanh cho internal tool
* không lý tưởng cho editor sản phẩm hóa dài hạn
* yếu hơn về desktop UX framework tổng thể

### Vulkan ngay từ đầu

Không chọn cho beta:

* đúng hướng dài hạn
* chi phí beta quá cao
* làm chậm vòng kiểm chứng viewport/import

