# PROJECT_CONTEXT.md

Tài liệu này tổng hợp từ đọc trực tiếp mã nguồn — không phải tài liệu.  
Cập nhật: 2026-08-11 (phiên 3).

---

## Tổng quan

**Phoenix Editor Beta** — phần mềm hoạt hình 3D desktop, C++20, Qt6, OpenGL 3.3 Core.  
Build system: CMake 3.21+, vcpkg (Windows x64), hai cấu hình: `ninja-msvc-debug` và `msvc2022-debug`.

Tên thực thi: `phoenix_editor`.  
Ngôn ngữ script giả Maya MEL (subset) — người dùng gõ lệnh vào Script Editor.

---

## Cấu trúc thư mục nguồn

```
E:/Animation Software/
├── apps/editor/          # Entry point + tất cả UI panel (Qt Widgets)
│   ├── main.cpp
│   ├── include/          # Headers cho các panel và controller UI
│   └── src/              # Implementations
├── src/
│   ├── core/
│   │   ├── app/          # EditorShell, EditorShellContexts, WorkspaceManager,
│   │   │                 # EditorMenuBar, EditorToolBar, EditorTheme
│   │   ├── commands/     # ScriptCommandSystem (MEL-like registry)
│   │   ├── history/      # EditorHistoryController (undo/redo)
│   │   ├── logging/      # LogCategories (Qt logging categories)
│   │   └── settings/     # EditorPreferences
│   ├── scene/            # Scene graph core (Scene, SceneObject, Transform, MeshData, Bounds3D)
│   ├── animation/
│   │   ├── data/         # ObjectAnimationState, TransformKeyframeTrack
│   │   ├── editor/       # EditorAnimationFlowController, EditorAnimationSceneMutation, EditorAnimationState
│   │   └── scene/        # SceneAnimationState
│   ├── rigging/
│   │   ├── data/         # ObjectRigState
│   │   ├── editor/       # EditorRiggingController
│   │   └── scene/        # SceneRiggingController, JointRiggingOps, SkinBindingOps, SkinDeformationOps
│   ├── engine/
│   │   ├── animation/    # EditorAnimationEngineFacade (aggregates flow + runtime)
│   │   ├── playback/     # EditorPlaybackController (timer-based playback)
│   │   └── runtime/      # EditorSceneRuntimeController, EditorViewportSceneController
│   ├── io/               # FbxImporter (via assimp), PhoenixSceneDocument (.phoenixscene), AssimpSceneDocument
│   ├── rendering/        # ViewportRenderer (OpenGL 3.3), ShaderUtils, RenderTypes
│   │   └── geometry/     # ViewportOverlayGeometryBuilder, gizmo geometry builders
│   └── viewport/
│       ├── EditorCamera.h
│       ├── gizmo/drag/   # TranslateGizmoDrag, RotateGizmoDrag, ScaleGizmoDrag
│       ├── interaction/  # ViewportInteractionMath, gizmo interaction handlers
│       └── runtime/      # ViewportRenderSync
└── tests/unit/
    ├── EditorUiTests.cpp   # Qt Test — integration tests chạy full EditorShell
    └── SceneTransformTests.cpp
```

---

## Scene Graph

**`Scene`** — container trung tâm, không kế thừa QObject.

- Lưu objects trong `QHash<SceneObject::Id, SceneObject>` (Id = `uint64_t`, auto-increment).
- Lưu mesh data trong `QHash<int, MeshData>` (handle = int).
- Sở hữu `SceneAnimationState` (global animation data).
- API chính: `createObject`, `createJoint`, `setLocalTransform`, `setObjectKeyframe`, `reparentObject`, `bindObjectToSkeleton`, `buildDeformedMesh`, `optimizeStorage`, `worldTransform`.

**`SceneObject`** — node trong scene tree.

- Hai loại: `Kind::Transform` (mesh node) và `Kind::Joint` (rig joint).
- Mỗi object mang: `localTransform`, `authoredTransform`, `jointOrientation`, `bindPoseLocalTransform`, `skinJointIds`, `skinWeights`, `meshHandles`, `ObjectAnimationState`, `ObjectRigState`.
- Hierarchy qua `parentId` + `childIds` (flat hash + pointer).

**`Transform`** — struct đơn giản: `translation (QVector3D)`, `rotation (QQuaternion)`, `scale (QVector3D)`.

---

## Kiến trúc EditorShell

`EditorShell : QMainWindow` — God object điều phối mọi thứ.

### Controllers (stateless namespaces / thin classes)

| Controller | Vai trò |
|---|---|
| `EditorAnimationFlowController` | Logic animation state (set frame, set key, auto key, playback range) |
| `EditorAnimationEngineFacade` | Aggregates flow + runtime; bind script commands |
| `EditorAnimationSceneMutation` | Mutate scene cho keyframe edits |
| `EditorPlaybackController` | Timer tick → advance playback |
| `EditorSceneRuntimeController` | Apply scene snapshot + selection + undo |
| `EditorViewportSceneController` | Create primitives, set transforms qua viewport |
| `EditorRiggingController` | Joint create, orient, bind pose, skin bind |
| `SceneRiggingController` | Low-level rig ops trên Scene |
| `EditorCreationController` | Tạo primitive objects |
| `EditorSelectionController` | Manage selected object ID |
| `EditorOutlinerController` | Populate outliner tree |
| `EditorInspectorController` | Update inspector panel |
| `EditorChannelBoxController` | Channel box (transform spinboxes) |
| `EditorDocumentController` | Scene load/save logic |
| `EditorFileFlowController` | File dialogs + error handling |
| `EditorSceneMutationController` | Rename, duplicate, delete, group, reparent |
| `EditorSceneQueryController` | Query scene by name |
| `EditorViewportCommandController` | Viewport toolbar commands |
| `EditorViewportUiController` | Viewport UI state |
| `EditorScriptExecutionController` | Execute script lines |
| `EditorScriptLogController` | Log script results |
| `EditorHistoryController` | Undo/redo stack (clone Scene snapshots) |

**Pattern chung**: Controller nhận `Context` struct (bundle of `std::function<>`) thay vì pointer trực tiếp vào EditorShell → testable không cần shell.

```cpp
// Ví dụ pattern Context:
struct Context {
    std::function<Scene()> sceneSnapshot;
    std::function<const SceneObject*(uint64_t)> findObject;
};
OperationResult setKeyForSelection(const Context& ctx, ...);
```

### Frontend Modules (tách ra từ EditorShell — 2026-08-11)

| Module | Loại | Vai trò |
|---|---|---|
| `EditorTheme` | namespace | Stylesheet tập trung (~80 dòng CSS). `apply(QWidget*)`, `styleSheet()` |
| `EditorMenuBar` | `QObject` | Build và sở hữu toàn bộ menu bar (8 menu, 50+ QAction). `Actions` struct để truy cập QAction*, `Bindings` struct nhận `std::function<>` từ EditorShell |
| `EditorToolBar` | namespace | `build(QMainWindow*, const Actions&)` → tạo toolbar từ action pointers có sẵn |

**Pattern**: `EditorMenuBar` nhận `Bindings` (bundle of `std::function<>`) → không phụ thuộc trực tiếp vào EditorShell pointer, song song với WorkspaceManager pattern.

```cpp
editorMenuBar_ = new EditorMenuBar(this, this);
editorMenuBar_->build(EditorMenuBar::Bindings { .newScene = [this]() { newScene(); }, ... });
toolbar_ = EditorToolBar::build(this, editorMenuBar_->actions());
```

EditorShell truy cập actions tại runtime qua: `editorMenuBar_->actions().xxx`

---

### WorkspaceManager

`WorkspaceManager : QObject` — wrap `QDockWidget` engine của Qt.

- Ba preset built-in: `Default`, `Animation`, `Rigging`.
- Panel IDs: `Outliner`, `Inspector`, `PolygonPrimitives`, `ScriptEditor`, `TimeSlider`, `RangeSlider`, `CommandLine`, `GraphEditor`, `RigPanel`.
- Lưu layout qua `QSettings`. User layouts: placeholder chưa implement đầy đủ.
- `compactTitleBar` flag: ẩn title bar khi docked, show minimal khi floating.

### Panels UI (apps/editor/)

| Panel | Widget |
|---|---|
| Viewport | `ViewportWorkspaceWidget` (container) + `ViewportWidget` (QOpenGLWidget) |
| Animation Timeline | `AnimationTimelinePanel` + `KeyframeTimelineWidget` |
| Range Slider | `RangeSliderPanel` |
| Graph Editor | `GraphEditorPanel` |
| Outliner | `QTreeWidget` (inline trong EditorShell) |
| Inspector / Channel Box | `QDoubleSpinBox` x9, `QCheckBox` visibility |
| Script Editor | `QPlainTextEdit` x2 (history + input) |

---

## Rendering Pipeline

**OpenGL 3.3 Core Profile**, double-buffered.

`ViewportRenderer` — low-level GL renderer:
- Nhiều VAO riêng biệt: scene geometry, imported mesh, preview mesh, joints, selection bounds, gizmos.
- `syncScene(ViewportRenderSceneData)` — upload geometry từ scene snapshot.
- `setGizmo(...)` — upload gizmo vertices (Translate/Rotate/Scale).
- `setPreviewScene(...)` — preview khi import trước khi commit.
- Shader: 1 shader program dùng chung, phân biệt draw calls.

`EditorCamera`:
- Orbit camera: yaw/pitch/distance/target.
- Presets: Perspective, Front, Back, Left, Right, Top, Bottom.
- Supports perspective + orthographic projection.
- `frameBounds(center, radius)` — focus camera vào AABB.

Gizmo geometry builders tách riêng: `TranslateGizmoGeometry`, `RotateGizmoGeometry`, `ScaleGizmoGeometry`.  
Gizmo interaction tách riêng: `TranslateGizmoInteraction`, `RotateGizmoInteraction`, `ScaleGizmoInteraction`.  
Gizmo drag logic: `TranslateGizmoDrag`, `RotateGizmoDrag`, `ScaleGizmoDrag`.

`ViewportRenderSync` — sync scene state → renderer trước mỗi frame.

---

## Animation System

**`EditorAnimationState`** — plain struct (không phải QObject):
```cpp
int currentFrame, visibleStartFrame, visibleEndFrame;
int playbackStartFrame, playbackEndFrame, framesPerSecond;
bool autoKeyEnabled, playing, hasSelectedRange;
int selectedRangeStartFrame, selectedRangeEndFrame;
```

**Keyframe storage**: `TransformKeyframeTrack` trong mỗi `ObjectAnimationState` (per-object).  
Keyframe = `(int frame → Transform)` map.

**Playback**: `EditorPlaybackController` bind vào `EditorAnimationFlowController::advancePlayback` qua timer.

**Auto Key**: khi bật, mọi transform change tự động set keyframe tại `currentFrame`.

**Script bindings**: `EditorAnimationEngineFacade::bindScriptCommands` inject các lambda vào `ScriptCommandContext`.

---

## Rigging System

- **Joint**: `SceneObject` với `Kind::Joint`. Có `jointOrientation (QQuaternion)` riêng.
- **Bind Pose**: captured per-joint (`bindPoseLocalTransform`, `hasBindPose`).
- **Skin Binding**: `skinJointIds` + `SkinWeightTable` stored per mesh object.
- **Deformation**: `Scene::buildDeformedMesh` → `SkinDeformationOps` tính linear blend skinning.

`SceneRiggingController` namespace: pure functions nhận `Scene&`, không state.

---

## I/O

| Format | Đọc | Ghi |
|---|---|---|
| `.phoenixscene` | `PhoenixSceneDocument::loadFromFile` | `PhoenixSceneDocument::saveToFile` |
| `.fbx` | `FbxImporter` (qua assimp) | — |
| Assimp generic | `AssimpSceneDocument` | — |

FBX pipeline: `FbxImporter` → `FbxSceneBuilder` → `FbxImportValidation` → `FbxImportMessages` → `FbxImportResult`.

---

## Script Command System (MEL-like)

`ScriptCommandRegistry::execute(commandLine, context)` — parse và dispatch.

Lệnh hỗ trợ (từ tests):
```
select -cl;
polyCube -w 1 -h 1 -d 1;
setAttr "pCube1.translate" 3.0 0.0 0.0;
playbackOptions -min 5 -max 48;
currentTime 12;
play -state on|off;
rename pCube1 heroCube;
duplicate heroCube;
group heroCubeCopy;
delete heroCube;
setToolTo RotateSuperContext;
viewFit pCube1;
viewSet -home;
file -f -new;
file -save "path";
file -o "path";
file -import "path.fbx";
setKeyframe pCube1 -t 12;
cutKey pCube1 -t 12;
copyKey pCube1 -t 10 -to 11;
shiftKey pCube1 -by -1;
autoKeyframe -state on|off;
joint -name shoulder_jnt;
parent wrist_jnt elbow_jnt;
unparent wrist_jnt;
jointOrient shoulder_jnt -euler 0 45 90;
jointOrient -reset shoulder_jnt;
jointOrient shoulder_jnt -alignToChild;
bindPose -capture -recursive shoulder_jnt;
bindSkin pCube1 root_jnt;
```

---

## Tests

**Framework**: Qt Test (`QTest`), một file `EditorUiTests.cpp`.

**Loại test**: Integration tests — tạo full `EditorShell`, trigger `QAction`, verify outliner/channel box state.  
Một số test là pure unit test trên controller namespace (không cần shell).

Tên test nói lên rõ scope:
- `editMenuUndoRedoRestoresCreatedPrimitive`
- `playbackControllerRoutesTimeIntentsAndTicks`
- `animationEngineFacadeAppliesKeyEditsAndScriptBindings`
- `sceneRuntimeControllerAppliesSceneFrameAndSelection`
- `viewportSceneControllerRoutesSceneMutations`
- `hierarchyActionsParentAndUnparentJoints`
- `timelineUiShowsKeyframeFeedback`
- `timelineUiSupportsFrameRangeSelection`

Build: `tests/unit/CMakeLists.txt` → target `editor_ui_tests`.

---

## Graph Editor — ✅ Đã implement (phiên 3)

**Trạng thái**: Hoàn thành tất cả 4 phase. `GraphEditorPanel` từ viewer read-only → editor đầy đủ.

### Files mới

| File | Vai trò |
|---|---|
| `src/animation/data/KeyTangent.h` | `TangentMode` enum (Auto/Linear/Flat/Stepped/Broken), `KeyTangent` struct (inAngle, outAngle, inWeight, outWeight) |
| `src/animation/data/CurveInterpolator.h/.cpp` | `evaluateChannel(keys, frame)` — cubic bezier với binary search; `computeAutoTangent` (Catmull-Rom) |

### Files đã sửa

| File | Thay đổi |
|---|---|
| `src/animation/data/TransformKeyframeTrack.h` | Thêm `KeyTangent tangent` vào `TransformKeyframe` |
| `src/animation/scene/SceneAnimationState.cpp` | `evaluateObjectTransformAtFrame()` dùng `CurveInterpolator::evaluateChannel()` cho 9 channel (tx/ty/tz/rx/ry/rz/sx/sy/sz) thay vì linear lerp |
| `apps/editor/include/GraphEditorPanel.h` | Thêm `GraphEditorKeyEdit`, `TangentEdit`, `TangentModeChange` structs; drag/tangent state; 3 callback setters; `fireKeyEditForTest()`; **không dùng Q_OBJECT** (std::function callbacks đủ dùng) |
| `apps/editor/src/GraphEditorPanel.cpp` | Phase 2: key drag → `keyEditedCallback_`; Phase 3: `cubicTo()` bezier draw + tangent handle 48px; Phase 4: context menu 5 mode + toolbar Frame All/Selected/Show Tangents |
| `src/core/app/EditorShell.h/.cpp` | `handleGraphEditorKeyEdited/TangentEdited/TangentModeChanged` — scene snapshot → patch → `applyScene(recordUndo=true)` |
| `apps/editor/CMakeLists.txt` | Thêm `CurveInterpolator.cpp` |
| `tests/unit/CMakeLists.txt` | Thêm `CurveInterpolator.cpp` + `EditorMenuBar/Theme/ToolBar.cpp` cho `editor_ui_tests` |

### 5 test methods (tất cả PASS)

```
testGraphEditorPanelPopulation    — 9 curves xuất hiện trong QListWidget
testGraphEditorCurrentFrameSync   — callback gọi không crash
testGraphEditorCurveVisibility    — xóa selection không crash
testGraphEditorKeyEdit            — tạo cube, set 2 keyframe, fire key edit, verify undo available
testGraphEditorInterpolationMode  — 5 TangentMode, vẽ không crash
```

### Điểm kỹ thuật quan trọng

- **AUTOMOC gotcha**: `Q_OBJECT` trong header thuộc `.cpp` của target khác → AUTOMOC non-inline mode không pick up được nếu header không được scan transitively từ header đã known. Giải pháp: bỏ `Q_OBJECT` khỏi `GraphEditorPanel`/`GraphEditorCanvasWidget` (dùng `std::function` callback thay signal/slot), dùng `setObjectName("graphEditorPanel")` + `findChild<QWidget*>(name)` + `static_cast` trong test.
- **Euler rotation per-channel**: `SceneAnimationState` dùng `QQuaternion::toEulerAngles()` để build 3 CurveKey array cho rx/ry/rz, eval riêng, rồi `QQuaternion::fromEulerAngles()` để tái tạo rotation.
- **Bezier control points**: `weight` là fraction của segment length (0–1), `controlOffset = weight * segLen * tan(angleDeg)` theo trục frame/value đã normalize.

---

## Dependencies (vcpkg)

| Thư viện | Dùng cho |
|---|---|
| Qt6 (Widgets, OpenGLWidgets, OpenGL, Gui) | Toàn bộ UI + rendering context |
| assimp | FBX + mesh import |
| poly2tri | Polygon triangulation |
| draco | Mesh compression (có trong vcpkg, chưa rõ dùng trực tiếp chưa) |

---

## Trạng thái hiện tại (branch: `codex/core-scene-step1-seams`)

Commit gần nhất: `958f267 — docs: update PROJECT_CONTEXT with Graph Editor plan`.

**Đã implement trong phiên 3** (chưa commit vào thời điểm cập nhật):
- Graph Editor 4 phase hoàn chỉnh (xem section trên)
- `editor_ui_tests`: 35 PASS, 2 FAIL (pre-existing: `transformToolbarUpdatesViewportToolState`, `viewMenuSwitchesViewportCameraPresets` — không liên quan Graph Editor)

Hướng tiếp theo: UI Redesign Plan (4-step) theo memory `[[ui-redesign-decision]]`.

---

## Điểm cần chú ý khi làm việc

1. **EditorShell rất lớn** (~350+ dòng header, 1 class làm hết). Đang có kế hoạch refactor → tách dần theo WorkspaceManager pattern.
2. **Controller pattern**: luôn dùng `Context` struct với `std::function` — không pass `EditorShell*` thẳng vào controller.
3. **Scene không phải QObject** — không có signals, không thread-safe theo Qt convention. Copy by value khi cần snapshot (undo).
4. **Undo**: clone toàn bộ `Scene` + `EditorAnimationState` → lưu vào `EditorHistoryController`. Không có fine-grained command pattern.
5. **Script log**: mọi thao tác người dùng đều sinh ra `commentLine` + `commandLine` + `resultLine` → hiển thị trong Script Editor history (Maya-style).
6. **Native file format**: `.phoenixscene` — custom format, không phải USD/glTF.
7. **Tests chạy headless Qt**: cần display server trên CI nếu cần.
