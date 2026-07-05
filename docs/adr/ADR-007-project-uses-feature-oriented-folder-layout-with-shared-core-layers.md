# ADR-007: Project Uses Feature-Oriented Folder Layout with Shared Core Layers

* Status: `Accepted`
* Date: `2026-07-04`

## Context

Sau ADR nền, project cần cấu trúc thư mục rõ để:

* code mới đặt đúng chỗ
* tránh trộn UI, render, IO, domain
* scale từ beta sang phase lớn hơn

Ta cần layout đủ nhỏ cho beta nhưng không thành throwaway structure.

## Decision

Project dùng layout này:

```text
apps/
  editor/
src/
  app/
  scene/
  viewport/
  rendering/
  io/
  util/
include/
tests/
  unit/
  integration/
third_party/
assets/
docs/
  adr/
tools/
```

Nguyên tắc:

* `apps/editor` chứa entrypoint app
* `src/app` chứa app shell, actions, orchestration
* `src/scene` chứa internal scene model
* `src/viewport` chứa camera + viewport behavior
* `src/rendering` chứa OpenGL backend + abstraction mỏng
* `src/io` chứa import pipeline
* `include` dành cho public headers khi cần tách rõ

## Consequences

Tốt:

* boundary rõ
* hợp ADR hiện tại
* dễ mở rộng thêm animation/rigging sau beta

Đổi lại:

* hơi nhiều thư mục hơn repo prototype siêu nhỏ
* cần kỷ luật placement file

## Alternatives Considered

### Flat `src/` duy nhất

Không chọn:

* nhanh lúc đầu
* nhưng sớm thành lẫn trách nhiệm

### Tách strict layered package quá sâu

Không chọn:

* đẹp trên giấy
* nhưng nặng cho beta

