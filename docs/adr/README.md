# Architecture Decision Records

Bộ `ADR` này ghi lại quyết định kiến trúc cốt lõi cho `Project Phoenix`.

Mục tiêu:

* khóa các quyết định nền trước khi code lớn
* giảm tranh luận lặp lại
* làm source of truth cho team

## Status Key

* `Proposed`
* `Accepted`
* `Superseded`
* `Deprecated`

## ADR Index

* [ADR-001: Core Technology Stack and Build System](</E:/Animation Software/docs/adr/ADR-001-core-technology-stack-and-build-system.md>)
* [ADR-002: Beta Editor Shell Uses Qt Widgets with OpenGL Viewport Host](</E:/Animation Software/docs/adr/ADR-002-beta-editor-shell-uses-qt-widgets-with-opengl-viewport-host.md>)
* [ADR-003: Internal Scene Model Uses Minimal Tree-Based Structure for Beta](</E:/Animation Software/docs/adr/ADR-003-internal-scene-model-uses-minimal-tree-based-structure-for-beta.md>)
* [ADR-004: FBX Is External Exchange Format, Internal Scene Remains Canonical](</E:/Animation Software/docs/adr/ADR-004-fbx-is-external-exchange-format-internal-scene-remains-canonical.md>)
* [ADR-005: Rendering Starts with OpenGL Behind a Small Abstraction Layer](</E:/Animation Software/docs/adr/ADR-005-rendering-starts-with-opengl-behind-a-small-abstraction-layer.md>)
* [ADR-006: FBX Import Uses Assimp for Beta](</E:/Animation Software/docs/adr/ADR-006-fbx-import-uses-assimp-for-beta.md>)
* [ADR-007: Project Uses Feature-Oriented Folder Layout with Shared Core Layers](</E:/Animation Software/docs/adr/ADR-007-project-uses-feature-oriented-folder-layout-with-shared-core-layers.md>)
* [ADR-008: Testing Strategy and Sample Asset Policy](</E:/Animation Software/docs/adr/ADR-008-testing-strategy-and-sample-asset-policy.md>)
* [ADR-009: Joint Orientation Stays Separate From Animated Rotation](</E:/Animation Software/docs/adr/ADR-009-joint-orientation-stays-separate-from-animated-rotation.md>)

## Notes

* Các ADR đầu tiên ưu tiên `v1 beta`: viewport + import `FBX` + orbit/pan/zoom.
* Khi quyết định mới thay thế quyết định cũ, thêm ADR mới. Không sửa lịch sử theo kiểu xóa quyết định cũ.
