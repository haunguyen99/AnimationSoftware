# Feature: Rigging Foundation v0.4

## Goal

Mo `Project Phoenix` tu editor co scene + animation data thanh editor co kha nang tao va sua `skeleton hierarchy` hop le.

`v0.4` phai dat duoc:

* tao `joint` / `bone` object trong scene
* parent joint thanh hierarchy
* giu duoc `local` / `world transform` nhat quan
* co `joint orientation` co ban
* luu duoc `bind pose` toi thieu trong scene model

---

## Feature Summary

`Rigging Foundation v0.4` la phase dat nen cho skinning va rig authoring sau nay.

Feature nay khong nham lam full rigging toolset. Feature nay nham dua vao editor:

* `joint` object type
* skeleton hierarchy workflow
* parenting / reparenting rule an toan
* orientation rule cho joint
* bind pose representation trong data model

Sau phase nay, user co the tao 1 skeleton toi thieu, sua hierarchy, va luu scene ma khong mat thong tin rig co ban.

---

## Scope

In scope:

* them `joint` / `bone` object type vao `Scene`
* hien `joint` trong `Outliner`
* tao joint moi tu UI va command
* parent / unparent joint va object co quy tac ro rang
* cap nhat `world transform` dung sau khi doi hierarchy
* `joint orientation` basics:
  * luu orientation rieng voi animated rotation
  * cho phep set / reset orientation theo rule don gian
* `bind pose` representation toi thieu trong scene model
* save/load scene co giu duoc joint hierarchy, orientation, bind pose
* viewport visualization co ban cho joint hierarchy

---

## Out-of-scope

* IK / FK
* constraints
* controllers / custom shapes
* skin bind
* weight painting
* mirror skeleton tools
* autorig
* retargeting
* advanced orientation presets giong Maya day du
* animation layer / rig evaluation graph

---

## Use Cases

1. User tao joint root trong scene rong -> thay joint trong `Outliner` va `Viewport`.

2. User tao joint con duoi joint dang selected -> hierarchy duoc noi dung va world transform hop le.

3. User reparent 1 joint sang parent moi -> hierarchy cap nhat ma khong pha scene state.

4. User chinh orientation co ban cua joint -> viewport va inspector cap nhat nhat quan.

5. User save scene -> mo lai -> skeleton hierarchy, orientation va bind pose van con.

6. User dung command de tao joint hoac parent joint -> UI sync lai dung.

---

## Constraints

* bam `ADR-003`: scene model van uu tien toi gian
* bam `ADR-004`: internal `Scene` la canonical, file format chi la external representation
* khong lam vo scene workflow va animation foundation da co
* khong dua `v0.5 Skinning` vao qua som
* object transform va joint orientation phai tach nghia ro rang de tranh debt cho playback sau nay
* implementation phai de test doc lap, uu tien core model truoc UI polish

---

## UML

```text
Scene
  |- SceneObject
       |- MeshObject
       |- JointObject

MainWindow
  |- Outliner
  |- Channel Box
  |- ViewportWidget
  |- Rig Commands

ViewportWidget
  |- JointRenderer
```

Quan he:

```text
MainWindow -> Scene
MainWindow -> ScriptCommandSystem
SceneObject(joint) -> local transform
SceneObject(joint) -> joint orientation
SceneObject(joint) -> bind pose data
ViewportWidget -> read Scene joint hierarchy -> draw joints/bones
```

---

## Data Flow

### Create Joint

```text
User click "Create Joint"
  -> MainWindow / command dispatch xac dinh parent target
  -> Scene tao SceneObject type = joint
  -> Scene set local transform + orientation mac dinh
  -> neu co parent thi add vao hierarchy
  -> MainWindow refresh Outliner / Channel Box / Viewport
```

### Reparent Joint

```text
User parent joint A duoi joint B
  -> validate khong tao cycle
  -> Scene tinh local transform moi neu can giu world transform
  -> Scene update parentId / childIds
  -> Scene rebuild world data
  -> UI refresh hierarchy + viewport
```

### Edit Orientation

```text
User doi joint orientation
  -> Channel Box / Inspector dua orientation moi vao Scene
  -> Scene cap nhat orientation field cua joint
  -> Scene rebuild world matrices
  -> Viewport redraw skeleton
```

### Save / Load

```text
Save Scene
  -> PhoenixSceneDocument serialize object type + hierarchy + orientation + bind pose

Open Scene
  -> PhoenixSceneDocument restore joint objects
  -> Scene rebuild hierarchy va world data
  -> UI refresh skeleton state
```

---

## Design Pattern

Pattern chinh:

* `Scene` = source of truth cho joint hierarchy
* `SceneObject` = host cho common transform + type-specific rig data nho
* `MainWindow` = orchestration cho rigging actions cap editor
* `ViewportWidget` = read-only visualization cho joint hierarchy
* `ScriptCommandSystem` = adapter command -> scene mutation

Rule:

* joint parenting phai di qua `Scene`, khong cho UI sua truc tiep child list
* orientation va animated rotation khong duoc tron vao mot field duy nhat
* bind pose la scene data, khong la UI cache
* viewport chi doc skeleton state, khong tu giu hierarchy rieng

---

## Module Boundaries

`Scene`

* dinh nghia object type `joint`
* validate hierarchy
* xu ly parent / unparent
* luu local transform, joint orientation, bind pose
* rebuild world data

`SceneObject`

* giu type cua object
* giu joint-specific data neu object la joint

`PhoenixSceneDocument`

* serialize / deserialize object type
* serialize / deserialize joint orientation
* serialize / deserialize bind pose

`MainWindow`

* expose action `Create Joint`, `Parent`, `Unparent`
* sync selection va UI widgets
* route action vao scene / command system

`ViewportWidget` / renderer

* draw joints va bone line don gian
* selected joint feedback

`ScriptCommandSystem`

* command support cho `joint`, `parent`, `unparent`, orientation action co ban

---

## Data Model Direction

Khuyen nghi toi thieu cho `SceneObject`:

* `ObjectKind kind`
* `Transform localTransform`
* `QQuaternion jointOrientation`
* `Transform bindPoseLocalTransform`
* `bool hasBindPose`

Ly do:

* `localTransform.rotation` nen tiep tuc la rotation runtime / animation
* `jointOrientation` la rotation co cau cua khop
* `bindPoseLocalTransform` la moc can cho `v0.5 Skinning`

Cho `v0.4`, co the de `jointOrientation` va `bindPoseLocalTransform` chi co y nghia khi `kind == joint`.

---

## Parenting Rules

Rule toi thieu:

* cam parent object vao chinh no
* cam parent tao cycle
* cho phep parent joint vao joint
* cho phep parent object vao joint neu team muon ho tro socket / prop sau nay
* reparent mac dinh nen giu `world transform`
* `Outliner` phai phan anh hierarchy moi ngay sau action

Khuyen nghi rollout:

1. joint -> joint
2. object -> joint
3. joint -> non-joint chi neu co ly do ro rang

---

## Orientation Rules

`v0.4` chi can basics:

* joint co orientation rieng
* user co the reset orientation ve identity
* user co the align orientation theo huong toi child neu joint co child
* neu joint khong co child -> orientation fallback identity hoac giu nguyen

Khong can trong `v0.4`:

* full orientation presets `xyz/xzy/...`
* secondary axis world up policy phuc tap
* mirror orientation tools

---

## Viewport Visualization Strategy

Phase 1:

* joint = diem / shape nho de thay duoc
* bone = line tu parent toi child
* mau selected ro rang
* khong can mesh skin preview

Uu diem:

* de verify hierarchy nhanh
* coupling thap hon so voi bone mesh render

---

## Risks

1. Tron `joint orientation` voi `animated rotation`
Neu khong tach data ngay tu dau, `v0.5` va `v0.6` se rat kho mo rong.

2. Hierarchy mutation de lam hong world transform
Parent / unparent rat de gay bug neu quy tac giu world transform khong ro.

3. `SceneObject` co the phinh neu nhoi qua nhieu field type-specific
Can giu data rig o muc toi thieu trong `v0.4`.

4. Viewport visualization co the keo theo renderer complexity
Can uu tien line + point renderer don gian truoc.

5. Save/load versioning
Them object kind va bind pose vao file format can giu backward compatibility voi scene cu.

---

## Open Questions

1. `joint` hien duoc giu la `SceneObject` voi `kind = joint` de bam `ADR-003` va giu beta model gon.

2. `v0.4 phase 1` uu tien skeleton workflow `joint -> joint`; object -> joint co the mo o phase sau neu can socket / prop attachment.

3. `bind pose` duoc support bang action capture rieng trong `Channel Box` va script command `bindPose -capture`.

4. Orientation UI dat trong `Channel Box` cho `v0.4` de tan dung editor shell hien co; chua mo panel rig rieng.

5. Da co `ADR-009` de khoa quyet dinh tach `jointOrientation` khoi animated rotation.
