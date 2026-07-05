# ADR-003: Internal Scene Model Uses Minimal Tree-Based Structure for Beta

* Status: `Accepted`
* Date: `2026-07-04`

## Context

Beta cần import `FBX` rồi render model trong viewport. Scene nội bộ phải:

* đủ để chứa object imported
* đủ để tính bounds
* đủ để cấp data cho renderer
* không quá nặng như scene system hoàn chỉnh

Ta cần chọn giữa:

* `tree/object model`
* `entity-component system`
* data model ad-hoc chỉ cho importer

## Decision

Chọn `minimal tree-based scene model` cho beta.

Scene tối thiểu gồm:

* `Scene`
* `SceneObject`
* parent/child relationship
* `localTransform`
* optional `meshRef`
* local/world bounds

Không dựng `ECS` trong beta.

## Consequences

Tốt:

* map tự nhiên từ `FBX` node hierarchy
* dễ debug
* dễ fit camera theo bounds
* ít abstraction hơn, ship nhanh hơn

Đổi lại:

* không tối ưu nhất cho scene cực lớn
* sau này nếu cần ECS cho runtime phức tạp, sẽ cần layer khác

## Alternatives Considered

### ECS ngay từ đầu

Không chọn:

* mạnh cho scale lớn
* nhưng overkill cho beta
* tăng complexity importer, transform propagation, debug

### Flat object list

Không chọn:

* đơn giản hơn nữa
* nhưng mất hierarchy tự nhiên từ `FBX`
* khó mở rộng sang rig/skeleton sau này

