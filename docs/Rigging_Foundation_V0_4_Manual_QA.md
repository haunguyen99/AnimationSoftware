# Rigging Foundation v0.4 Manual QA

## Goal

Xac nhan `v0.4 phase 1` on trong app truoc khi khoa feature va chuyen trong tam sang `v0.5`.

## Preconditions

* app mo tu `build/ninja-msvc-debug`
* scene rong
* khong co file scene cu dang giu lock

## QA Pass

### 1. Create Joint Root

1. vao menu `Create -> Joint`
2. xac nhan co `joint` moi trong `Outliner`
3. xac nhan viewport hien joint shape

Expected:

* `joint` duoc select ngay sau khi tao
* `Channel Box` hien `Joint Tools`
* `Bind pose` status = `not captured`

### 2. Create Child Joint

1. giu root joint dang select
2. vao menu `Create -> Joint`
3. xac nhan joint moi thanh child cua root trong `Outliner`
4. xac nhan viewport hien bone line parent-child

Expected:

* hierarchy dung
* child joint duoc select
* viewport thay structure skeleton

### 3. Unparent / Reparent

1. select child joint
2. dung `Rig -> Unparent Selected`
3. xac nhan child len root level
4. select root joint
5. dung `Rig -> Mark Selected As Parent`
6. select child joint
7. dung `Rig -> Parent Selected To Marked Parent`

Expected:

* unparent thanh cong
* reparent thanh cong
* khong crash
* hierarchy refresh dung ngay

### 4. Drag Hierarchy

1. left-drag child joint ra vung rong trong `Outliner`
2. xac nhan child thanh root
3. left-drag child joint vao root joint
4. xac nhan child quay lai duoi root

Expected:

* drag root area = unparent
* drag vao item = parent vao item

### 5. Joint Orientation

1. select root joint
2. doi `Joint Orient X/Y/Z` trong `Channel Box`
3. xac nhan gia tri giu lai sau khi edit
4. dung `Rig -> Align Joint Orientation To Child`
5. dung `Rig -> Reset Joint Orientation`

Expected:

* edit orientation khong bi reset ve `0` bat ngo
* `Align` chi hoat dong khi joint co child
* `Reset` dua orientation ve identity

### 6. Bind Pose

1. select root joint
2. dung `Rig -> Capture Bind Pose`
3. xac nhan `Bind pose` status doi thanh `captured`
4. dung `Rig -> Capture Bind Pose Recursive`
5. select child joint
6. xac nhan child cung co `Bind pose` status = `captured`

Expected:

* bind pose status update dung
* recursive capture xuong child joint

### 7. Save / Reload

1. save scene thanh file `.phoenixscene`
2. dong scene, mo lai file vua save
3. xac nhan hierarchy van dung
4. xac nhan joint orientation van con
5. xac nhan bind pose status van con

Expected:

* save/load khong mat `joint`, `jointOrientation`, `bindPose`

## Exit Rule

`v0.4 phase 1` co the khoa neu:

* tat ca step tren pass
* khong co bug nghiem trong UX hoac data loss
* khong loi save/load skeleton scene

## Result

Status:

* Passed

Notes:

* user xac nhan pass het
* co the khoa `v0.4`
* next milestone = `v0.5 Skinning`
