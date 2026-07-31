# Development History

Tai lieu nay gom lai lich su phat trien cua `Project Phoenix` tinh den ngay `2026-07-31`.

Muc tieu:

* tao mot timeline de quay lai du an nhanh
* giu boi canh cho cac dot refactor sau
* tach ro `lich su san pham` va `lich su kien truc`
* giam viec moi lan planning lai phai doan "vi sao codebase dang o hinh dang nay"

Tai lieu nay khong thay the:

* `CURRENT_STATE.md` cho working memory hien tai
* ADR cho quyet dinh ky thuat da khoa
* roadmap cho dinh huong tuong lai

No dong vai tro:

* `past record`
* `change narrative`
* `architecture evolution log`

## Nguon Moc Thoi Gian

Tai lieu nay duoc tong hop tu:

* `git log` hien co trong repo
* cac roadmap va checklist trong `docs/`
* cum boundary map / architecture docs da duoc cap nhat den cuoi thang `07/2026`
* trang thai codebase va ket qua refactor dang ton tai trong source tree

Co 2 loai moc:

* `moc co ngay ro rang` = co the neo bang commit hoac tai lieu
* `moc suy ra` = duoc tong hop tu nhieu tai lieu cung ky, nhung khong dai dien cho 1 commit duy nhat

## Timeline Tong Quan

| Date | Milestone | Y nghia |
| --- | --- | --- |
| `2026-07-05` | Khoi tao repo va editor shell foundation | Bat dau duong xay editor desktop dung `Qt Widgets + OpenGL` |
| `2026-07-12` | Scene workflow + animation foundation | Scene authoring, timeline/playback shell, animation data core vao hinh |
| `2026-07-13` | `v0.4 Rigging Foundation phase 1` | `Joint`, hierarchy, orientation, bind pose tro thanh domain that |
| `2026-07-23` | Tach document/scene/history seams va timeline UI seams | `EditorShell` bot om logic, editor seam pattern bat dau ro |
| `2026-07-24` | Tach editor shell seams, chot ownership | doi ten va dinh hinh `MainWindow -> EditorShell` ro hon |
| `2026-07-25` | Tach engine/runtime seams | `Engine` bat dau co hinh dang module dieu phoi |
| `2026-07-26` den `2026-07-31` | Boundary-map wave + viewport gizmo wave | Khoa lai module boundaries, modularize `Rendering/Animation/Rigging/Scene`, va nang cap object manipulation trong viewport |

## Phase 1: Editor Foundation

### `2026-07-05`

Moc neo:

* commit `dfd0f6b` `Initial commit`
* ADR nen tang bat dau duoc khoa trong cum `ADR-001` toi `ADR-006`

Thanh qua chinh:

* dat nen stack `C++ + CMake + Qt + OpenGL`
* co app shell dau tien trong `apps/editor`
* co viewport, camera, import pipeline, outliner, inspector host
* tao bo khung de xay editor theo workflow desktop, khong di theo game runtime

Y nghia kien truc:

* chon `Qt Widgets + QOpenGLWidget` cho beta
* chon `Scene` noi bo la canonical model
* chon `OpenGL` sau seam mong cho rendering

Tai lieu nen doc kem:

* [docs/adr/ADR-001-core-technology-stack-and-build-system.md](</E:/Animation Software/docs/adr/ADR-001-core-technology-stack-and-build-system.md:1>)
* [docs/adr/ADR-002-beta-editor-shell-uses-qt-widgets-with-opengl-viewport-host.md](</E:/Animation Software/docs/adr/ADR-002-beta-editor-shell-uses-qt-widgets-with-opengl-viewport-host.md:1>)
* [docs/adr/ADR-003-internal-scene-model-uses-minimal-tree-based-structure-for-beta.md](</E:/Animation Software/docs/adr/ADR-003-internal-scene-model-uses-minimal-tree-based-structure-for-beta.md:1>)

## Phase 2: Scene Workflow And Animation Base

### `2026-07-12`

Moc neo:

* commit `119197f` `Add editor workflow and scene animation foundations`

Thanh qua chinh:

* scene workflow vao duoc:
  * new/open/save/export
  * timeline shell
  * playback shell
  * command-driven workflow
* animation foundation vao duoc:
  * per-object transform tracks
  * set key / auto key / delete key
  * playback evaluate animated transforms
  * timeline key markers

Y nghia san pham:

* Phoenix khong con chi la shell inspect scene
* Phoenix bat dau tro thanh animation editor that su

Y nghia kien truc:

* `Animation` luc nay van song gan `Scene`, nhung da co ly do domain ro
* command system tro thanh seam quan trong cho automation va script

Tai lieu nen doc kem:

* [docs/Development_Roadmap.md](</E:/Animation Software/docs/Development_Roadmap.md:1>)
* [docs/Animation_Tools_V0_6_Roadmap.md](</E:/Animation Software/docs/Animation_Tools_V0_6_Roadmap.md:1>)

## Phase 3: Rigging Foundation

### `2026-07-13`

Moc neo:

* commit `de5d9f8` `Implement v0.4 rigging foundation phase 1`
* ADR `ADR-009` khoa `joint orientation` tach khoi animated rotation

Thanh qua chinh:

* co `Joint` object model
* co skeleton hierarchy authoring
* co parent / unparent va cycle reject
* co `Joint Orientation`
* co `Bind Pose`
* co skeleton visualization trong viewport

Y nghia san pham:

* project di tu object animation sang character/rig workflow
* dat nen cho `Skinning Foundation v0.5`

Y nghia kien truc:

* `Rigging` tro thanh mot domain dung ten, khong con la tap hop utility quanh `Scene`
* ngon ngu trong `CONTEXT.md` bat dau co suc nang cao hon voi code va docs

Tai lieu nen doc kem:

* [docs/adr/ADR-009-joint-orientation-stays-separate-from-animated-rotation.md](</E:/Animation Software/docs/adr/ADR-009-joint-orientation-stays-separate-from-animated-rotation.md:1>)
* [docs/Rigging_Module_Boundary_Map.md](</E:/Animation Software/docs/Rigging_Module_Boundary_Map.md:1>)

## Phase 4: Skinning And Early Animation Tooling

### `2026-07-13` den `2026-07-22` `moc suy ra`

Moc neo:

* duoc tong hop tu `CURRENT_STATE.md`, `Development_Roadmap.md`, va cac tai lieu `v0.5` / `v0.6`

Thanh qua chinh:

* `v0.5 Skinning` vao duoc:
  * skin bind data
  * deformation runtime
  * serialization
  * weight normalization core
* `v0.6 Phase 1` khoi dong:
  * duplicate key
  * shift keys
  * previous/next key navigation

Y nghia san pham:

* pipeline `rig -> bind -> animate -> preview` bat dau lien mach

Y nghia kien truc:

* `Scene` van la canonical host rat manh
* nhung suc ep tach module bat dau ro, vi `Scene` dang gom qua nhieu trach nhiem

Tai lieu nen doc kem:

* [CURRENT_STATE.md](</E:/Animation Software/CURRENT_STATE.md:1>)
* [docs/Animation_Tools_V0_6_Roadmap.md](</E:/Animation Software/docs/Animation_Tools_V0_6_Roadmap.md:1>)

## Phase 5: Seams Extraction Wave

### `2026-07-23`

Moc neo:

* commit `4b09000` `Extract editor document scene and history seams`
* commit `2bbffb2` `Extract animation timeline UI and script seams`

Thanh qua chinh:

* tach document / scene / history responsibilities khoi cum editor cu
* tach timeline UI-building khoi mot phan shell logic
* tang su hien dien cua pattern:
  * `Controller`
  * `FlowController`
  * `ViewBuilder`
  * `SceneMutation`

Y nghia kien truc:

* day la dot ma codebase bat dau di ro theo huong `editor seam`
* `apps/editor` van con la adapter lon, nhung khong con la noi duy nhat giu logic

### `2026-07-24`

Moc neo:

* commit `8cf6215` `Extract editor shell seams and document ownership`

Thanh qua chinh:

* dinh hinh lai ownership cua application shell
* doi ten ngon ngu kien truc tu `MainWindow` sang `EditorShell`
* cap nhat tai lieu de thoat khoi tu duy `single giant window class`

Y nghia kien truc:

* day la moc chuyen tu ten lop UI cu sang ten module co y nghia hon
* rat quan trong cho cac vong refactor ve sau, vi no doi ca language map cua repo

Tai lieu nen doc kem:

* [docs/MainWindow_Core_Extraction_Checklist.md](</E:/Animation Software/docs/MainWindow_Core_Extraction_Checklist.md:1>)
* [docs/Editor_Shell_Ownership_Note.md](</E:/Animation Software/docs/Editor_Shell_Ownership_Note.md:1>)

### `2026-07-25`

Moc neo:

* commit `3a4112e` `Extract editor engine runtime seams`

Thanh qua chinh:

* `Engine` khong con chi la y tuong trong roadmap
* co cac seam runtime/orchestration ro hon
* dat nen cho playback, viewport-scene coordination, va future authoring orchestration

Y nghia kien truc:

* huong di `Core -> Engine -> Scene/Animation/Rigging/Rendering` bat dau co hinh dang that
* `EditorShell` giam bot vai tro "lam tat ca"

Tai lieu nen doc kem:

* [docs/Module_Dependency_Diagram.md](</E:/Animation Software/docs/Module_Dependency_Diagram.md:1>)
* [docs/Architecture_Scaling_Roadmap.md](</E:/Animation Software/docs/Architecture_Scaling_Roadmap.md:1>)

## Phase 6: Module Boundary Consolidation

### `2026-07-26` den `2026-07-31` `moc suy ra`

Day la dot khong phai chi them feature, ma la khoa lai cach doc toan bo repo theo module.

Thanh qua chinh:

* ra soat va map lai:
  * `Scene`
  * `Rendering`
  * `Animation`
  * `Rigging`
* dong bo tai lieu boundary map theo cung format va muc chi tiet
* chot lai architecture stack:
  * `Core`
  * `Engine`
  * `Scene`
  * `Rendering`
  * `Animation`
  * `Rigging`
* bo sung tai lieu:
  * dependency graph
  * code map
  * architecture document map
  * scaling roadmap

Y nghia kien truc:

* day la moc repo tu "dang refactor" sang "co he tai lieu kien truc de tiep tuc phat trien"
* future work gio co the dua tren module ownership thay vi dua tren nho nho tong quat

Tai lieu nen doc kem:

* [docs/Architecture_Document_Map.md](</E:/Animation Software/docs/Architecture_Document_Map.md:1>)
* [docs/Scene_Module_Boundary_Map.md](</E:/Animation Software/docs/Scene_Module_Boundary_Map.md:1>)
* [docs/Rendering_Module_Boundary_Map.md](</E:/Animation Software/docs/Rendering_Module_Boundary_Map.md:1>)
* [docs/Animation_Module_Boundary_Map.md](</E:/Animation Software/docs/Animation_Module_Boundary_Map.md:1>)
* [docs/Rigging_Module_Boundary_Map.md](</E:/Animation Software/docs/Rigging_Module_Boundary_Map.md:1>)

## Phase 7: Viewport Object Manipulation And Gizmo Wave

### `2026-07-26` den `2026-07-31` `moc suy ra`

Moc nay chay song song voi dot boundary consolidation, nhung nen duoc tach rieng vi no la bien doi lon o workflow user-facing.

Thanh qua chinh:

* viewport object picking va selection sync duoc cung co
* `Translate` gizmo da co:
  * axis move
  * plane move
  * free move
* `Rotate` gizmo da co:
  * X / Y / Z rotation
  * screen-space outer ring
  * back-facing arc hidden
* `Scale` gizmo da co:
  * axis scale
  * plane scale
  * uniform scale
* toi uu drag responsiveness:
  * interactive scene transform update
  * preview subtree rendering
  * throttled UI notification
* modularize gizmo thanh 3 lop ownership:
  * `interaction`
  * `geometry`
  * `drag`

Y nghia san pham:

* Phoenix chuyen ro hon tu "scene editor co viewport" thanh "editor co object manipulation workflow that"

Y nghia kien truc:

* `ViewportWidget` da duoc giam phan nao khoi mot cum logic lon
* `Rendering + Viewport UI` co boundary ro hon cho nhung lan nang cap sau

Tai lieu nen doc kem:

* [docs/Viewport_Object_Manipulation_Design.md](</E:/Animation Software/docs/Viewport_Object_Manipulation_Design.md:1>)
* [apps/editor/src/ViewportWidget.cpp](</E:/Animation Software/apps/editor/src/ViewportWidget.cpp:1>)
* [src/viewport/gizmo/GizmoTypes.h](</E:/Animation Software/src/viewport/gizmo/GizmoTypes.h:1>)

## Lich Su Kien Truc Theo Chu De

Neu khong muon doc theo ngay, co the nhin lich su theo 5 chuyen dong lon sau.

### 1. Tu `MainWindow` sang `EditorShell`

Day la chuyen dong doi ngon ngu va doi ownership.

Truoc:

* de nghi ve mot lop UI trung tam qua lon
* de keo them logic vao cung mot cho

Sau:

* nhin shell nhu mot `workflow surface`
* tach dan responsibilities thanh seams
* de dat ten va tai lieu theo module thay vi theo widget cu the

### 2. Tu `Scene` om logic sang `Scene` + domain seams

Truoc:

* `Scene` va `SceneObject` la noi gom nhieu logic animation/rigging/runtime

Sau:

* `Animation` da co state va scene seam rieng
* `Rigging` da co state va controller rieng
* `Scene` van la canonical host, nhung muc tieu dai han la facade mong hon

### 3. Tu editor monolith sang `editor seam pattern`

Pattern bat dau lap lai duoc:

* `Controller`
* `FlowController`
* `SceneMutation`
* `ViewBuilder`

Gia tri:

* de du doan ownership
* de refactor lap lai cach lam
* de feature moi it quay lai gom vao shell

### 4. Tu rendering ad-hoc sang `Rendering` module ro hon

Tien trinh:

* renderer surface ro hon
* scene adapter ro hon
* overlay/gizmo geometry duoc tach
* preview path cho interactive manipulation duoc dua vao renderer

Gia tri:

* viewport feature sau nay de mo rong hon
* giam nguy co drag logic va draw logic dan dinh vao nhau

### 5. Tu viewport inspect sang viewport manipulate

Tien trinh:

* luc dau viewport chu yeu de xem scene, frame, va chon object
* sau do co object picking
* roi co transform gizmo
* roi toi uu responsiveness va modularize interaction

Gia tri:

* user flow animation/rigging tro nen "thao tac truc tiep" hon
* viewport tro thanh trung tam cua editor, khong chi la panel preview

## Cach Dung Tai Lieu Nay Ve Sau

Khi quay lai du an sau mot thoi gian:

1. doc file nay truoc de nho lai duong tien hoa
2. doc [CURRENT_STATE.md](</E:/Animation Software/CURRENT_STATE.md:1>) de biet trang thai hien tai
3. doc [docs/Architecture_Document_Map.md](</E:/Animation Software/docs/Architecture_Document_Map.md:1>) de di sau vao boundary hien tai

Khi mo mot dot refactor moi:

1. xac dinh thay doi do dang tiep noi phase nao trong lich su nay
2. kiem tra no co dang pha vo huong `MainWindow -> EditorShell`, `Scene -> seams`, hay `Viewport -> modules` hay khong
3. neu co, cap nhat lai tai lieu nay sau khi xong

## Muc Nen Cap Nhat Sau Moi Dot Lon

Sau moi dot phat trien lon, nen cap nhat toi thieu 4 muc:

* `Timeline Tong Quan`
* phase moi neu thay doi du lon
* `Lich Su Kien Truc Theo Chu De`
* link tai lieu lien quan

## Ket Luan

Tinh den `2026-07-31`, `Project Phoenix` da di qua 3 lop tien hoa ro rang:

* tu editor foundation sang animation/rigging-capable tool
* tu codebase tap trung quanh shell/scene sang module boundaries ro hon
* tu viewport inspect sang viewport manipulation co kha nang mo rong

Neu giu tai lieu nay song song voi `CURRENT_STATE.md` va cum boundary map, cac dot phat trien sau se de lap ke hoach va it mat boi canh hon rat nhieu.
