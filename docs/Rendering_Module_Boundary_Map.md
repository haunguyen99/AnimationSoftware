# Rendering Module Boundary Map

Tai lieu nay khoa ro ranh gioi hien tai cua `Rendering` trong repo:

* nhung gi la `Rendering` thuan
* nhung gi thuc chat la `Viewport UI`
* nhung gi la `Engine/Runtime orchestration`

Muc tieu cua tai lieu:

* tranh tiep tuc coi tat ca code lien quan viewport la `Rendering`
* tach ro backend draw voi widget host va input interaction
* tao bang move map an toan neu can refactor tiep

Tai lieu nay map theo `ownership thuc te cua code`, khong map theo ten thu muc don thuan.

## Ket luan nhanh

`Rendering` hien tai da co mot loi kha ro:

* `ShaderUtils` = shader/OpenGL helper thuan
* `ViewportRenderer` = draw backend chinh
* `EditorCamera` = camera math va viewport projection state

Tuy vay, `ViewportRenderer` chua phai backend thuan tuyet doi vi van dang om:

* scene-to-render data extraction
* skeleton/joint draw adaptation
* skinning-aware mesh upload decision
* gizmo/joint geometry builders

Dong thoi [ViewportWidget](</E:/Animation Software/apps/editor/src/ViewportWidget.cpp>) van la noi ghep:

* Qt OpenGL host
* input handling
* object picking
* gizmo interaction
* selection/render sync
* runtime scene mutation bridge

Vi vay, `Rendering` hien tai nen duoc doc nhu:

* co backend ro
* nhung boundary giua `Rendering` va `Viewport UI` van con mot lop pha tron

## File-Level Map

| Hien trang | Ownership dung | Dich module de xep | Ghi chu |
| --- | --- | --- | --- |
| `src/rendering/ShaderUtils.h/.cpp` | `Rendering` thuan | `Rendering` | shader compile/link + GL error logging |
| `src/rendering/ViewportRenderer.h/.cpp` | `Rendering` backend + scene adapter | `Rendering` | chua hoan toan thuan vi dang doc `Scene`/`Rigging` truc tiep |
| `src/viewport/EditorCamera.h/.cpp` | camera/view math | `Rendering` hoac `Viewport` shared seam | hien tai hop ly trong lop viewport/rendering |
| `apps/editor/include/ViewportWidget.h` / `src/ViewportWidget.cpp` | `Viewport UI` + runtime bridge | `Viewport UI` | khong nen coi la `Rendering` thuan |
| `apps/editor/include/ViewportWorkspaceWidget.h` / `src/ViewportWorkspaceWidget.cpp` | `Viewport UI` shell adapter | `Viewport UI` | workspace/panel host, khong phai render backend |
| `apps/editor/include/EditorViewportUiController.h` / `src/EditorViewportUiController.cpp` | viewport action/UI policy | `Viewport UI` | transform mode, camera preset, axis orientation checks |
| `apps/editor/include/EditorViewportCommandController.h` / `src/EditorViewportCommandController.cpp` | command/interaction shaping | `Viewport UI` hoac `Engine bridge` | can doc theo workflow hon la rendering |
| `src/engine/runtime/EditorViewportSceneController.h/.cpp` | runtime scene mutation orchestration | `Engine` | viewport-side mutation seam, khong phai rendering |

## Ownership Map Theo Block

### 1. `Rendering` thuan

Day la nhung phan da ro ownership backend.

`ShaderUtils`

* `buildProgram(...)`
* `logOpenGlError(...)`

Code tham chieu:

* [E:\Animation Software\src\rendering\ShaderUtils.h](</E:/Animation Software/src/rendering/ShaderUtils.h:1>)
* [E:\Animation Software\src\rendering\ShaderUtils.cpp](</E:/Animation Software/src/rendering/ShaderUtils.cpp:1>)

`EditorCamera`

* orbit/pan/zoom
* perspective/orthographic projection
* camera preset math
* `viewMatrix()`
* `projectionMatrix()`
* `worldUnitsPerPixelAt(...)`

Code tham chieu:

* [E:\Animation Software\src\viewport\EditorCamera.h](</E:/Animation Software/src/viewport/EditorCamera.h:1>)
* [E:\Animation Software\src\viewport\EditorCamera.cpp](</E:/Animation Software/src/viewport/EditorCamera.cpp:1>)

`ViewportRenderer`

* GL resource lifecycle
* shader program ownership
* buffer upload
* draw pass orchestration
* grid/axis/mesh/selection/joint/gizmo draw calls

Code tham chieu:

* [E:\Animation Software\src\rendering\ViewportRenderer.h](</E:/Animation Software/src/rendering/ViewportRenderer.h:1>)
* [E:\Animation Software\src\rendering\ViewportRenderer.cpp](</E:/Animation Software/src/rendering/ViewportRenderer.cpp:1>)

Ownership note:

* day la `Rendering` vi no dung ra GPU draw backend va camera/view math
* tuy nhien `ViewportRenderer` van chua phai backend thuan tuyet doi vi van doc domain model truc tiep

### 2. `Viewport UI`

Day la nhung phan thuoc ownership cua widget host, input interaction, va UI-facing viewport behavior.

`ViewportWidget`

* `QOpenGLWidget` lifecycle:
  * `initializeGL()`
  * `resizeGL()`
  * `paintGL()`
* mouse/wheel input handling
* object picking
* gizmo axis picking
* drag transform interaction
* selection sync
* camera preset label / viewport label paint
* renderer sync trigger

Code tham chieu:

* [E:\Animation Software\apps\editor\include\ViewportWidget.h](</E:/Animation Software/apps/editor/include/ViewportWidget.h:1>)
* [E:\Animation Software\apps\editor\src\ViewportWidget.cpp](</E:/Animation Software/apps/editor/src/ViewportWidget.cpp:1>)

`ViewportWorkspaceWidget`

* viewport workspace host
* active viewport switching / workspace-level forwarding

Code tham chieu:

* [E:\Animation Software\apps\editor\include\ViewportWorkspaceWidget.h](</E:/Animation Software/apps/editor/include/ViewportWorkspaceWidget.h:1>)
* [E:\Animation Software\apps\editor\src\ViewportWorkspaceWidget.cpp](</E:/Animation Software/apps/editor/src/ViewportWorkspaceWidget.cpp:1>)

`EditorViewportUiController`

* transform mode checks
* camera preset checks
* axis orientation checks
* workspace layout result shaping

Code tham chieu:

* [E:\Animation Software\apps\editor\include\EditorViewportUiController.h](</E:/Animation Software/apps/editor/include/EditorViewportUiController.h:1>)
* [E:\Animation Software\apps\editor\src\EditorViewportUiController.cpp](</E:/Animation Software/apps/editor/src/EditorViewportUiController.cpp:1>)

Ownership note:

* day la `Viewport UI`, khong nen coi la `Rendering`
* no host renderer, nhung khong phai draw backend

### 3. `Engine/Runtime orchestration`

Day la nhung phan khong nen thuoc `Rendering`, du dang o viewport workflow.

`EditorViewportSceneController`

* append/replace/clear scene
* object transform mutation
* visibility mutation
* current frame mutation
* keyframe mutation
* rigging mutation
* primitive/joint creation

Code tham chieu:

* [E:\Animation Software\src\engine\runtime\EditorViewportSceneController.h](</E:/Animation Software/src/engine/runtime/EditorViewportSceneController.h:1>)
* [E:\Animation Software\src\engine\runtime\EditorViewportSceneController.cpp](</E:/Animation Software/src/engine/runtime/EditorViewportSceneController.cpp:1>)

`EditorViewportCommandController`

* viewport command/result shaping
* workflow-facing viewport command bridge

Code tham chieu:

* [E:\Animation Software\apps\editor\include\EditorViewportCommandController.h](</E:/Animation Software/apps/editor/include/EditorViewportCommandController.h:1>)
* [E:\Animation Software\apps\editor\src\EditorViewportCommandController.cpp](</E:/Animation Software/apps/editor/src/EditorViewportCommandController.cpp:1>)

Ownership note:

* day la orchestration layer, khong phai rendering backend
* neu de no tron vao `Rendering`, module se bi mo boundary nhanh

## Diem pha tron hien tai trong `ViewportRenderer`

Nhung diem duoi day cho thay `ViewportRenderer` chua phai backend thuan:

* [syncScene(const Scene&)](</E:/Animation Software/src/rendering/ViewportRenderer.h:34>) doc canonical `Scene` truc tiep
* [uploadImportedMesh(...)](</E:/Animation Software/src/rendering/ViewportRenderer.cpp:463>) tu map `Scene` -> render vertices/indices
* [uploadJointGeometry(...)](</E:/Animation Software/src/rendering/ViewportRenderer.cpp:532>) tu map rig hierarchy -> line geometry
* [render(...)](</E:/Animation Software/src/rendering/ViewportRenderer.cpp:159>) ve mesh, joints, selection, gizmo trong cung mot backend class
* gizmo/joint builders dang nam inline trong [ViewportRenderer.cpp](</E:/Animation Software/src/rendering/ViewportRenderer.cpp:11>)

Y nghia:

* `ViewportRenderer` hien la `Rendering backend + render data adapter`
* neu muon sach hon nua, nen tach lop `render data builder` khoi renderer

## Diem pha tron hien tai trong `ViewportWidget`

Nhung diem duoi day cho thay `ViewportWidget` van la cluster lon:

* [pickObjectAtScreenPos(...)](</E:/Animation Software/apps/editor/src/ViewportWidget.cpp:331>)
* [pickGizmoAxisAtScreenPos(...)](</E:/Animation Software/apps/editor/src/ViewportWidget.cpp:358>)
* [applyDrag(...)](</E:/Animation Software/apps/editor/src/ViewportWidget.cpp:478>)
* [syncRendererSelection()](</E:/Animation Software/apps/editor/src/ViewportWidget.cpp:287>)
* [syncSceneToRenderer()](</E:/Animation Software/apps/editor/src/ViewportWidget.cpp:316>)
* [sceneControllerContext()](</E:/Animation Software/apps/editor/src/ViewportWidget.cpp:455>)

Y nghia:

* `ViewportWidget` hien la `widget host + picking + gizmo interaction + runtime bridge`
* no khong nen bi map 1-1 thanh `Rendering`

## Move Map De Xuat

| Uu tien | Hien trang | Move de xuat | Ly do |
| --- | --- | --- | --- |
| 1 | `ViewportRenderer` scene adaptation | `src/rendering/scene/ViewportRenderSceneAdapter.*` | xong qua `ViewportRenderSceneAdapter` |
| 2 | gizmo/joint/selection geometry builders trong `ViewportRenderer.cpp` | `src/rendering/geometry/...` hoac `src/rendering/overlay/...` | xong qua `ViewportOverlayGeometryBuilder` |
| 3 | shader source strings inline | `src/rendering/shaders/...` hoac shader string provider nho | backend gon hon, de doi backend sau nay |
| 4 | picking/gizmo math trong `ViewportWidget.cpp` | `src/viewport/interaction/...` | xong qua `ViewportInteractionMath` |
| 5 | viewport render sync bridge trong `ViewportWidget` | `src/viewport/runtime/...` neu can | xong qua `ViewportRenderSync` |

## Tieu chi xong cho `Rendering`

Co the coi `Rendering` da sach hon nua khi:

* `ViewportRenderer` khong doc `Scene` truc tiep nua
* `ViewportRenderer` nhan render-ready data thay vi tu duyet object/mesh hierarchy
* geometry builders cho gizmo/joint/selection khong nam cung file voi GL backend chinh
* `ViewportWidget` khong con om qua nhieu picking/gizmo math
* runtime scene mutation van nam o `Engine`, khong tro lai `Rendering`

## Trang thai hien tai

Hien tai `Rendering` da dat muc:

* co backend ro
* co camera seam ro
* co viewport UI seam ro

Nhung chua sach 100% vi:

* `ViewportRenderer` van la `backend + adapter`
* `ViewportWidget` van la `widget + interaction + runtime bridge`

Ket luan:

* ve backend ownership: kha ro
* ve viewport UI ownership: kha ro
* ve boundary backend <-> adapter <-> widget: van con dat de lam dep tiep
