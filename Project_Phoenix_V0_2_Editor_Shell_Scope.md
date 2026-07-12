# Project Phoenix v0.2 Scope

## 1. Goal

`v0.2` = buoc tiep theo sau `v1 beta` de bien `FBX viewer beta` thanh `editor shell` nho, on dinh, de mo rong sang phase animation.

Ban nay khong nham giao full tool `Rigging + Animation`.

Ban nay nham:

* lam scene de inspect hon
* tao flow chon object ro rang
* cung co import pipeline
* dat nen cho test va refactor truoc phase lon hon

---

## 2. Scope

## In Scope

* `Scene Outliner` toi thieu
* `Inspector` read-only cho object duoc chon
* object selection qua `Outliner`
* frame selected object
* viewport highlight selected object o muc co ban
* import summary ro rang hon
* import error messaging than thien hon
* test foundation cho `camera`, `bounds`, `scene append`, `import`
* refactor `FBX import module` theo huong parse / convert / report

## Out of Scope

* rigging tools
* animation authoring
* timeline
* keyframe editing
* graph editor
* skeleton editing UI
* save/load project
* export `FBX`
* plugin runtime that
* material editor day du

---

## 3. Product Intent

Sau `v1 beta`, user da xem duoc model.

`v0.2` phai giup user:

* biet scene dang co gi
* chon tung object trong scene
* xem thong tin co ban cua object
* frame nhanh object can xem
* hieu ro import vua tao ra ket qua gi

Neu luong nay on -> `Project Phoenix` co `editor shell` that, san sang cho phase animation-readiness tiep theo.

---

## 4. User Stories

### Story 1

User mo app -> import `FBX` -> thay `Outliner` hien node tree -> click vao object -> viewport frame va highlight object -> `Inspector` hien thong tin object.

### Story 2

User import file loi hoac file co van de -> app bao thong diep de hieu -> scene hien tai khong hong -> user tiep tuc import file khac duoc.

### Story 3

Dev chinh import internals hoac camera math -> co test surface de bat loi som -> giam risk pha vo ban shell.

---

## 5. Success Criteria

`v0.2` dat khi:

* user import duoc `FBX` nhu beta
* `Outliner` hien hierarchy toi thieu cua `SceneObject`
* user chon duoc object tu `Outliner`
* `Inspector` hien it nhat:
  * `name`
  * `parent`
  * `child count`
  * `mesh count`
  * `local bounds`
  * `world bounds`
  * `local transform`
* co `Frame Selected`
* object selected duoc nhan biet ro trong viewport
* import fail van khong crash app
* co test tu dong dau tien cho phan core da chot trong `ADR-008`

---

## 6. Functional Requirements

## 6.1 Scene Outliner

`Outliner` phai:

* hien root objects
* hien child hierarchy toi thieu
* support single selection
* dong bo voi `Scene` hien tai sau moi lan import
* khong can drag-drop reparent
* khong can multi-select

## 6.2 Inspector

`Inspector` phai:

* hien thong tin object dang selected
* read-only trong `v0.2`
* xu ly duoc case chua co selection
* khong can edit transform trong `v0.2`

## 6.3 Selection Flow

Selection flow phai:

* click object trong `Outliner` -> object thanh selected
* `Inspector` cap nhat theo selected object
* viewport cap nhat highlight
* user goi `Frame Selected` -> camera focus vao object do

Khong bat buoc `v0.2`:

* pick object bang chuot trong viewport
* selection history
* box select

## 6.4 Import UX

Sau import thanh cong:

* status bar hien thong diep de doc
* thong diep nen gom:
  * file name
  * node count
  * mesh count
  * triangle count
* `Outliner` va `Inspector` dong bo dung

Khi import fail:

* user thay thong diep ngan, ro
* log van giu thong tin chi tiet cho dev
* scene hien tai khong bi clear nham

## 6.5 Frame Selected

Khi object duoc chon:

* camera focus vao `world bounds` cua object neu bounds hop le
* neu object khong co bounds hop le -> fallback ve flow an toan

---

## 7. Technical Scope

## 7.1 Editor Shell Modules

`v0.2` nen co it nhat cac module sau:

* `Application Layer`
* `Editor Shell Layer`
* `Scene Layer`
* `FBX Import Layer`
* `Rendering Layer`
* `Testing Layer`

## 7.2 FBX Import Module Deepening

`FbxImporter` giu interface nho cho caller.

Ben duoi seam nay, implementation nen tach thanh:

* file validation
* `Assimp` parse adapter
* scene conversion
* import report formatting

Muc tieu:

* tang locality cho bug import
* tang leverage cho test
* giu hanh vi beta on dinh

## 7.3 Conservative Data Rule

Trong `v0.2`, mesh import van co the giu behavior hien tai neu behavior do da on.

Khong bat importer doi data model lon chi de "future-proof" qua som neu no mo scope render them.

Rule:

* uu tien refactor an toan
* uu tien interface sau hon implementation
* tranh scope creep sang animation data model day du

---

## 8. Testing Requirements

## 8.1 Unit Tests

Phai bat dau co test cho:

* `EditorCamera`
* `Bounds3D`
* `Scene::appendScene`
* import validation co ban

## 8.2 Integration Tests

Nen co test nhe cho:

* import `box_static.fbx`
* import `hierarchy_multi_mesh.fbx`
* import fail voi file corrupt sample

## 8.3 Manual QA

Manual QA van can giu cho:

* import success flow
* import fail flow
* selection flow
* frame selected flow
* viewport highlight quan sat duoc

---

## 9. UI Layout Direction

`v0.2` UI co the toi thieu nhu sau:

* menu + toolbar hien co
* `Outliner` dock ben trai
* `Inspector` dock ben phai
* `Viewport` o giua
* status bar duoi cung

Khong can:

* dock system phuc tap hon
* workspace layout save/restore
* panel customization

---

## 10. Milestones

## Milestone 1 - Editor Shell

* them `Outliner`
* them `Inspector`
* dong bo scene -> UI panels

## Milestone 2 - Selection

* single object selection
* `Frame Selected`
* selected object feedback trong viewport

## Milestone 3 - Import Hardening

* import summary ro hon
* import error message ro hon
* khong lam hong scene hien tai khi import fail

## Milestone 4 - Test and Refactor

* unit test dau tien
* integration test import nhe
* refactor `FBX import module` theo parse / convert / report

---

## 11. Definition of Done

`v0.2` done khi:

* import `FBX` van on nhu beta
* `Outliner` hien hierarchy scene
* object selection hoat dong
* `Inspector` read-only hoat dong
* `Frame Selected` hoat dong
* viewport co selected feedback ro
* import fail van an toan
* co test tu dong dau tien cho core modules da chon
* tai lieu scope nay van khop implementation thuc te

---

## 12. Recommended Next Step After v0.2

Sau `v0.2`, phase hop ly nhat:

* `Animation Data Readiness`

Muc tieu phase do:

* doc skeleton data
* doc animation clip metadata
* inspect duoc animation-related scene data
* chua vao animation authoring day du
