# Rigging Module Boundary Map

Tai lieu nay khoa ro ranh gioi hien tai cua `Rigging` trong repo:

* nhung gi la `Rigging` thuan
* nhung gi la `Rigging editor shared seam`
* nhung gi la `Rigging UI`
* nhung gi la `Engine/runtime bridge`

Muc tieu:

* tranh tiep tuc coi moi thu lien quan joint/skin/bind pose la mot khoi lon
* co map ownership ro sau dot tach seam
* giu module `Rigging` de doc va de refactor tiep

## Ket luan nhanh

`Rigging` hien tai da ro hon truoc va co the doc thanh 4 lop:

* `src/rigging/data` = rigging data core
* `src/rigging/scene` = rigging scene logic
* `src/rigging/editor` = editor workflow seam
* `apps/editor` + `src/engine` = UI va runtime bridge

Sau cac buoc tach gan day:

* `EditorRiggingController` da roi khoi `apps/editor`
* `SceneRiggingController` da duoc chia concern thanh:
  * joint ops
  * skin binding ops
  * skin deformation ops

## File-Level Map

| Hien trang | Ownership dung | Dich module de xep | Ghi chu |
| --- | --- | --- | --- |
| `src/rigging/data/ObjectRigState.h/.cpp` | rigging data | `Rigging` | joint orientation, bind pose, skin binding data |
| `src/rigging/scene/JointRiggingOps.h/.cpp` | joint scene logic | `Rigging` | create joint, joint orient, bind pose world transform |
| `src/rigging/scene/SkinBindingOps.h/.cpp` | skin binding scene logic | `Rigging` | bind skin, validate weights, set/clear binding |
| `src/rigging/scene/SkinDeformationOps.h/.cpp` | deformation scene logic | `Rigging` | build deformed mesh from skin weights |
| `src/rigging/scene/SceneRiggingController.h/.cpp` | facade seam | `Rigging` | giu API cu, delegate qua 3 ops |
| `src/rigging/editor/EditorRiggingController.h/.cpp` | rigging workflow seam | `Rigging editor seam` | selection-aware rigging flow, command/result shaping |
| `src/scene/Scene.h/.cpp` | scene bridge | `Scene bridge` | expose canonical rigging entry points |
| `src/scene/SceneObject.h/.cpp` | object bridge | `Scene bridge` | host `ObjectRigState`, expose joint/skin accessors |
| `apps/editor/src/EditorInspectorController.cpp` | rigging inspector UI | `Rigging UI` | joint orient display, bind pose/skin status display |
| `apps/editor/src/EditorChannelBoxController.cpp` | rigging transform UI | `Rigging UI` | joint orient editing flow |
| `src/engine/runtime/EditorViewportSceneController.cpp` | runtime mutation bridge | `Engine` | viewport-driven joint mutation bridge |
| `apps/editor/src/ViewportWidget.cpp` / `ViewportWorkspaceWidget.cpp` | viewport host bridge | `Viewport UI` | route joint mutation tu viewport vao runtime |

## Ownership Theo Lop

### 1. Rigging thuan

Day la phan domain rigging dung nghia.

`ObjectRigState`

* `jointOrientation`
* `bindPoseLocalTransform`
* `hasBindPose`
* `skinBindLocalTransform`
* `skinJointIds`
* `skinWeights`

Code:

* [ObjectRigState.h](</E:/Animation Software/src/rigging/data/ObjectRigState.h:1>)
* [ObjectRigState.cpp](</E:/Animation Software/src/rigging/data/ObjectRigState.cpp:1>)

`JointRiggingOps`

* create joint
* set/reset/align joint orientation
* capture bind pose
* build bind-pose world transform

Code:

* [JointRiggingOps.h](</E:/Animation Software/src/rigging/scene/JointRiggingOps.h:1>)
* [JointRiggingOps.cpp](</E:/Animation Software/src/rigging/scene/JointRiggingOps.cpp:1>)

`SkinBindingOps`

* collect joint subtree
* infer nearest joint binding
* normalize weights
* validate and set skin binding
* clear skin binding

Code:

* [SkinBindingOps.h](</E:/Animation Software/src/rigging/scene/SkinBindingOps.h:1>)
* [SkinBindingOps.cpp](</E:/Animation Software/src/rigging/scene/SkinBindingOps.cpp:1>)

`SkinDeformationOps`

* build skinned mesh positions/normals
* apply joint matrices from bind pose -> current pose

Code:

* [SkinDeformationOps.h](</E:/Animation Software/src/rigging/scene/SkinDeformationOps.h:1>)
* [SkinDeformationOps.cpp](</E:/Animation Software/src/rigging/scene/SkinDeformationOps.cpp:1>)

Ownership note:

* day la `Rigging` core
* khong phai UI
* khong phai engine playback/runtime policy

### 2. Rigging editor shared seam

`EditorRiggingController`

Phan nay khong phai rigging core, nhung cung khong con la UI thuần.

No dang chua:

* validation theo selection hien tai
* workflow mark parent / reparent / bind skin
* status text / error text / command text / result text
* operation result packaging cho shell

Code:

* [EditorRiggingController.h](</E:/Animation Software/src/rigging/editor/EditorRiggingController.h:1>)
* [EditorRiggingController.cpp](</E:/Animation Software/src/rigging/editor/EditorRiggingController.cpp:1>)

Ownership note:

* day la `Rigging application seam`
* no la lop shared giua shell, scene snapshot flow, va editor command semantics
* khong nen map nham thanh `apps/editor UI`

### 3. Scene bridge

`Scene` va `SceneObject` van la canonical host cho rigging data va mutation API.

`Scene`

* `setJointOrientation(...)`
* `resetJointOrientation(...)`
* `alignJointOrientationToChild(...)`
* `captureBindPose(...)`
* `bindObjectToSkeleton(...)`
* `setObjectSkinBinding(...)`
* `clearObjectSkinBinding(...)`
* `buildDeformedMesh(...)`
* `bindPoseWorldTransform(...)`

Code:

* [Scene.h](</E:/Animation Software/src/scene/Scene.h:1>)
* [Scene.cpp](</E:/Animation Software/src/scene/Scene.cpp:1>)

`SceneObject`

* exposes `ObjectRigState` qua getter/setter
* joint + bind pose + skin binding data access

Code:

* [SceneObject.h](</E:/Animation Software/src/scene/SceneObject.h:1>)
* [SceneObject.cpp](</E:/Animation Software/src/scene/SceneObject.cpp:1>)

Ownership note:

* day la bridge hop ly, nhung cung la cho boundary co the bi loang neu khong can than

### 4. Rigging UI

Day la nhung phan UI-facing.

`EditorInspectorController`

* display joint orient
* display bind pose status
* display skin binding status
* enable/disable joint tools

Code:

* [EditorInspectorController.cpp](</E:/Animation Software/apps/editor/src/EditorInspectorController.cpp:120>)

`EditorChannelBoxController`

* apply joint orientation edit tu UI fields
* build command/result text cho joint orient edit

Code:

* [EditorChannelBoxController.cpp](</E:/Animation Software/apps/editor/src/EditorChannelBoxController.cpp:53>)

Ownership note:

* day la `Rigging UI`
* khong nen dua nguoc vao `src/rigging`

### 5. Engine/runtime bridge

`EditorViewportSceneController`

* route viewport-side rigging mutations vao scene/runtime

Code:

* [EditorViewportSceneController.cpp](</E:/Animation Software/src/engine/runtime/EditorViewportSceneController.cpp:113>)

`ViewportWidget` / `ViewportWorkspaceWidget`

* host viewport action bridge cho joint orientation mutation

Code:

* [ViewportWidget.cpp](</E:/Animation Software/apps/editor/src/ViewportWidget.cpp:302>)
* [ViewportWorkspaceWidget.cpp](</E:/Animation Software/apps/editor/src/ViewportWorkspaceWidget.cpp:364>)

Ownership note:

* day la bridge/runtime + viewport UI
* khong phai rigging domain logic

## Diem con dính boundary

Nhung diem sau van can ghi nho:

* `Scene` va `SceneObject` van expose rat nhieu API rigging, nen `Rigging` chua hoan toan tach khoi `Scene`
* `EditorSceneMutationController::bindSkin(...)` van la mot diem co mo boundary voi rigging workflow
* `EditorInspectorController` va `EditorChannelBoxController` van co logic UI-facing cho rigging, nhung dieu nay hop ly

## Tieu chi xong cho Rigging

Co the coi `Rigging` da sach hon nua khi:

* joint ops, skin binding ops, skin deformation ops khong bi tron lai vao mot file lon
* editor rigging workflow khong tro lai `apps/editor`
* scene bridge khong tiep tuc nuot them workflow/UI concerns
* deformation logic chi nam trong rigging seam, khong ro ri ra rendering/runtime

## Trang thai hien tai

Hien tai `Rigging` da dat muc:

* data core ro
* scene logic ro hon nhieu sau khi tach 3 concern
* editor workflow seam da roi khoi `apps/editor`
* UI va runtime bridge doc de nhan dien hon

Ket luan:

* `Rigging` khong con la mot khoi lon gom joint ops + skin ops + deformation + editor flow tron chung
* boundary hien tai da du tot de tiep tuc sang module khac hoac lam sau hon neu can
