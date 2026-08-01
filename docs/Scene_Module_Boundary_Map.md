# Scene Module Boundary Map

Tai lieu nay khoa ro ranh gioi hien tai cua `Scene` trong repo:

* nhung gi la `Scene` thuan
* nhung gi la `Scene bridge`
* nhung gi thuc chat da thuoc `Animation`
* nhung gi thuc chat da thuoc `Rigging`

Muc tieu:

* tranh tiep tuc coi `src/scene` la mot khoi dong nhat
* giu ro vai tro canonical graph package cua `Scene`
* dong bo cach doc module nay voi boundary maps cua `Rendering`, `Animation`, va `Rigging`

## Ket luan nhanh

`Scene` hien tai da sach hon truoc nhieu, nhung van chua la `Scene` thuan tuyet doi.

Trang thai hien tai:

* `Animation` da co seam rieng qua `ObjectAnimationState` va `SceneAnimationState`
* `Rigging` da co seam rieng qua `ObjectRigState`, `JointRiggingOps`, `SkinBindingOps`, `SkinDeformationOps`, va `SceneRiggingController`
* `Scene` va `SceneObject` van giu `bridge API` de call sites cua editor, io, rendering, va test khong phai doi hang loat

Vi vay:

* `storage ownership` da tach ro hon
* `implementation ownership` da tach ro hon
* `public API ownership` van con mot lop facade/bridge

## File-Level Map

| Hien trang | Ownership dung | Dich module de xep | Ghi chu |
| --- | --- | --- | --- |
| `src/scene/Transform.h` | transform value type co ban | `Scene` | value object nen o lai |
| `src/scene/Bounds3D.h/.cpp` | bounds/hinh hoc co ban | `Scene` | geometry primitive phu hop o lai |
| `src/scene/MeshData.h` | mesh payload canonical | `Scene` | mesh registry payload cho scene graph |
| `src/scene/SceneMath.h/.cpp` | scene math/shared geometry compose | `Scene` | compose matrix, bounds transform |
| `src/scene/PrimitiveMeshFactory.h/.cpp` | primitive geometry creation | `Scene` | phuc vu primitive scene objects |
| `src/scene/SceneObject.h/.cpp` | scene object graph + bridge API | `Scene bridge` | host graph state, expose animation/rigging accessors |
| `src/scene/Scene.h/.cpp` | scene graph + bridge API | `Scene bridge` | canonical graph facade, delegate sang animation/rigging seams |
| `src/animation/data/TransformKeyframeTrack.h` | keyframe data model | `Animation` | da tach khoi `Scene` |
| `src/animation/data/ObjectAnimationState.h/.cpp` | object animation storage | `Animation` | storage keyframe cua object |
| `src/animation/scene/SceneAnimationState.h/.cpp` | scene animation evaluation | `Animation` | current frame + evaluate transform |
| `src/rigging/data/ObjectRigState.h/.cpp` | rigging data storage | `Rigging` | joint, bind pose, skin data |
| `src/rigging/scene/JointRiggingOps.h/.cpp` | joint scene logic | `Rigging` | joint creation/orientation/bind pose |
| `src/rigging/scene/SkinBindingOps.h/.cpp` | skin binding logic | `Rigging` | bind skin, validate weights |
| `src/rigging/scene/SkinDeformationOps.h/.cpp` | deformation logic | `Rigging` | build skinned mesh |
| `src/rigging/scene/SceneRiggingController.h/.cpp` | rigging facade seam | `Rigging` | giu API cu, delegate qua ops |

## Ownership Theo Lop

### 1. Scene thuan

Day la nhung phan nen o lai trong `Scene` domain.

`Transform`

* translation / rotation / scale value object

Code:

* [Transform.h](</E:/Animation Software/src/scene/Transform.h:1>)

`Bounds3D`

* bounds value type
* expand / center / radius / valid state

Code:

* [Bounds3D.h](</E:/Animation Software/src/scene/Bounds3D.h:1>)
* [Bounds3D.cpp](</E:/Animation Software/src/scene/Bounds3D.cpp:1>)

`MeshData`

* canonical mesh payload
* positions / normals / colors / indices / bounds

Code:

* [MeshData.h](</E:/Animation Software/src/scene/MeshData.h:1>)

`SceneMath`

* compose matrix
* bounds transform helpers

Code:

* [SceneMath.h](</E:/Animation Software/src/scene/SceneMath.h:1>)
* [SceneMath.cpp](</E:/Animation Software/src/scene/SceneMath.cpp:1>)

`PrimitiveMeshFactory`

* primitive geometry generation

Code:

* [PrimitiveMeshFactory.h](</E:/Animation Software/src/scene/PrimitiveMeshFactory.h:1>)
* [PrimitiveMeshFactory.cpp](</E:/Animation Software/src/scene/PrimitiveMeshFactory.cpp:1>)

Ownership note:

* day la `Scene` core vi no mo ta graph payload, transform, bounds, va primitive geometry
* no khong nen bi keo vao workflow cua animation hay rigging

### 2. Scene bridge

Day la lop canonical facade cho object graph, nhung da co bridge API sang domain seams.

`SceneObject`

Phan `Scene` thuan ben trong `SceneObject`:

* `id`, `name`
* `parentId`, `childIds`
* `meshHandles`
* `localBounds`, `worldBounds`
* `visible`
* `localTransform`, `authoredTransform`

Code:

* [SceneObject.h](</E:/Animation Software/src/scene/SceneObject.h:1>)
* [SceneObject.cpp](</E:/Animation Software/src/scene/SceneObject.cpp:1>)

`Scene`

Phan `Scene` thuan ben trong `Scene`:

* create/find/contains object
* root/all object ids
* mesh registry
* clear/append/remove/duplicate/reparent
* scene bounds rebuild
* world data rebuild
* scene bounds/debug dump

Code:

* [Scene.h](</E:/Animation Software/src/scene/Scene.h:1>)
* [Scene.cpp](</E:/Animation Software/src/scene/Scene.cpp:1>)

Ownership note:

* `SceneObject` va `Scene` hien la `Scene bridge`, khong chi la scene core
* day la lop giu call sites on dinh trong khi implementation ownership da tach sang module khac

### 3. Animation

Day la nhung phan trong `src/scene` co ownership thuc te nghieng ve `Animation`.

`SceneObject` animation bridge API:

* `hasAnimation()`
* `hasTransformKeyframe(...)`
* `transformKeyframes()`
* `setTransformKeyframes(...)`
* `setTransformKeyframe(...)`
* `removeTransformKeyframe(...)`
* `duplicateTransformKeyframe(...)`
* `offsetAllTransformKeyframes(...)`
* `nextTransformKeyframeAfter(...)`
* `previousTransformKeyframeBefore(...)`

`Scene` animation bridge API:

* `currentFrame()`
* `setCurrentFrame(...)`
* `setLocalTransform(..., autoKeyEnabled)`
* `setObjectKeyframe(...)`
* `removeObjectKeyframe(...)`
* `duplicateObjectKeyframe(...)`
* `offsetObjectKeyframes(...)`
* `nextObjectKeyframe(...)`
* `previousObjectKeyframe(...)`

Code:

* [SceneObject.h](</E:/Animation Software/src/scene/SceneObject.h:1>)
* [SceneObject.cpp](</E:/Animation Software/src/scene/SceneObject.cpp:1>)
* [Scene.h](</E:/Animation Software/src/scene/Scene.h:1>)
* [Scene.cpp](</E:/Animation Software/src/scene/Scene.cpp:1>)
* [ObjectAnimationState.h](</E:/Animation Software/src/animation/data/ObjectAnimationState.h:1>)
* [ObjectAnimationState.cpp](</E:/Animation Software/src/animation/data/ObjectAnimationState.cpp:1>)
* [SceneAnimationState.h](</E:/Animation Software/src/animation/scene/SceneAnimationState.h:1>)
* [SceneAnimationState.cpp](</E:/Animation Software/src/animation/scene/SceneAnimationState.cpp:1>)

Ownership note:

* `Scene` khong con giu implementation current-frame state/evaluation nhu truoc
* phan nay da la `Animation`, `Scene` chi con giu bridge facade

### 4. Rigging

Day la nhung phan trong `src/scene` co ownership thuc te nghieng ve `Rigging`.

`SceneObject` rigging bridge API:

* `Kind::Joint`
* `isJoint()`
* `jointOrientation()`
* `setJointOrientation(...)`
* `bindPoseLocalTransform()`
* `setBindPoseLocalTransform(...)`
* `hasBindPose()`
* `setHasBindPose(...)`
* `hasSkinBinding()`
* `setHasSkinBinding(...)`
* `skinBindLocalTransform()`
* `setSkinBindLocalTransform(...)`
* `skinJointIds()`
* `setSkinJointIds(...)`
* `skinWeights()`
* `setSkinWeights(...)`
* `clearSkinBinding()`

`Scene` rigging bridge API:

* `createJoint(...)`
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

* [SceneObject.h](</E:/Animation Software/src/scene/SceneObject.h:1>)
* [SceneObject.cpp](</E:/Animation Software/src/scene/SceneObject.cpp:1>)
* [Scene.h](</E:/Animation Software/src/scene/Scene.h:1>)
* [Scene.cpp](</E:/Animation Software/src/scene/Scene.cpp:1>)
* [ObjectRigState.h](</E:/Animation Software/src/rigging/data/ObjectRigState.h:1>)
* [ObjectRigState.cpp](</E:/Animation Software/src/rigging/data/ObjectRigState.cpp:1>)
* [JointRiggingOps.h](</E:/Animation Software/src/rigging/scene/JointRiggingOps.h:1>)
* [SkinBindingOps.h](</E:/Animation Software/src/rigging/scene/SkinBindingOps.h:1>)
* [SkinDeformationOps.h](</E:/Animation Software/src/rigging/scene/SkinDeformationOps.h:1>)
* [SceneRiggingController.h](</E:/Animation Software/src/rigging/scene/SceneRiggingController.h:1>)
* [SceneRiggingController.cpp](</E:/Animation Software/src/rigging/scene/SceneRiggingController.cpp:1>)

Ownership note:

* theo [ADR-009](</E:/Animation Software/docs/adr/ADR-009-joint-orientation-stays-separate-from-animated-rotation.md>), `jointOrientation` la rigging state rieng
* `Scene` khong con giu implementation rigging lon nhu truoc, nhung van con la public facade cho call sites

## Diem con dính boundary

Nhung diem sau van can ghi nho:

* `Scene` va `SceneObject` van expose nhieu API cua `Animation` va `Rigging`, nen boundary public API chua mong hoan toan
* `worldTransform(...)` va `rebuildWorldDataForObject(...)` van can biet mot phan rigging state de compose dung world data
* `Scene` van la diem canonical ma editor, io, rendering, va tests di qua de chạm animation/rigging seams

## Tieu chi coi Scene da sach

Co the coi `Scene` sach hon nua khi:

* `Scene.cpp` khong tiep tuc nuot them animation/rigging logic moi
* `SceneObject` khong tiep tuc mo rong them state module-specific moi
* domain seams chiu trach nhiem implementation, `Scene` chi giu facade mong
* call sites moi uu tien di qua seam phu hop thay vi day logic vao `Scene`

## Trang thai hien tai

Hien tai `Scene` da dat muc:

* la canonical graph package
* da tach duoc storage ownership cua animation va rigging
* da tach duoc implementation ownership cua animation va rigging
* van giu bridge facade de giu repo on dinh

Ket luan:

* `Scene` khong con la mot `mixed domain package` nang nhu truoc
* nhung no van la `Scene bridge`, chua phai `Scene` thuan 100%
* day la trang thai hop ly cho giai doan hien tai, mien la logic moi khong quay tro lai day vao `Scene`
