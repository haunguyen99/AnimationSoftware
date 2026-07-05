# ADR-004: FBX Is External Exchange Format, Internal Scene Remains Canonical

* Status: `Accepted`
* Date: `2026-07-04`

## Context

`FBX` là format bắt buộc để beta import model. Nhưng `FBX` là format trao đổi ngoài hệ thống, không nên thành mô hình dữ liệu lõi của app.

Nếu app giữ data theo cấu trúc importer/vendor SDK:

* domain layer bị khóa vào SDK
* debug khó
* test khó
* save/load nội bộ sau này lệ thuộc `FBX`

## Decision

Chọn nguyên tắc:

* `FBX` chỉ là external exchange format
* sau import, data phải convert sang internal scene model của `Project Phoenix`
* renderer chỉ đọc internal scene/render data, không đọc trực tiếp SDK objects

Importer boundary:

* parse `FBX`
* extract geometry, transforms, hierarchy, bounds
* map sang `Scene` + `SceneObject` + render buffers nội bộ

## Consequences

Tốt:

* core sạch hơn
* test importer tách riêng dễ hơn
* sau này thêm format khác (`glTF`, custom project file) dễ hơn
* save/load nội bộ độc lập với `FBX`

Đổi lại:

* cần thêm bước mapping data
* import path dài hơn một chút

## Alternatives Considered

### Dùng `FBX` SDK objects trực tiếp trong app

Không chọn:

* nhanh cho prototype rất ngắn
* nhưng nợ kỹ thuật lớn
* phá boundary giữa IO và domain

