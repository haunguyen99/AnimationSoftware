# Feature: Rigging Foundation v0.4 Implementation Plan

## Status

`Step 3 - Implementation In Progress`

Design source:

* [Rigging_Foundation_V0_4_Design.md](</E:/Animation Software/docs/Rigging_Foundation_V0_4_Design.md>)

Gate status:

* core implementation da vao code
* `phase 1` da build + `ctest` pass
* con manual QA va quyet dinh co can `phase 2` polish hay khong

Progress snapshot:

* done: `Task 1` -> `Task 9` o muc `phase 1`
* con mo: polish orientation rules nang cao va manual QA trong app

---

## Task Breakdown

### Task 1 - Scene Joint Data Model

Muc tieu:

* them `ObjectKind` hoac equivalent cho `joint`
* them `jointOrientation`
* them `bindPose` representation toi thieu
* giu backward compatibility cho object thuong

Output:

* `SceneObject` biet object nao la joint
* data model du cho hierarchy va skinning sau nay

---

### Task 2 - Hierarchy Mutation Rules

Muc tieu:

* them API `parent`, `unparent`, validate cycle
* chot rule `keep world transform on reparent`
* rebuild world data an toan sau hierarchy doi

Output:

* `Scene` co seam ro cho rig hierarchy edits

---

### Task 3 - Scene Serialization Upgrade

Muc tieu:

* save/load `kind`, `jointOrientation`, `bindPose`
* giu scene cu mo duoc
* test round-trip cho skeleton scene

Output:

* scene file giu duoc rig foundation data

---

### Task 4 - Joint Creation Commands

Muc tieu:

* them action UI tao joint
* them command `joint`
* them command `parent` / `unparent`

Output:

* user co duong tao va sua skeleton tu UI va script

---

### Task 5 - Outliner And Selection Sync

Muc tieu:

* hien joint trong `Outliner`
* parent/unparent refresh dung
* selection flow joint <-> viewport <-> channel box van on

Output:

* joint hierarchy hien dung trong editor shell hien co

---

### Task 6 - Viewport Skeleton Visualization

Muc tieu:

* draw point/shape cho joint
* draw line parent-child
* selected feedback ro rang

Output:

* user nhin duoc skeleton structure trong viewport

---

### Task 7 - Orientation Basics

Muc tieu:

* expose `jointOrientation`
* them reset orientation
* them align-to-child orientation rule don gian

Output:

* orientation workflow toi thieu de tao skeleton hop le

---

### Task 8 - Bind Pose Workflow

Muc tieu:

* seed `bind pose` cho joint
* cho phep cap nhat / luu bind pose theo rule don gian
* khong lam sang skinning

Output:

* scene model san sang cho `v0.5`

---

### Task 9 - Tests And QA

Muc tieu:

* unit test cho hierarchy mutation
* unit test cho cycle rejection
* unit test cho orientation / bind pose serialization
* UI/runtime test cho create joint, parent, save/load skeleton scene

Output:

* `v0.4` co xac nhan build + test + manual QA

---

## Implementation Order

1. `Scene Joint Data Model`
2. `Hierarchy Mutation Rules`
3. `Scene Serialization Upgrade`
4. `Joint Creation Commands`
5. `Outliner And Selection Sync`
6. `Viewport Skeleton Visualization`
7. `Orientation Basics`
8. `Bind Pose Workflow`
9. `Tests And QA`

Reason:

* data model va hierarchy seam phai on truoc UI
* serialization can chot som de tranh doi format muon
* viewport va orientation nen dung tren scene model da on
* test can chot cuoi nhung nen viet song song cho core path

---

## Dependencies

Phu thuoc san co:

* `Scene`
* `SceneObject`
* `PhoenixSceneDocument`
* `MainWindow`
* `ViewportWidget`
* `ScriptCommandSystem`
* [FEATURE_WORKFLOW.md](</E:/Animation Software/FEATURE_WORKFLOW.md>)
* [Rigging_Foundation_V0_4_Design.md](</E:/Animation Software/docs/Rigging_Foundation_V0_4_Design.md>)

Phu thuoc moi:

* khong can dependency third-party moi cho `v0.4`

---

## Validation Checklist

* tao duoc `joint root`
* tao duoc `joint child`
* `Outliner` hien hierarchy joint dung
* reparent khong tao cycle
* reparent giu `world transform` theo rule da chot
* viewport hien duoc point/line skeleton
* selected joint feedback ro
* `jointOrientation` edit duoc va save/load duoc
* `bind pose` representation co ton tai va save/load duoc
* scene cu khong co joint van mo duoc
* app build duoc
* `ctest` pass

---

## Test Plan

### Build

* build bang flow hien co cua project

### Automated Tests

1. `Scene` test:
   * create joint object
   * parent / unparent
   * cycle reject
   * keep world transform khi reparent
   * orientation data khong mat sau mutation

2. serialization test:
   * save/load scene co joint hierarchy
   * save/load `jointOrientation`
   * save/load `bindPose`
   * backward compatibility voi scene khong co field moi

3. UI / command test:
   * create joint bang command
   * parent / unparent bang command
   * `Outliner` refresh sau hierarchy edit
   * viewport selected joint feedback

### Manual QA

1. mo app scene rong
2. tao `joint root`
3. tao tiep `joint child`
4. xac nhan hierarchy dung trong `Outliner`
5. reparent joint
6. xac nhan skeleton van dung trong viewport
7. doi orientation co ban
8. save scene
9. mo lai scene
10. xac nhan hierarchy + orientation + bind pose van con

---

## Risks

* type-specific data trong `SceneObject` co the phinh nhanh
* reparent logic de sinh regression cho scene graph hien tai
* viewport skeleton render co the mo duong cho renderer debt neu lam qua dep qua som
* backward compatibility cua `.phoenixscene` can test ky

Risk control:

* uu tien `joint` renderer don gian
* khoa `v0.4` o skeleton authoring, khong lam skinning
* viet test core scene graph truoc khi polish UI
* neu data model doi lon, mo ADR rieng

---

## Definition of Done

`v0.4` duoc xem la done khi:

* co `joint` object type trong scene
* user tao va parent duoc skeleton hierarchy hop le
* hierarchy mutation khong tao cycle va giu state an toan
* viewport hien skeleton du de lam viec
* `jointOrientation` basics co mat
* `bindPose` representation co mat
* save/load scene giu duoc rig foundation data
* command path co cho `joint` / `parent` / `unparent`
* test va QA pass

---

## Next Step After This Plan

Neu plan nay duoc duyet:

* vao `Step 3 - Implementation` cho `Rigging Foundation v0.4`

Feature tiep theo sau khi xong item nay:

* `v0.5 Skinning`
