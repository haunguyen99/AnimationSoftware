# Project Phoenix v0.2 Checklist

## Purpose

Checklist nay gom cong viec va guardrail cho `v0.2`.

Scope `v0.2`:

* `Editor Shell`
* `Scene Outliner`
* `Inspector`
* `Selection`
* `Frame Selected`
* `Import Hardening`
* `Test Foundation`

Khong bao gom:

* rigging tools
* animation authoring
* timeline
* graph editor
* save/load
* export

---

## 1. Application Shell

## Must Have

* [x] app shell hien tai van build duoc
* [x] app launch duoc sau thay doi `v0.2`
* [x] viewport van la khu vuc trung tam
* [x] menu / toolbar hien co khong bi vo
* [x] status bar van cap nhat duoc thong diep

## Nice to Have

* [x] them action `Frame Selected`
* [ ] them shortcut hop ly cho flow inspect

---

## 2. Scene Outliner

## Must Have

* [x] co panel `Outliner`
* [x] hien root objects trong scene
* [x] hien child hierarchy toi thieu
* [x] refresh dung sau moi lan import
* [x] support single selection
* [x] xu ly duoc scene rong

## Nice to Have

* [ ] auto expand root sau import
* [ ] object label ro rang khi node khong co ten dep

---

## 3. Inspector

## Must Have

* [x] co panel `Inspector`
* [x] hien trang thai `no selection`
* [x] hien `name`
* [x] hien `parent`
* [x] hien `child count`
* [x] hien `mesh count`
* [x] hien `local transform`
* [x] hien `local bounds`
* [x] hien `world bounds`

## Nice to Have

* [ ] format gia tri so de doc
* [ ] tach thong tin theo section ro rang

---

## 4. Selection Flow

## Must Have

* [x] click object trong `Outliner` -> object duoc selected
* [x] `Inspector` cap nhat theo object selected
* [x] viewport co selected feedback ro
* [x] `Frame Selected` focus dung object duoc chon
* [x] bo chon an toan khi scene doi hoac object khong con ton tai

## Nice to Have

* [ ] giu selection hop ly sau import append
* [x] status bar hien object dang selected

---

## 5. Viewport Feedback

## Must Have

* [x] object selected duoc nhan biet trong viewport
* [x] selected feedback khong lam hong flow render hien tai
* [x] `Frame Selected` fallback an toan neu object khong co bounds hop le

## Nice to Have

* [ ] selected feedback de thay tren asset nhieu mesh
* [ ] color / wire overlay nhat quan voi look hien tai

---

## 6. Import UX

## Must Have

* [x] import thanh cong van hoat dong nhu beta
* [x] status bar hien file name sau import
* [x] status bar hien `node count / mesh count / triangle count`
* [x] `Outliner` dong bo dung voi scene moi
* [x] import fail khong clear nham scene hien tai
* [x] thong diep loi than thien hon cho user
* [x] log van giu thong tin chi tiet cho dev

## Nice to Have

* [x] thong diep fail phan biet duoc `file not found / wrong extension / parse fail`
* [x] import summary de doc hon beta hien tai

---

## 7. FBX Import Module Refactor

## Must Have

* [x] giu interface `FbxImporter` gon cho caller
* [x] tach file validation khoi import orchestration
* [x] tach `Assimp` parse adapter khoi scene conversion
* [x] tach import report formatting khoi parse path
* [x] hanh vi import success giu on dinh so voi beta
* [x] hanh vi import fail giu an toan so voi beta

## Nice to Have

* [x] module names ro rang theo domain
* [x] implementation de test doc lap hon

---

## 8. Scene and Core Data Safety

## Must Have

* [ ] `Scene` van append duoc scene imported thu hai
* [ ] hierarchy khong bi hong sau append
* [ ] object bounds van hop le sau import
* [x] `Frame Selected` dua tren `world bounds` dung

## Nice to Have

* [ ] co them debug assert cho invalid selection state
* [ ] co helper nho cho query scene data phuc vu `Outliner` va `Inspector`

---

## 9. Testing

## Must Have

* [ ] tao duoc `tests/unit` that, khong con placeholder
* [ ] unit test cho `EditorCamera`
* [ ] unit test cho `Bounds3D`
* [ ] unit test cho `Scene::appendScene`
* [ ] test cho import validation co ban

## Nice to Have

* [ ] integration test import `box_static.fbx`
* [ ] integration test import `hierarchy_multi_mesh.fbx`
* [ ] integration test fail voi `corrupt_minimal.fbx`

---

## 10. Manual QA

## Must Have

* [x] mo app khi scene rong -> UI panels on
* [x] import file hop le -> `Outliner` dung
* [x] chon object -> `Inspector` dung
* [x] `Frame Selected` hoat dong
* [x] import file loi -> app khong crash
* [x] import file thu hai -> scene va selection van an toan

## Nice to Have

* [ ] test asset nhieu mesh
* [ ] test object khong co mesh
* [ ] test object co hierarchy sau

---

## 11. Documentation

## Must Have

* [x] scope doc `v0.2` con khop implementation
* [x] `CURRENT_STATE.md` cap nhat sang phase moi
* [x] checklist nay duoc update khi scope doi

## Nice to Have

* [ ] them screenshot shell moi sau khi on
* [ ] them note QA cho flow selection / inspector

---

## 12. Out of Scope Guardrail

Khong lam trong `v0.2`:

* [ ] rigging authoring
* [ ] animation timeline
* [ ] keyframe editing
* [ ] graph editor
* [ ] skeleton editing UI
* [ ] save/load project
* [ ] export `FBX`

Muc nay dung de chan scope creep.

---

## 13. Definition of Done

`v0.2` duoc xem la done khi tat ca muc duoi day hoan tat:

* [x] app build duoc
* [x] app launch duoc
* [x] import `FBX` van on nhu beta
* [x] co `Outliner`
* [x] co `Inspector`
* [x] selection flow hoat dong
* [x] `Frame Selected` hoat dong
* [x] selected feedback trong viewport ro
* [x] import fail khong crash va khong pha scene hien tai
* [ ] co test tu dong dau tien cho core modules da chon
* [x] tai lieu trang thai duoc cap nhat
