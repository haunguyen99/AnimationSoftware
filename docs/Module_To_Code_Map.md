# Module To Code Map

## Purpose

Tai lieu nay map 6 `module` kien truc hien tai cua `Project Phoenix` sang code dang co trong repo.

Muc tieu:

* biet moi `module` dang song o dau
* thay ro cho nao da tach kha sach
* thay ro cho nao van dang tron vai tro
* ho tro planning refactor sau nay

Ngay cap nhat: `2026-07-22`

---

## Target Module Stack

```text
AnimationStudio/
├── Core/
├── Engine/
├── Scene/
├── Rendering/
├── Animation/
├── Rigging/
```

Luu y:

* day la `module map`, khong phai thu muc that 1-1 trong repo
* code hien tai van theo layout da khoa trong `ADR-007`
* vi vay, mot `module` co the dang trai tren nhieu folder

---

## 1. Core

### Current code map

Nang nhat hien tai nam o:

* [apps/editor/main.cpp](</E:/Animation Software/apps/editor/main.cpp>)
* [apps/editor/include/MainWindow.h](</E:/Animation Software/apps/editor/include/MainWindow.h>)
* [apps/editor/src/MainWindow.cpp](</E:/Animation Software/apps/editor/src/MainWindow.cpp>)
* [apps/editor/include/MainWindowContexts.h](</E:/Animation Software/apps/editor/include/MainWindowContexts.h>)
* [apps/editor/src/MainWindowContexts.cpp](</E:/Animation Software/apps/editor/src/MainWindowContexts.cpp>)
* [apps/editor/include/ScriptCommandSystem.h](</E:/Animation Software/apps/editor/include/ScriptCommandSystem.h>)
* [apps/editor/src/ScriptCommandSystem.cpp](</E:/Animation Software/apps/editor/src/ScriptCommandSystem.cpp>)
* [src/logging/LogCategories.h](</E:/Animation Software/src/logging/LogCategories.h>)
* [src/logging/LogCategories.cpp](</E:/Animation Software/src/logging/LogCategories.cpp>)

### Current responsibility in code

`Core` hien dang gom:

* app bootstrap
* `EditorShell` lifecycle
* menu / toolbar / dock wiring
* context and dependency composition
* script command registry
* command routing tu UI vao scene actions
* editor history state cho undo/redo
* logging categories

### Notes

`Core` hien tai chua nam thanh mot folder rieng.
Phan lon dang song trong `apps/editor`, voi `MainWindow` la adapter shell trung tam.

Dieu nay co nghia:

* `Core` da ton tai ve mat `module`
* nhung `interface` hien tai van dang kha rong
* trong docs, nen goi `module` nay la `EditorShell`
* `MainWindow` thuoc `Core/Application`, khong nen duoc doc nhu feature module host

---

## 2. Engine

### Current code map

Hien tai `Engine` chua thanh mot package/doc lap.
No dang phan tan o:

* [apps/editor/include/MainWindow.h](</E:/Animation Software/apps/editor/include/MainWindow.h>)
* [apps/editor/src/MainWindow.cpp](</E:/Animation Software/apps/editor/src/MainWindow.cpp>)
* [apps/editor/include/ViewportWidget.h](</E:/Animation Software/apps/editor/include/ViewportWidget.h>)
* [apps/editor/src/ViewportWidget.cpp](</E:/Animation Software/apps/editor/src/ViewportWidget.cpp>)
* [src/scene/Scene.h](</E:/Animation Software/src/scene/Scene.h>)
* [src/scene/Scene.cpp](</E:/Animation Software/src/scene/Scene.cpp>)

### Current responsibility in code

`Engine` dang xuat hien duoi dang:

* playback tick qua `QTimer`
* current frame coordination
* update loop cho playback
* orchestration giua `Scene`, `Animation`, `Rigging`, `Rendering`
* mutation -> evaluate -> render refresh flow

### Notes

`Engine` la `module` co do tach thap nhat hien tai.

No chua co seam rieng ma dang bi chia giua:

* `MainWindow` cho playback/UI coordination
* `ViewportWidget` cho runtime mutation/render refresh
* `Scene` cho evaluation data

Noi cach khac:

* `Engine` da ton tai trong hanh vi
* nhung chua ton tai ro rang trong cau truc code

---

## 3. Scene

### Current code map

`Scene` la `module` ro nhat trong repo hien tai, nam o:

* [src/scene/Scene.h](</E:/Animation Software/src/scene/Scene.h>)
* [src/scene/Scene.cpp](</E:/Animation Software/src/scene/Scene.cpp>)
* [src/scene/SceneObject.h](</E:/Animation Software/src/scene/SceneObject.h>)
* [src/scene/SceneObject.cpp](</E:/Animation Software/src/scene/SceneObject.cpp>)
* [src/scene/Transform.h](</E:/Animation Software/src/scene/Transform.h>)
* [src/scene/Bounds3D.h](</E:/Animation Software/src/scene/Bounds3D.h>)
* [src/scene/Bounds3D.cpp](</E:/Animation Software/src/scene/Bounds3D.cpp>)
* [src/scene/SceneMath.h](</E:/Animation Software/src/scene/SceneMath.h>)
* [src/scene/SceneMath.cpp](</E:/Animation Software/src/scene/SceneMath.cpp>)
* [src/scene/MeshData.h](</E:/Animation Software/src/scene/MeshData.h>)
* [src/scene/PrimitiveMeshFactory.h](</E:/Animation Software/src/scene/PrimitiveMeshFactory.h>)
* [src/scene/PrimitiveMeshFactory.cpp](</E:/Animation Software/src/scene/PrimitiveMeshFactory.cpp>)

### Current responsibility in code

`Scene` hien dang la source of truth cho:

* object identity
* hierarchy
* transforms
* bounds
* visibility
* mesh attachment
* current frame
* world transform rebuild

### Notes

`Scene` hien dang la `module` co locality tot nhat trong codebase.

Nhung no dang chua ca:

* scene structure
* animation data ownership
* rigging state ownership
* skin bind/deformation-related data

Dieu nay giup nhanh cho beta, nhung sau nay co the can tang submodule ro hon ben trong `Scene`.

---

## 4. Rendering

### Current code map

`Rendering` hien dang nam o:

* [src/rendering/ViewportRenderer.h](</E:/Animation Software/src/rendering/ViewportRenderer.h>)
* [src/rendering/ViewportRenderer.cpp](</E:/Animation Software/src/rendering/ViewportRenderer.cpp>)
* [src/rendering/ShaderUtils.h](</E:/Animation Software/src/rendering/ShaderUtils.h>)
* [src/rendering/ShaderUtils.cpp](</E:/Animation Software/src/rendering/ShaderUtils.cpp>)
* [src/viewport/EditorCamera.h](</E:/Animation Software/src/viewport/EditorCamera.h>)
* [src/viewport/EditorCamera.cpp](</E:/Animation Software/src/viewport/EditorCamera.cpp>)
* [apps/editor/include/ViewportWidget.h](</E:/Animation Software/apps/editor/include/ViewportWidget.h>)
* [apps/editor/src/ViewportWidget.cpp](</E:/Animation Software/apps/editor/src/ViewportWidget.cpp>)
* [apps/editor/include/ViewportWorkspaceWidget.h](</E:/Animation Software/apps/editor/include/ViewportWorkspaceWidget.h>)
* [apps/editor/src/ViewportWorkspaceWidget.cpp](</E:/Animation Software/apps/editor/src/ViewportWorkspaceWidget.cpp>)

### Current responsibility in code

`Rendering` hien dang gom:

* OpenGL viewport host
* camera interaction
* viewport draw pipeline
* grid / axis / wireframe / selection outline
* gizmo display
* scene-to-GPU sync

### Notes

Code hien tai tach thanh 3 lop:

* `EditorCamera` = camera behavior
* `ViewportRenderer` = draw backend
* `ViewportWidget` = Qt host + scene/render bridge

Day la 1 map kha tot cho `Rendering`, du van con dinh mot phan scene mutation trong `ViewportWidget`.

---

## 5. Animation

### Current code map

`Animation` hien tai chua co folder rieng.
No dang song chu yeu o:

* [src/scene/AnimationData.h](</E:/Animation Software/src/scene/AnimationData.h>)
* [src/scene/Scene.h](</E:/Animation Software/src/scene/Scene.h>)
* [src/scene/Scene.cpp](</E:/Animation Software/src/scene/Scene.cpp>)
* [src/scene/SceneObject.h](</E:/Animation Software/src/scene/SceneObject.h>)
* [src/scene/SceneObject.cpp](</E:/Animation Software/src/scene/SceneObject.cpp>)
* [apps/editor/include/KeyframeTimelineWidget.h](</E:/Animation Software/apps/editor/include/KeyframeTimelineWidget.h>)
* [apps/editor/src/KeyframeTimelineWidget.cpp](</E:/Animation Software/apps/editor/src/KeyframeTimelineWidget.cpp>)
* [apps/editor/include/ScriptCommandSystem.h](</E:/Animation Software/apps/editor/include/ScriptCommandSystem.h>)
* [apps/editor/src/ScriptCommandSystem.cpp](</E:/Animation Software/apps/editor/src/ScriptCommandSystem.cpp>)
* [apps/editor/include/MainWindow.h](</E:/Animation Software/apps/editor/include/MainWindow.h>)
* [apps/editor/src/MainWindow.cpp](</E:/Animation Software/apps/editor/src/MainWindow.cpp>)

### Current responsibility in code

`Animation` hien dang bao gom:

* transform keyframe track
* current frame
* set/delete key
* duplicate key
* shift keyframes
* next/previous key query
* playback controls
* timeline key marker UI

### Notes

Day la `module` product core nhung chua tach ro trong thu muc.

Hien tai no bi chia thanh 3 tang:

* data va ops trong `Scene` / `SceneObject`
* command seam trong `ScriptCommandSystem`
* workflow/UI trong `MainWindow` va `KeyframeTimelineWidget`

Noi nay la ung vien refactor tot nhat neu muon co `Animation/` folder ro rang sau nay.

---

## 6. Rigging

### Current code map

`Rigging` hien tai cung chua co folder rieng.
No dang song chu yeu o:

* [src/scene/Scene.h](</E:/Animation Software/src/scene/Scene.h>)
* [src/scene/Scene.cpp](</E:/Animation Software/src/scene/Scene.cpp>)
* [src/scene/SceneObject.h](</E:/Animation Software/src/scene/SceneObject.h>)
* [src/scene/SceneObject.cpp](</E:/Animation Software/src/scene/SceneObject.cpp>)
* [apps/editor/include/MainWindow.h](</E:/Animation Software/apps/editor/include/MainWindow.h>)
* [apps/editor/src/MainWindow.cpp](</E:/Animation Software/apps/editor/src/MainWindow.cpp>)
* [apps/editor/include/ViewportWidget.h](</E:/Animation Software/apps/editor/include/ViewportWidget.h>)
* [apps/editor/src/ViewportWidget.cpp](</E:/Animation Software/apps/editor/src/ViewportWidget.cpp>)
* [src/rendering/ViewportRenderer.h](</E:/Animation Software/src/rendering/ViewportRenderer.h>)
* [src/rendering/ViewportRenderer.cpp](</E:/Animation Software/src/rendering/ViewportRenderer.cpp>)

### Current responsibility in code

`Rigging` hien dang bao gom:

* `Joint`
* `Skeleton Hierarchy`
* parent/unparent rules
* `Joint Orientation`
* `Bind Pose`
* skin bind state
* skeleton draw support
* deformation support can thiet cho viewport

### Notes

`Rigging` hien tai dang la 1 phan mo rong cua `Scene`.

Do do:

* domain language dung
* data locality tot cho beta
* nhung seam giua `Scene` va `Rigging` chua ro

Neu sau nay mo rong `Constraint`, `IK`, hay weight editing UX, day la noi rat de bi phinh neu khong tach them submodule.

---

## Cross-Cutting IO

Khong nam trong 6 `module` goc, nhung rat quan trong cho mapping thuc te:

* [src/io/FbxImporter.h](</E:/Animation Software/src/io/FbxImporter.h>)
* [src/io/FbxImporter.cpp](</E:/Animation Software/src/io/FbxImporter.cpp>)
* [src/io/FbxSceneBuilder.h](</E:/Animation Software/src/io/FbxSceneBuilder.h>)
* [src/io/FbxSceneBuilder.cpp](</E:/Animation Software/src/io/FbxSceneBuilder.cpp>)
* [src/io/FbxImportValidation.h](</E:/Animation Software/src/io/FbxImportValidation.h>)
* [src/io/FbxImportValidation.cpp](</E:/Animation Software/src/io/FbxImportValidation.cpp>)
* [src/io/FbxImportMessages.h](</E:/Animation Software/src/io/FbxImportMessages.h>)
* [src/io/FbxImportMessages.cpp](</E:/Animation Software/src/io/FbxImportMessages.cpp>)
* [src/io/AssimpSceneDocument.h](</E:/Animation Software/src/io/AssimpSceneDocument.h>)
* [src/io/AssimpSceneDocument.cpp](</E:/Animation Software/src/io/AssimpSceneDocument.cpp>)
* [src/io/PhoenixSceneDocument.h](</E:/Animation Software/src/io/PhoenixSceneDocument.h>)
* [src/io/PhoenixSceneDocument.cpp](</E:/Animation Software/src/io/PhoenixSceneDocument.cpp>)

Map thuc te:

* `IO` hien la adapter layer phuc vu `Scene`
* no cap du lieu vao `Scene`, `Animation`, `Rigging`
* no khong nen tro thanh source of truth rieng

---

## Test Map

Test hien tai map vao `module` nhu sau:

* [tests/unit/SceneTransformTests.cpp](</E:/Animation Software/tests/unit/SceneTransformTests.cpp>)
  * nghieng ve `Scene`, `Animation`, `Rigging`
* [tests/unit/EditorUiTests.cpp](</E:/Animation Software/tests/unit/EditorUiTests.cpp>)
  * nghieng ve `Core`, `Rendering`, `Animation` workflow trong editor

---

## Current Reality Summary

Neu map ngan gon 6 `module` vao code hien tai:

* `Core` = `apps/editor` + `src/logging`
* `Engine` = orchestration dang nam rai trong `MainWindow`, `ViewportWidget`, `Scene`
* `Scene` = `src/scene`
* `Rendering` = `src/rendering` + `src/viewport` + viewport widgets
* `Animation` = `src/scene` + timeline/script/editor workflow
* `Rigging` = `src/scene` + viewport/editor integrations

---

## Recommended Near-Term Interpretation

Trong giai doan hien tai, team nen doc code voi cach hieu sau:

* `src/scene` = canonical domain home, dang chua ca `Scene`, `Animation`, `Rigging`
* `apps/editor` = `Core/Application` workflow surface va mot phan `Engine`
* `MainWindow` + `MainWindowContexts` = concrete adapter cua `EditorShell`
* `src/rendering` + `src/viewport` = `Rendering`
* `src/io` = adapter layer cap du lieu cho cac `module` domain

Neu can tach code dan dan ma khong pha beta rhythm, thu tu refactor hop ly nhat la:

1. lam ro `Animation` seam trong `src/scene`
2. lam ro `Rigging` seam trong `src/scene`
3. rut `Engine` orchestration khoi `MainWindow`
