# Feature: Application Shell v0.2 Implementation Plan

## Status

`Step 2 - Implementation Plan`

Design source:

* [Application_Shell_V0_2_Design.md](</E:/Animation Software/docs/Application_Shell_V0_2_Design.md>)

Gate status:

* design reviewed
* sang implementation plan
* chua vao code trong tai lieu nay

---

## Task Breakdown

### Task 1 - Shell Layout Hardening

Muc tieu:

* khoa layout `EditorShell`
* giu `Viewport` o giua
* dat cho `Outliner` ben trai
* dat cho `Inspector` ben phai
* giu `Toolbar` va `Status Bar` on

Output:

* shell layout ro rang
* empty layout van dung khi chua co scene

---

### Task 2 - Outliner Panel Host

Muc tieu:

* tao host cho `Outliner`
* dinh nghia empty state
* chuan bi seam de scene sau nay do vao

Output:

* panel xuat hien on
* chua can full selection logic

---

### Task 3 - Inspector Panel Host

Muc tieu:

* tao host cho `Inspector`
* dinh nghia empty state text ngan
* read-only

Output:

* panel xuat hien on
* khong sua domain data

---

### Task 4 - App-shell Action Policy

Muc tieu:

* giu `Import FBX`
* giu `Reset Camera`
* giu `Frame Scene`
* chua dua `Frame Selected` vao feature nay

Output:

* action map ro
* shell khong om selection policy qua som

---

### Task 5 - Panel Refresh Contract

Muc tieu:

* sau import success -> shell refresh duoc
* khi scene rong -> panel hien empty state
* import fail -> shell state an toan

Output:

* contract ro giua `EditorShell` va panel state

---

## Implementation Order

1. `Shell Layout Hardening`
2. `Outliner Panel Host`
3. `Inspector Panel Host`
4. `App-shell Action Policy`
5. `Panel Refresh Contract`

Reason:

* layout truoc
* panel host sau
* policy va refresh contract cuoi

---

## Dependencies

Phu thuoc san co:

* `EditorShell`
* `ViewportWidget`
* `Scene`
* `FEATURE_WORKFLOW.md`
* `Application_Shell_V0_2_Design.md`

Khong them dependency moi.

---

## Validation Checklist

* app build duoc
* app launch duoc
* `Viewport` van o giua
* `Toolbar` van hoat dong
* `Status Bar` van hoat dong
* `Outliner` panel hien duoc
* `Inspector` panel hien duoc
* scene rong -> panel khong vo
* import fail -> shell khong vo

---

## Test Plan

### Build

* build bang flow hien co cua project

### Manual QA

1. mo app khi scene rong
2. xac nhan `Viewport` o giua
3. xac nhan `Outliner` ben trai
4. xac nhan `Inspector` ben phai
5. xac nhan toolbar action cu van bam duoc
6. import file hop le -> shell van on
7. import file loi -> shell van on

### Automated Tests

Khong uu tien test tu dong rieng cho feature nay o vong dau.

Reason:

* feature nay chu yeu la shell composition bang Qt Widgets
* ROI test tu dong thap hon so voi `camera`, `bounds`, `scene append`, `import`

---

## Risks

* `EditorShell` tiep tuc phinh
* panel host trong `EditorShell` co the thanh shallow module
* shell refresh contract mo ho -> ve sau de leak policy vao nhieu noi

Risk control:

* gioi han feature nay o shell host
* khong dua selection policy vao day
* khong dua viewport highlight vao day

---

## Definition of Done

Feature nay done khi:

* shell layout `v0.2` da khoa
* `Outliner` host co mat
* `Inspector` host co mat
* empty state ro rang
* action cu cua beta van on
* import flow beta khong bi vo
* khong mo rong sang selection logic

---

## Next Step After This Plan

Neu plan nay duoc duyet:

* vao `Step 3 - Implementation` cho `Application Shell`

Feature tiep theo sau khi xong item nay:

* `Selection Flow`
