# ADR-005: Rendering Starts with OpenGL Behind a Small Abstraction Layer

* Status: `Accepted`
* Date: `2026-07-04`

## Context

Project vision ghi `OpenGL -> Vulkan`. Beta cần viewport nhanh:

* render grid
* render imported mesh
* orbit/pan/zoom

Nếu viết abstraction quá lớn từ đầu:

* chậm beta
* design sớm dễ sai

Nếu code thẳng toàn bộ vào `OpenGL`:

* khó chuyển backend sau này
* renderer API rò sang phần còn lại

## Decision

Cho beta:

* render backend đầu tiên là `OpenGL`
* đặt sau `small abstraction layer`
* abstraction chỉ bọc phần thật sự cần cho beta

Abstraction ban đầu có thể gồm:

* device/context init
* mesh upload
* shader program binding
* draw submission
* viewport resize handling

Không thiết kế render graph/phức hệ sớm.

## Consequences

Tốt:

* ship viewport nhanh
* không khóa toàn app vào OpenGL calls
* đủ khoảng trống để chuyển `Vulkan` sau này

Đổi lại:

* abstraction đầu sẽ mỏng, chưa đẹp hoàn hảo
* khi scope lớn hơn có thể phải refactor renderer layer

## Alternatives Considered

### OpenGL trực tiếp mọi nơi

Không chọn:

* nhanh lúc đầu
* nhưng tạo coupling mạnh

### Full renderer abstraction ngay từ đầu

Không chọn:

* lý tưởng hóa sớm
* beta chưa đủ thông tin để thiết kế đúng

