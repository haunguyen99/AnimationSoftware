# Current State

## Purpose

File nay la `working memory` ben cho `Project Phoenix`.

Dung de:

* bat nhip nhanh khi quay lai du an
* tranh phu thuoc vao lich su chat dai
* chot `da xong / dang lam / lam tiep`

Moi khi co thay doi lon, phai cap nhat file nay.

---

## Project Summary

`Project Phoenix` dang duoc dinh huong thanh tool `Rigging + Animation` hien dai cho `Game Animator`.

Scope dang tap trung truoc mat:

* `v0.4`
* `rigging foundation`
* skeleton hierarchy authoring
* parenting va joint orientation co ban

---

## Current Product Scope

Tai lieu scope hien dung:

* [Project_Phoenix_Overview.md](</E:/Animation Software/Project_Phoenix_Overview.md>)
* [Project_Phoenix_Technical_Spec_v1.md](</E:/Animation Software/Project_Phoenix_Technical_Spec_v1.md>)
* [Project_Phoenix_V1_Beta_Viewport_FBX_Spec.md](</E:/Animation Software/Project_Phoenix_V1_Beta_Viewport_FBX_Spec.md>)
* [Project_Phoenix_V1_Beta_Checklist.md](</E:/Animation Software/Project_Phoenix_V1_Beta_Checklist.md>)
* [Project_Phoenix_V0_2_Editor_Shell_Scope.md](</E:/Animation Software/Project_Phoenix_V0_2_Editor_Shell_Scope.md>)
* [Project_Phoenix_V0_2_Editor_Shell_Checklist.md](</E:/Animation Software/Project_Phoenix_V0_2_Editor_Shell_Checklist.md>)
* [docs/Beta_Manual_QA.md](</E:/Animation Software/docs/Beta_Manual_QA.md>)

Scope active hien tai:

* da co `scene workflow` va `animation data foundation`
* co `Outliner`
* co `Channel Box`
* co `Timeline`
* co `Playback`
* co `Script Editor`
* co `save/load scene`
* co `Set Key` / `Auto Key`
* co import pipeline
* da co `joint` / skeleton authoring foundation
* da co parenting / unparenting / hierarchy drag trong `Outliner`
* da co `joint orientation` basics va `bind pose` capture workflow

Quy trinh feature chinh thuc:

* [FEATURE_WORKFLOW.md](</E:/Animation Software/FEATURE_WORKFLOW.md>)
* `v0.4` da qua `design -> implementation plan -> implementation phase 1`

---

## Architecture Decisions

ADR index:

* [docs/adr/README.md](</E:/Animation Software/docs/adr/README.md>)

ADR da chot:

* `ADR-001` core stack = `C++20/23 + CMake + Qt + OpenGL + GLM + spdlog + GoogleTest`
* `ADR-002` beta shell = `Qt Widgets + QOpenGLWidget`
* `ADR-003` scene model beta = minimal tree-based
* `ADR-004` `FBX` la external format, internal scene la canonical
* `ADR-005` rendering bat dau bang `OpenGL` sau abstraction mong
* `ADR-006` importer beta dung `Assimp`
* `ADR-007` repo layout theo feature-oriented folders
* `ADR-008` testing strategy + sample asset policy
* `ADR-009` `joint orientation` tach khoi animated rotation

---

## Repo Status

Da co:

* root `CMakeLists.txt`
* root `CMakePresets.json`
* app shell tai `apps/editor/`
* `MainWindow`
* `ViewportWidget`
* `EditorCamera`
* `ViewportRenderer`
* `ShaderUtils`
* `Scene`
* `SceneObject`
* `Bounds3D`
* `Transform`
* `FbxImporter`
* `src/logging/LogCategories.*`
* `docs/adr/`
* `docs/Beta_Manual_QA.md`
* `docs/TROUBLESHOOTING.md`
* `docs/Windows_Development_Setup.md`
* `docs/images/beta-first-screenshot.png`
* `assets/sample/README.md`
* `assets/sample/box_static.fbx`
* `assets/sample/hierarchy_multi_mesh.fbx`
* `assets/sample/corrupt_minimal.fbx`
* `tools/build_debug.ps1`
* `tools/deploy_qt_runtime.ps1`
* scope doc [Project_Phoenix_V0_2_Editor_Shell_Scope.md](</E:/Animation Software/Project_Phoenix_V0_2_Editor_Shell_Scope.md>)
* checklist [Project_Phoenix_V0_2_Editor_Shell_Checklist.md](</E:/Animation Software/Project_Phoenix_V0_2_Editor_Shell_Checklist.md>)
* design doc [docs/Application_Shell_V0_2_Design.md](</E:/Animation Software/docs/Application_Shell_V0_2_Design.md>)
* implementation plan [docs/Application_Shell_V0_2_Implementation_Plan.md](</E:/Animation Software/docs/Application_Shell_V0_2_Implementation_Plan.md>)
* `Application Shell` `v0.2` da xong layout host ban dau
* `Outliner` dock host ben trai
* `Inspector` dock host ben phai
* empty state cho shell panels
* `Outliner` single-selection flow
* wrapper `RootNode` duoc an o layer `Outliner` khi chi la node gom 1 child va khong co mesh
* `Inspector` read-only detail fields
* `Frame Selected` action/menu/toolbar/button
* selected object feedback bang viewport bounds overlay
* import summary va import error message da duoc polish them
* `FBX import module` da tach theo validation / parse adapter / scene builder / message formatting
* `PrimitiveMeshFactory`
* `ScriptCommandSystem`
* `KeyframeTimelineWidget`
* `PhoenixSceneDocument`
* animation data model theo object
* timeline + playback shell
* `Set Key` / `Delete Key` / `Auto Key`
* save/open scene workflow
* export scene workflow
* command support cho timeline, keyframe va file workflow
* automated tests trong `tests/unit`

Chua co:

* skin bind / weight workflow
* mesh deformation evaluation
* orientation presets nang cao
* dedicated rig panel ngoai `Channel Box`

Da xong end-to-end:

* import duoc file `FBX` that
* model hien trong viewport
* fit camera sau import hoat dong
* support nhieu mesh trong 1 file
* append file thu hai vao scene
* fail path import da pass:
  * cancel import
  * chon sai duoi file
  * chon file hong
  * import fail xong van import lai file tot duoc
  * import fail khong lam hong viewport interaction
* startup logging + viewport init logging
* logging categories: `app`, `viewport`, `io`, `fbx`
* debug asserts cho importer / scene append / renderer upload paths
* bo sample assets toi thieu cho beta QA
* manual QA theo [docs/Beta_Manual_QA.md](</E:/Animation Software/docs/Beta_Manual_QA.md>) da pass
* scope `v0.2` da duoc chot
* `Application Shell` da qua design -> implementation plan -> implementation -> build verification
* `Outliner` runtime test pass
* `Inspector` runtime test pass
* `Selection Flow` runtime test pass
* `Frame Selected` runtime test pass
* `Viewport Feedback` runtime test pass
* `Import UX` runtime test pass
* `FBX Import Module Refactor` build + runtime test pass
* `Scene Workflow` save/open/export pass
* `Animation Data Foundation` pass:
  * keyframe data model
  * per-object transform tracks
  * playback evaluate animated transforms
  * `Set Key` / `Auto Key` / `Delete Key`
  * timeline key markers
  * script commands cho key va playback
* `Rigging Foundation v0.4 phase 1` pass:
  * `joint` object model
  * hierarchy parent / unparent + cycle reject
  * keep-world-transform on reparent
  * save/load `jointOrientation` + `bindPose`
  * viewport skeleton visualization
  * `joint`, `parent`, `unparent`, `jointOrient`, `bindPose` command paths
  * `Channel Box` workflow cho orientation va bind pose capture
* `ctest` trong `build/ninja-msvc-debug` pass `2/2`

---

## Environment Status

Da xac nhan:

* co `Visual Studio 2022`
* co `MSVC v143`
* co `Windows SDK`
* `CMake` da cai, tim thay tai `C:\Program Files\CMake\bin\cmake.exe`
* `Qt` da cai vao `C:\Qt`
* hien co `Qt 6.11.1`
* kit desktop dung da co tai `C:\Qt\6.11.1\msvc2022_64`
* may co `Qt 5.15.2` tu Maya nhung khong dung cho project nay
* configure CMake chay thanh cong qua `VsDevCmd + Ninja`
* `CMakePresets.json` da chuyen sang route `Ninja`
* `cmake --preset msvc2022-debug` chay duoc trong `VsDevCmd`
* build app shell chay thanh cong tai `build/ninja-msvc-debug`
* Qt runtime da deploy bang `windeployqt`
* app shell launch truc tiep duoc sau deploy runtime

---

## Immediate Next Steps

Uu tien gan nhat:

1. manual QA them cho `v0.4 phase 1` trong app
2. quyet scope `v0.4 phase 2` neu co:
   * object -> joint socket workflow
   * orientation preset / up-axis policy ro hon
   * inspector polish cho rig authoring
3. chuan bi design cho `v0.5 Skinning`

---

## Working Rules

Khi quay lai du an:

1. doc file nay truoc
2. doc ADR lien quan
3. doc spec dang active
4. chi doc chat cu neu con diem mo ho

Khi hoan thanh task dang ke:

* cap nhat `Repo Status`
* cap nhat `Environment Status`
* cap nhat `Immediate Next Steps`
