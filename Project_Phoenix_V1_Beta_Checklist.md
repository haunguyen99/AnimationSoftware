# Project Phoenix v1 Beta Checklist

## Purpose

Checklist nay gom toan bo cong viec va tinh nang can cho `v1 beta`.

Scope beta hien tai:

* `Viewport`
* `Import FBX`
* `Orbit / Pan / Zoom`
* xem duoc model trong scene

Khong bao gom:

* animation
* timeline
* rigging tools
* plugin system
* save/load
* export

---

## 1. Environment and Tooling

## Must Have

* [x] `Visual Studio 2022` co `Desktop development with C++`
* [x] co `MSVC v143` build tools
* [x] co `Windows SDK`
* [x] co `CMake`
* [x] co `Qt 6.x msvc2022_64`
* [x] xac nhan `Qt6Config.cmake` path dung
* [x] configure CMake chay thanh cong
* [x] build app shell chay thanh cong

## Nice to Have

* [x] `Qt Creator`
* [x] `CMakePresets.json` hoan chinh cho debug/release
* [x] script build nhanh cho team

---

## 2. Application Shell

## Must Have

* [x] main window mo duoc
* [x] title app dung
* [x] central viewport host hoat dong
* [x] menu `File`
* [x] action `Import FBX`
* [x] status bar co ban

## Nice to Have

* [x] toolbar toi thieu
* [x] action `Reset Camera`
* [x] action `Frame Scene`

---

## 3. Rendering Foundation

## Must Have

* [x] tao duoc OpenGL context
* [x] viewport clear color hoat dong
* [x] viewport resize dung
* [x] render loop/update flow ro rang
* [x] depth test bat dung

## Nice to Have

* [x] basic render abstraction class
* [x] shader loading utility
* [x] GPU error logging o debug mode

---

## 4. Grid and Scene Visualization

## Must Have

* [x] render duoc grid
* [x] grid dung orientation
* [x] background viewport ro
* [x] scene rong van hien thi on

## Nice to Have

* [x] world axis indicator
* [x] wireframe toggle
* [ ] simple lighting

---

## 5. Camera System

## Must Have

* [x] co `EditorCamera`
* [x] ho tro `orbit`
* [x] ho tro `pan`
* [x] ho tro `zoom`
* [x] input mapping nhat quan
* [x] camera update khong giat lon
* [x] camera hoat dong khi chua co model

## Nice to Have

* [x] `Reset Camera`
* [x] `Frame Scene`
* [x] clamp pitch hop ly
* [x] zoom sensitivity config

---

## 6. Scene Model

## Must Have

* [x] co `Scene`
* [x] co `SceneObject`
* [x] co transform toi thieu
* [x] co mesh reference / render reference
* [x] co scene bounds
* [x] co object bounds
* [x] hierarchy toi thieu neu importer tra node tree

## Nice to Have

* [x] object id on dinh
* [x] debug dump scene tree
* [x] scene reset/clear flow

---

## 7. FBX Import Pipeline

## Must Have

* [x] tich hop `Assimp`
* [x] mo file picker
* [x] chon duoc file `.fbx`
* [x] importer parse duoc `FBX`
* [x] convert tu importer data -> internal scene model
* [x] tao renderable mesh data
* [x] tinh scene bounds sau import
* [x] import xong viewport redraw

## Nice to Have

* [x] import progress log
* [x] import stats: node count / mesh count / triangle count
* [x] support nhieu mesh trong 1 file

---

## 8. Model Rendering

## Must Have

* [x] model imported duoc render trong viewport
* [x] transform co ban ap dung
* [x] mesh khong bi invisible do camera/setup sai
* [x] material fallback mac dinh hoat dong

## Nice to Have

* [x] vertex color neu asset co
* [x] simple normals-based shading
* [x] backface culling config

---

## 9. Fit to Model

## Must Have

* [x] sau import camera focus vao model
* [x] target = center bounds
* [x] distance tinh theo size model
* [x] model khong spawn ngoai view mac dinh

## Nice to Have

* [x] manual `Frame Scene` command
* [x] reframe khi import file moi

---

## 10. Error Handling and Logging

## Must Have

* [x] app khong crash khi import file loi
* [x] app khong crash khi scene rong
* [x] hien loi ngan, ro cho user
* [x] log loi import chi tiet hon
* [x] log startup
* [x] log viewport init fail

## Nice to Have

* [x] log categories ro: `app`, `viewport`, `io`, `fbx`
* [x] debug asserts cho invalid state

---

## 11. Sample Assets

## Must Have

* [x] co it nhat 1 file `FBX` test co ban
* [x] co note mo ta asset dung de test gi
* [x] asset nhe, de commit, hop phap

## Nice to Have

* [x] 1 static mesh sample
* [x] 1 hierarchy sample
* [x] 1 file loi/corrupt de test fail path

---

## 12. Testing

## Must Have

* [x] manual test scene rong
* [x] manual test import file hop le
* [x] manual test orbit/pan/zoom
* [x] manual test resize viewport
* [x] manual test import file loi
* [x] manual test import file thu hai

## Nice to Have

* [ ] unit test camera math
* [ ] unit test bounds calculation
* [ ] integration test import result validation

---

## 13. Documentation

## Must Have

* [x] `CURRENT_STATE.md` cap nhat dung tien do
* [x] ADR con khop implementation
* [x] setup guide dung voi moi truong thuc te
* [x] checklist nay duoc cap nhat khi scope doi

## Nice to Have

* [x] troubleshooting doc cho build
* [x] screenshot beta dau tien

---

## 14. Out of Scope Guardrail

Khong lam trong beta nay:

* [ ] animation system
* [ ] timeline UI
* [ ] graph editor
* [ ] rigging tool authoring
* [ ] plugin runtime
* [ ] save/load project
* [ ] export `FBX`

Muc nay dung de tu nhac team khong scope creep.

---

## 15. Beta Definition of Done

`v1 beta` duoc xem la done khi tat ca muc duoi day hoan tat:

* [x] app build duoc
* [x] app launch duoc
* [x] viewport hien grid
* [x] import duoc file `FBX` test
* [x] model hien trong viewport
* [x] orbit / pan / zoom hoat dong
* [x] camera tu fit model sau import
* [x] import loi khong crash
* [x] manual QA pass cac luong chinh
* [x] tai lieu trang thai duoc cap nhat
