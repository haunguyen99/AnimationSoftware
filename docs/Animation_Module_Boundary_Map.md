# Animation Module Boundary Map

Tai lieu nay khoa ro ranh gioi hien tai cua `Animation` trong repo:

* nhung gi la `Animation` thuan
* nhung gi la `Animation editor shared seam`
* nhung gi la `Animation UI`
* nhung gi la `Engine/runtime orchestration`

Muc tieu:

* tranh tiep tuc coi moi thu lien quan timeline/playback la `Animation` thuan
* co moc ownership ro sau dot tach boundary
* de cac buoc refactor sau khong bi tro lai `MainWindow`-style cluster

## Ket luan nhanh

`Animation` hien tai da duoc tach thanh 4 lop ro hon:

* `src/animation/data` + `src/animation/scene` = animation domain core
* `src/animation/editor` = cac seam shared cho editor/runtime workflow
* `apps/editor` = timeline UI va UI-facing projection
* `src/engine/*` = orchestration, playback timer, runtime apply

Boundary nay tot hon truoc vi:

* `EditorAnimationState` khong con bi nhot trong UI controller header
* scene mutation da tach khoi `EditorAnimationController`
* timeline view-model projection da tach khoi state math
* animation workflow seam da roi khoi `apps/editor`

## File-Level Map

| Hien trang | Ownership dung | Dich module de xep | Ghi chu |
| --- | --- | --- | --- |
| `src/animation/data/TransformKeyframeTrack.h` | animation data | `Animation` | keyframe track type |
| `src/animation/data/ObjectAnimationState.h/.cpp` | object animation data | `Animation` | luu/sua keyframes cho tung object |
| `src/animation/scene/SceneAnimationState.h/.cpp` | animation scene evaluation | `Animation` | current frame, evaluate/interpolate transform |
| `src/animation/editor/EditorAnimationState.h` | editor-shared animation state | `Animation editor seam` | playback range, auto-key, playing |
| `src/animation/editor/EditorAnimationSceneMutation.h/.cpp` | scene mutation seam | `Animation editor seam` | set/delete/duplicate/shift key, jump key |
| `src/animation/editor/EditorAnimationFlowController.h/.cpp` | workflow/policy seam | `Animation editor seam` | selection-aware animation flows, script/result shaping |
| `apps/editor/include/EditorAnimationTimelineViewModel.h` | timeline UI model | `Animation UI` | UI-facing projection data |
| `apps/editor/include/EditorAnimationTimelineViewBuilder.h` / `src/EditorAnimationTimelineViewBuilder.cpp` | timeline projection | `Animation UI` | map scene + state -> timeline view model |
| `apps/editor/include/AnimationTimelinePanel.h` / `src/AnimationTimelinePanel.cpp` | timeline controls UI | `Animation UI` | panel wiring, callbacks, button states |
| `apps/editor/include/KeyframeTimelineWidget.h` / `src/KeyframeTimelineWidget.cpp` | timeline drawing UI | `Animation UI` | draw frame markers/current frame |
| `apps/editor/include/EditorAnimationController.h` / `src/EditorAnimationController.cpp` | animation state math | `Animation editor seam` | setCurrentFrame, playback range, step, autoplay state |
| `src/engine/animation/EditorAnimationEngineFacade.h/.cpp` | engine facade | `Engine` | apply flow result vao runtime |
| `src/engine/playback/EditorPlaybackController.h/.cpp` | playback runtime | `Engine` | QTimer playback orchestration |
| `src/engine/runtime/EditorSceneRuntimeController.h/.cpp` | runtime scene apply | `Engine` | undo, replace scene, selection refresh |

## Ownership Theo Lop

### 1. Animation thuan

Day la phan domain animation dung nghia:

* keyframe storage
* keyframe editing o muc data
* transform interpolation
* evaluate object transform theo frame

Code tham chieu:

* [ObjectAnimationState.h](</E:/Animation Software/src/animation/data/ObjectAnimationState.h:1>)
* [ObjectAnimationState.cpp](</E:/Animation Software/src/animation/data/ObjectAnimationState.cpp:1>)
* [SceneAnimationState.h](</E:/Animation Software/src/animation/scene/SceneAnimationState.h:1>)
* [SceneAnimationState.cpp](</E:/Animation Software/src/animation/scene/SceneAnimationState.cpp:1>)

Ownership note:

* day la `Animation` core
* khong phu thuoc vao UI
* khong phu thuoc vao playback timer

### 2. Animation editor shared seam

Day la lop khong phai domain thuan, nhung cung khong nen nam trong UI widget.

`EditorAnimationState`

* current frame
* playback range
* auto key
* playing

Code:

* [EditorAnimationState.h](</E:/Animation Software/src/animation/editor/EditorAnimationState.h:1>)

`EditorAnimationController`

* state transition math
* current frame clamp
* playback range normalize
* step / advance / toggle-related state math

Code:

* [EditorAnimationController.h](</E:/Animation Software/apps/editor/include/EditorAnimationController.h:1>)
* [EditorAnimationController.cpp](</E:/Animation Software/apps/editor/src/EditorAnimationController.cpp:1>)

`EditorAnimationSceneMutation`

* scene snapshot -> apply key mutation -> return updated scene
* jump to next/previous keyframe

Code:

* [EditorAnimationSceneMutation.h](</E:/Animation Software/src/animation/editor/EditorAnimationSceneMutation.h:1>)
* [EditorAnimationSceneMutation.cpp](</E:/Animation Software/src/animation/editor/EditorAnimationSceneMutation.cpp:1>)

`EditorAnimationFlowController`

* selection-aware animation actions
* error/status shaping
* script command/result text shaping
* flow result packaging

Code:

* [EditorAnimationFlowController.h](</E:/Animation Software/src/animation/editor/EditorAnimationFlowController.h:1>)
* [EditorAnimationFlowController.cpp](</E:/Animation Software/src/animation/editor/EditorAnimationFlowController.cpp:1>)

Ownership note:

* day la lop `application seam` cua animation
* no dung cho editor/runtime, nhung khong nen map nham thanh UI thuần

### 3. Animation UI

Day la phan hien thi va dieu khien timeline.

`EditorAnimationTimelineViewModel`

* data cho panel/timeline UI
* text/style/button state hien thi

Code:

* [EditorAnimationTimelineViewModel.h](</E:/Animation Software/apps/editor/include/EditorAnimationTimelineViewModel.h:1>)

`EditorAnimationTimelineViewBuilder`

* map `Scene + EditorAnimationState + selection` thanh view model

Code:

* [EditorAnimationTimelineViewBuilder.h](</E:/Animation Software/apps/editor/include/EditorAnimationTimelineViewBuilder.h:1>)
* [EditorAnimationTimelineViewBuilder.cpp](</E:/Animation Software/apps/editor/src/EditorAnimationTimelineViewBuilder.cpp:1>)

`AnimationTimelinePanel`

* timeline controls
* transport buttons
* spinbox/slider wiring
* callback bridge tu UI ra shell

Code:

* [AnimationTimelinePanel.h](</E:/Animation Software/apps/editor/include/AnimationTimelinePanel.h:1>)
* [AnimationTimelinePanel.cpp](</E:/Animation Software/apps/editor/src/AnimationTimelinePanel.cpp:1>)

`KeyframeTimelineWidget`

* marker drawing
* current frame line drawing

Code:

* [KeyframeTimelineWidget.h](</E:/Animation Software/apps/editor/include/KeyframeTimelineWidget.h:1>)
* [KeyframeTimelineWidget.cpp](</E:/Animation Software/apps/editor/src/KeyframeTimelineWidget.cpp:1>)

Ownership note:

* day la `Animation UI`
* khong nen dua nguoc ve `src/animation`

### 4. Engine/runtime orchestration

Day la lop thuc thi va dong bo runtime.

`EditorAnimationEngineFacade`

* goi flow
* apply result vao runtime scene + animation state

Code:

* [EditorAnimationEngineFacade.h](</E:/Animation Software/src/engine/animation/EditorAnimationEngineFacade.h:1>)
* [EditorAnimationEngineFacade.cpp](</E:/Animation Software/src/engine/animation/EditorAnimationEngineFacade.cpp:1>)

`EditorPlaybackController`

* QTimer playback
* advance frame theo runtime tick

Code:

* [EditorPlaybackController.h](</E:/Animation Software/src/engine/playback/EditorPlaybackController.h:1>)
* [EditorPlaybackController.cpp](</E:/Animation Software/src/engine/playback/EditorPlaybackController.cpp:1>)

`EditorSceneRuntimeController`

* replace scene
* record undo
* apply animation state
* refresh selection/panels

Code:

* [EditorSceneRuntimeController.h](</E:/Animation Software/src/engine/runtime/EditorSceneRuntimeController.h:1>)
* [EditorSceneRuntimeController.cpp](</E:/Animation Software/src/engine/runtime/EditorSceneRuntimeController.cpp:1>)

Ownership note:

* day la `Engine`
* no tieu thu animation seam, khong phai animation core

## Diem con dính boundary

Nhung diem sau van can nho khi doc code:

* `Scene` va `SceneObject` van la canonical host cho animation data/mutation API
* `EditorAnimationController` van nam duong dan `apps/editor`, du ownership da nghieng ve shared seam hon la UI
* `EditorAnimationTimelineViewModel` van dat trong `apps/editor/include`, hop ly hien tai vi no UI-facing

## Tieu chi coi Animation da sach

Co the coi boundary nay on khi:

* animation core khong bi keo them UI concerns
* workflow seam khong quay lai `apps/editor`
* engine orchestration khong chua keyframe business logic chi tiet
* timeline projection va panel UI khong chen nguoc vao domain core

## Trang thai hien tai

Hien tai `Animation` da dat muc:

* core kha sach
* workflow seam kha ro
* UI projection da tach rieng
* playback/runtime orchestration da ro ownership

Ket luan:

* `Animation` khong con la mot khoi lon tron giua state, flow, scene mutation, UI, va playback nua
* boundary hien tai da du de di tiep sang module khac ma khong can quay lai don dot gap
