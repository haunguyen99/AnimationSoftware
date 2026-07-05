# Project Phoenix Technical Specification v1

## 1. Purpose

Tài liệu này chuyển đổi bản overview sản phẩm thành đặc tả kỹ thuật mức nền tảng cho `Project Phoenix`.

Mục tiêu tài liệu:

* Xác định phạm vi `v1`.
* Chốt kiến trúc kỹ thuật ban đầu.
* Định nghĩa module, dữ liệu, luồng chính.
* Làm nền cho implementation, test, plugin, AI sau này.

---

## 2. Product Goal

`Project Phoenix` là phần mềm `Rigging + Animation` hiện đại, ưu tiên workflow cho `Game Animator`, chạy trên `PC` và mở đường cho `Tablet + Pen`.

Giá trị cốt lõi:

* Tập trung `Animation`, không ôm toàn bộ DCC pipeline.
* Core gọn, dễ bảo trì, dễ mở rộng.
* Plugin là cơ chế mở rộng chính.
* Kiến trúc sẵn đường cho `AI`, `Python`, `Cloud`.

Không thuộc phạm vi v1:

* Modeling
* Sculpting
* UV Editing
* Material/Shading authoring
* Final Rendering pipeline
* Marketplace
* Real-time collaboration

---

## 3. Release Scope

## 3.1 Scope v1 / Phase 1

`v1` phải cho phép animator hoàn thành luồng tối thiểu:

1. Mở project.
2. Import character/skeleton từ `FBX`.
3. Xem skeleton trong viewport.
4. Chọn joint/control cơ bản.
5. Tạo keyframe transform.
6. Scrub timeline và playback animation.
7. Save/load project.
8. Export animation hoặc scene data ra `FBX`.

## 3.2 Success Criteria

`v1` đạt khi:

* Import được ít nhất 1 character rig đơn giản từ `FBX`.
* Hiển thị skeleton ổn định trong viewport.
* Cho phép tạo/sửa/xóa keyframe transform cơ bản.
* Playback đúng theo timeline.
* Lưu lại project rồi mở lại không mất dữ liệu animation chính.
* Có khung plugin hoạt động mức cơ bản.

## 3.3 Deferred to v2+

Các hạng mục sau chưa bắt buộc trong `v1`:

* IK/FK solve hoàn chỉnh
* Constraint system
* Skinning toolset
* Graph Editor đầy đủ
* Python scripting public API
* AI assistant
* Cloud sync

---

## 4. Target Users

## 4.1 Primary Users

* `Game Animator`
* `Technical Animator`
* `Indie Game Developer`

## 4.2 Secondary Users

* Small/medium game studio
* Animation training school

## 4.3 Primary User Needs

* Mở scene nhanh.
* Playback mượt.
* Chọn rig/skeleton rõ ràng.
* Keyframe thao tác ít click.
* File project ổn định, dễ recover.
* Kiến trúc đủ mở để studio thêm tool riêng.

---

## 5. Technical Principles

* `Core first`: logic lõi không phụ thuộc plugin.
* `Data-oriented enough`: dữ liệu animation phải dễ evaluate, serialize, cache.
* `Plugin-safe boundaries`: extension không được làm hỏng core.
* `Viewport responsive`: tương tác phải ưu tiên độ mượt.
* `Docs as source of truth`: mọi module mới phải có contract rõ.
* `Portable architecture`: tránh khóa cứng vào `OpenGL` trong domain layer.

---

## 6. High-Level Architecture

Hệ thống chia thành 6 lớp:

1. `Application Layer`
2. `Editor UI Layer`
3. `Domain/Core Layer`
4. `Runtime Evaluation Layer`
5. `Infrastructure Layer`
6. `Extension Layer`

## 6.1 Application Layer

Chịu trách nhiệm:

* App lifecycle
* Project lifecycle
* Command routing
* Undo/redo orchestration
* Module bootstrapping

Thành phần:

* `Application`
* `ProjectManager`
* `DocumentController`
* `CommandDispatcher`

## 6.2 Editor UI Layer

Tech: `Qt 6 + QML`

Thành phần:

* Main window
* Dock panels
* Timeline panel
* Outliner
* Inspector
* Viewport host

Nguyên tắc:

* UI không giữ source of truth cho scene/animation data.
* UI bind vào view model hoặc controller interface.
* Domain logic không viết trực tiếp trong QML.

## 6.3 Domain/Core Layer

Chứa business logic chính:

* Scene graph
* Node/transform
* Skeleton
* Animation data
* Selection
* Playback state
* Project schema

Không phụ thuộc:

* Qt UI
* OpenGL renderer cụ thể
* Python runtime

## 6.4 Runtime Evaluation Layer

Chịu trách nhiệm:

* Evaluate transform hierarchy
* Sample animation curves/clips
* Resolve playback state theo time
* Chuẩn bị data cho viewport render

## 6.5 Infrastructure Layer

Gồm:

* File I/O
* FBX import/export adapter
* Logging
* Config
* Asset path handling
* Serialization

## 6.6 Extension Layer

Bao gồm:

* Plugin loader
* Plugin registration
* Event hooks
* Future Python bindings

---

## 7. Module Breakdown

## 7.1 Core Engine

Vai trò:

* Nền tảng app/editor runtime.
* Quản lý service registry, lifecycle, command bus, update tick.

Trách nhiệm:

* Boot modules
* Register services
* Dispatch app events
* Manage undo/redo stack

Không chứa:

* Logic render API cụ thể
* QML view logic

## 7.2 Scene Graph

Vai trò:

* Đại diện cấu trúc object trong scene.

Entity chính:

* `Scene`
* `Node`
* `TransformNode`
* `Skeleton`
* `Joint`

Yêu cầu:

* Hỗ trợ quan hệ parent/child.
* Có id ổn định để serialize.
* Dễ query từ viewport, outliner, animation system.

## 7.3 Viewport

Vai trò:

* Hiển thị scene 3D.
* Cho tương tác camera, selection, playback visualization.

Yêu cầu v1:

* Orbit/pan/zoom camera
* Draw skeleton/joints
* Draw grid
* Highlight selection
* Play animation trong viewport

Thiết kế:

* Render abstraction bọc `OpenGL`.
* Public interface không khóa cứng vào OpenGL để dễ chuyển `Vulkan`.

## 7.4 Rigging

Trong `v1`, module này giới hạn:

* Biểu diễn skeleton/joint hierarchy
* Import rig structure từ `FBX`
* Chỉnh transform cơ bản cho joint/control nếu có

Chưa bắt buộc:

* Full rig authoring
* Constraint graph
* Solver stack

## 7.5 Animation

Vai trò:

* Lưu keyframe data
* Quản lý animation clip
* Sample transform theo time

Entity chính:

* `AnimationClip`
* `AnimationTrack`
* `Keyframe`
* `Channel`

Channel v1:

* Translation
* Rotation
* Scale

## 7.6 Timeline

Vai trò:

* Điều hướng thời gian.
* Hiển thị keyframe mức cơ bản.

Yêu cầu v1:

* Current frame indicator
* Frame range
* Scrub
* Play/pause/stop
* Set key
* Delete key

## 7.7 Graph Editor

Không thuộc phạm vi hoàn chỉnh của `v1`.

Khuyến nghị:

* Chừa interface cho curve data ngay từ đầu.
* Timeline dùng cùng nguồn dữ liệu với graph editor tương lai.

## 7.8 Plugin SDK

Mục tiêu `v1`:

* Nạp plugin native cơ bản.
* Cho plugin đăng ký menu action/tool command.
* Cho plugin nghe event mức app/project.

Chưa cần ở `v1`:

* Full sandbox
* Marketplace
* Hot reload phức tạp
* ABI stability tuyệt đối

---

## 8. Proposed Directory Architecture

```text
Phoenix/
  apps/
    editor/
  src/
    engine/
    app/
    scene/
    animation/
    rigging/
    timeline/
    viewport/
    rendering/
    io/
    plugins/
    commands/
    serialization/
    util/
  ui/
    qml/
  include/
  tests/
    unit/
    integration/
  docs/
    adr/
  third_party/
  tools/
```

Nguyên tắc:

* `src/scene`, `src/animation` giữ domain logic.
* `src/rendering` giữ graphics abstraction.
* `src/io` chứa `FBX`, file format, asset import/export.
* `ui/qml` chỉ cho presentation layer.

---

## 9. Data Model

## 9.1 Scene Model

`Scene` gồm:

* Scene metadata
* Node collection
* Hierarchy relationships
* Animation bindings
* Asset references

Node fields tối thiểu:

* `NodeId`
* `name`
* `type`
* `parentId`
* `localTransform`
* `visibility`

## 9.2 Transform Model

`Transform` gồm:

* translation `vec3`
* rotation `quat`
* scale `vec3`

Yêu cầu:

* Internal rotation dùng `quaternion`.
* UI có thể hiển thị `Euler` khi cần.
* Evaluation pipeline tránh gimbal lock ở domain level.

## 9.3 Skeleton Model

`Skeleton` gồm:

* danh sách joints
* bind/rest transform
* parent hierarchy

Joint fields:

* `JointId`
* `name`
* `parentJointId`
* `restTransform`
* `animatedNodeId`

## 9.4 Animation Model

`AnimationClip`:

* `clipId`
* `name`
* `startFrame`
* `endFrame`
* `frameRate`
* collection of tracks

`AnimationTrack`:

* `targetNodeId`
* `channels`

`Channel`:

* `channelType`
* ordered keyframes

`Keyframe`:

* `frame`
* `value`
* interpolation mode

Interpolation v1:

* `Step`
* `Linear`

Thiết kế mở:

* Chừa chỗ cho `Bezier/Hermite` ở v2.

---

## 10. File Formats

## 10.1 Project File

Đề xuất:

* Dùng format text-readable cho `v1`, ưu tiên `JSON` hoặc `YAML`.
* Đuôi file gợi ý: `.phoenix` hoặc `.phx`.

Nội dung:

* project metadata
* scene hierarchy
* animation clips
* asset references
* editor state tối thiểu

Lý do:

* Dễ debug
* Dễ diff
* Dễ migrate schema giai đoạn đầu

## 10.2 Asset Import/Export

`FBX` dùng cho:

* character import
* skeleton import
* animation export

Lưu ý:

* Tách `internal scene format` khỏi `FBX`.
* Không để domain layer phụ thuộc trực tiếp SDK importer.

## 10.3 Versioning

Mỗi project file phải có:

* `schemaVersion`
* `appVersion`

Yêu cầu:

* Có migration path tối thiểu giữa minor schema changes.

---

## 11. Editor Workflow

## 11.1 Main Workflow v1

1. User tạo project mới hoặc mở project.
2. User import `FBX`.
3. App sinh scene nodes + skeleton + asset refs.
4. User chọn joint/control trong outliner hoặc viewport.
5. User chỉnh transform.
6. User bấm `Set Key`.
7. Timeline lưu keyframe vào track tương ứng.
8. Playback engine evaluate clip theo current frame.
9. Viewport nhận pose đã evaluate và render.
10. User save project.

## 11.2 Selection Workflow

User có thể chọn bằng:

* Outliner
* Viewport picking

Selection service phải là shared source cho:

* Inspector
* Timeline context
* Gizmo
* Future graph editor

## 11.3 Playback Workflow

Playback loop:

1. Timeline cập nhật current time.
2. Animation system sample tracks.
3. Runtime evaluation resolve local transform.
4. Scene graph build world transform.
5. Viewport render pose mới.

---

## 12. UI Specification v1

## 12.1 Main Panels

`v1` nên có:

* `Viewport`
* `Outliner`
* `Inspector`
* `Timeline`
* `Asset/Project panel` mức tối thiểu

## 12.2 Main Commands

* New Project
* Open Project
* Save Project
* Import FBX
* Export FBX
* Undo
* Redo
* Play
* Pause
* Stop
* Set Key
* Delete Key

## 12.3 Tablet/Pen Readiness

Chưa cần full tablet UX trong `v1`, nhưng kiến trúc UI phải:

* Không phụ thuộc hover-only interaction.
* Cho phép binding pen input sau này.
* Giữ hit target đủ lớn ở control quan trọng.

---

## 13. Rendering Architecture

## 13.1 v1 Rendering Backend

Backend đầu tiên:

* `OpenGL`

Yêu cầu:

* Có lớp `RenderDevice` hoặc tương đương.
* Viewport chỉ giao tiếp qua abstraction layer.

## 13.2 Future Migration

Thiết kế phải cho phép:

* `OpenGL` -> `Vulkan`

Điều kiện:

* Render command/model data tách khỏi API binding.
* Shader/resource management không rò sang domain layer.

## 13.3 Viewport Render Features v1

* Grid
* Joint lines/bones
* Selection highlight
* Basic transform gizmo
* Camera controls

---

## 14. Plugin Architecture

## 14.1 Plugin Goals

Plugin system cho phép mở rộng:

* Tools
* Import/export handlers
* Menu/command integration
* Future scripting bridge

## 14.2 Plugin Types

`v1` đề xuất 3 loại:

* `EditorPlugin`
* `ImportExportPlugin`
* `ToolPlugin`

## 14.3 Plugin Contract

Mỗi plugin nên cung cấp:

* `name`
* `version`
* `apiVersion`
* `initialize()`
* `shutdown()`

Plugin có thể đăng ký:

* commands
* menu items
* project hooks
* scene import/export capability

## 14.4 Safety Rules

* Plugin crash không được làm hỏng project file.
* Error plugin phải được log rõ.
* Plugin API public phải nhỏ gọn trong `v1`.

---

## 15. Undo/Redo Model

Undo/redo là bắt buộc ngay từ `v1`.

Áp dụng cho:

* transform edits
* keyframe create/delete
* scene selection command nếu cần
* import action ở mức transaction lớn

Thiết kế:

* Dùng command pattern.
* Command phải serializable ở mức debug/log nếu có thể.

---

## 16. Performance Requirements

## 16.1 Startup

* Mở app ở mức chấp nhận được trên máy dev phổ thông.

## 16.2 Viewport

Mục tiêu:

* Tương tác viewport mượt với scene rig đơn giản đến trung bình.

## 16.3 Playback

Mục tiêu:

* Playback ổn định cho clip ngắn/trung bình trong `v1`.

## 16.4 Memory

Yêu cầu:

* Không duplicate dữ liệu scene/animation vô ích giữa UI và core.

---

## 17. Reliability Requirements

* Save project phải atomic hoặc gần atomic.
* Crash lúc import/save không được làm hỏng file cũ.
* Logging phải đủ trace lỗi import, serialize, plugin load.
* App phải chịu được file `FBX` không hợp lệ bằng error rõ ràng.

---

## 18. Logging and Diagnostics

Tech:

* `spdlog`

Log categories đề xuất:

* `app`
* `project`
* `io`
* `fbx`
* `scene`
* `animation`
* `viewport`
* `plugin`

Build `Debug` nên có:

* verbose logs
* assertion mạnh cho invalid state

---

## 19. Testing Strategy

## 19.1 Unit Tests

Test cho:

* transform math
* hierarchy evaluation
* animation sampling
* serialization/deserialization
* command undo/redo

## 19.2 Integration Tests

Test cho:

* import `FBX` -> scene model
* save -> load project consistency
* playback updates viewport data pipeline

## 19.3 Manual QA Scenarios

Ít nhất phải test:

1. Import character rồi playback.
2. Set key nhiều frame khác nhau.
3. Save/load rồi verify pose/keys.
4. Plugin load fail rồi app vẫn chạy.

---

## 20. Security and Safety Boundaries

Trong `v1`, security không phải trọng tâm sản phẩm internet-facing, nhưng cần:

* Không execute plugin không rõ nguồn nếu user chưa bật.
* Validate đường dẫn file nhập/xuất.
* Tách plugin API khỏi internal mutable state nguy hiểm.

---

## 21. Milestone Plan

## 21.1 Milestone A - Foundation

* App bootstrap
* Module boundaries
* Empty scene
* Basic viewport
* Logging
* Project shell

## 21.2 Milestone B - Scene and Skeleton

* Scene graph
* Skeleton model
* Outliner
* Selection system

## 21.3 Milestone C - FBX Pipeline

* FBX import adapter
* Import to scene graph
* Basic export path

## 21.4 Milestone D - Animation Core

* Timeline state
* Animation clip/track/keyframe
* Set key
* Playback evaluation

## 21.5 Milestone E - Persistence

* Save/load project
* Schema versioning
* Recovery-safe writes

## 21.6 Milestone F - Plugin Base

* Plugin loader
* Register command/menu
* Sample plugin

---

## 22. Open Questions

Các điểm cần quyết sớm:

* Dùng thư viện nào cho `FBX import/export`?
* `JSON` hay `YAML` cho project format?
* Có dùng `entity-component` style hay tree-object model thuần?
* Cần multi-document hay single-document editor trong `v1`?
* Gizmo transform tự viết hay dùng giải pháp sẵn?
* Plugin ABI native có khóa theo compiler/version không?

---

## 23. Recommended Decisions

Đề xuất ban đầu:

* `single project window` cho `v1`
* `JSON` cho project file
* `object/tree scene model` trước, chưa cần ECS
* `OpenGL abstraction` ngay từ đầu
* `quaternion internal rotation`
* `plugin API nhỏ`, chưa hứa ABI ổn định dài hạn

---

## 24. Definition of Done for v1

`v1` hoàn thành khi:

* User import được rig/skeleton từ `FBX`.
* User set key transform, scrub, playback được.
* Save/load project giữ được scene + animation chính.
* Export `FBX` hoạt động ở mức cơ bản.
* Viewport, outliner, inspector, timeline phối hợp ổn định.
* Có plugin mẫu chứng minh extension path thật sự chạy.

---

## 25. Next Documents Recommended

Sau tài liệu này nên có thêm:

* `docs/adr/001-scene-graph-model.md`
* `docs/adr/002-project-file-format.md`
* `docs/adr/003-render-abstraction.md`
* `docs/adr/004-plugin-api-boundary.md`
* `docs/implementation-plan-phase1.md`

