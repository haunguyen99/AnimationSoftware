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

* `v1 beta`
* co `Viewport`
* import duoc `FBX`
* orbit / pan / zoom de xem model

---

## Current Product Scope

Tai lieu scope hien dung:

* [Project_Phoenix_Overview.md](</E:/Animation Software/Project_Phoenix_Overview.md>)
* [Project_Phoenix_Technical_Spec_v1.md](</E:/Animation Software/Project_Phoenix_Technical_Spec_v1.md>)
* [Project_Phoenix_V1_Beta_Viewport_FBX_Spec.md](</E:/Animation Software/Project_Phoenix_V1_Beta_Viewport_FBX_Spec.md>)
* [Project_Phoenix_V1_Beta_Checklist.md](</E:/Animation Software/Project_Phoenix_V1_Beta_Checklist.md>)
* [docs/Beta_Manual_QA.md](</E:/Animation Software/docs/Beta_Manual_QA.md>)

Scope active hien tai:

* beta nho, khong animation
* khong timeline
* khong plugin impl
* khong save/load

Quy trinh feature chinh thuc:

* [FEATURE_WORKFLOW.md](</E:/Animation Software/FEATURE_WORKFLOW.md>)
* `Step 1 = Design only`
* chua duyet design -> chua code

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

Chua co:

* tests tu dong that

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

1. can nhac test tu dong cho camera / bounds / import validation
2. can nhac polish them thong diep loi than thien cho corrupt FBX
3. neu khong mo rong scope, co the xem `v1 beta` da dong

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
