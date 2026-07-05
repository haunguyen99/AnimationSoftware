# ADR-006: FBX Import Uses Assimp for Beta

* Status: `Accepted`
* Date: `2026-07-04`

## Context

Beta cần import `FBX` nhanh để render model trong viewport. Ta cần chọn thư viện import phù hợp.

Tiêu chí:

* tích hợp nhanh
* hỗ trợ `FBX`
* đủ tốt cho mesh + transform + hierarchy
* giảm lock-in vào SDK vendor
* hợp scope beta hơn production-perfect fidelity

## Decision

Cho `v1 beta`, chọn `Assimp` làm thư viện import `FBX`.

`Assimp` chỉ dùng trong `io/import` boundary. Data sau parse phải convert sang internal scene model.

## Consequences

Tốt:

* tích hợp nhanh hơn Autodesk `FBX SDK`
* hỗ trợ nhiều format nếu sau này cần
* phù hợp prototype/beta
* tránh phụ thuộc nặng vào object model vendor

Đổi lại:

* fidelity `FBX` không luôn hoàn hảo bằng SDK vendor
* scene/animation phức tạp có thể cần workaround
* tương lai có thể phải thay bằng `FBX SDK` hoặc importer riêng cho use case sâu hơn

## Alternatives Considered

### Autodesk FBX SDK

Không chọn cho beta:

* mạnh hơn với `FBX`
* nhưng integration nặng hơn
* tăng coupling với vendor SDK

### Viết importer riêng

Không chọn:

* chi phí quá cao
* không hợp giai đoạn beta

