# Feature: Viewport Object Manipulation

## Goal

Cho user chon object truc tiep trong `Viewport`, thay `transform gizmo`, va thao tac `translate / rotate / scale` giong workflow quen thuoc cua `Maya`.

Feature nay nham bien `Project Phoenix` tu editor shell inspect scene thanh editor shell co kha nang thao tac object trong khong gian 3D.

---

## Feature Summary

`Viewport Object Manipulation` gom 3 nang luc lien ket:

* `Viewport Picking`
* `Viewport Selection Sync`
* `Transform Gizmo Interaction`

User click vao object trong `Viewport` -> object duoc selected -> `Outliner`, `Inspector`, `Viewport` dong bo -> gizmo xuat hien tai object -> user drag gizmo de doi transform.

---

## Scope

In scope:

* click object trong `Viewport` de select
* dong bo selection giua `Viewport`, `Outliner`, `Inspector`
* hien `transform gizmo`
* support 3 mode:
  * `Translate`
  * `Rotate`
  * `Scale`
* thao tac gizmo de cap nhat `SceneObject.localTransform`
* cap nhat viewport render sau transform
* cap nhat `Inspector` sau transform

---

## Out-of-scope

* undo / redo
* snapping
* local/world mode switch day du
* pivot editing
* multi-selection
* marquee / lasso selection
* soft selection
* hierarchy transform propagation phuc tap hon beta scene model hien tai
* animation keyframe authoring khi doi transform
* constraint / IK / FK

---

## Use Cases

1. User import `FBX` -> click vao mesh trong `Viewport` -> object duoc select ma khong can click `Outliner`.

2. User thay gizmo `Translate` -> keo truc `X/Y/Z` de doi vi tri object.

3. User doi sang `Rotate` -> xoay object quanh truc.

4. User doi sang `Scale` -> scale object theo truc hoac uniform rule don gian.

5. User click object trong `Outliner` -> gizmo trong `Viewport` cap nhat theo object do.

---

## Constraints

* bam `ADR-002`: `Qt Widgets + QOpenGLWidget`
* bam `ADR-003`: `Scene` hien tai la tree-based, toi gian
* bam `ADR-004`: internal `Scene` la canonical
* khong duoc lam vo selection flow da co
* khong duoc lam vo import / inspect / frame selected flow da co
* uu tien implementation nho, de verify, khong nhay thang toi full Maya parity

---

## UML

```text
MainWindow
  |- ViewportWidget
  |- Outliner
  |- Inspector
  |- SelectionState

ViewportWidget
  |- PickingAdapter
  |- GizmoController
  |- ViewportRenderer

ViewportRenderer
  |- Scene mesh pass
  |- Selection overlay pass
  |- Gizmo pass
```

Quan he:

```text
Viewport click -> PickingAdapter -> SelectionState
SelectionState -> MainWindow / Outliner / Inspector / ViewportWidget
Viewport drag gizmo -> GizmoController -> SceneObject.localTransform
Scene change -> ViewportRenderer + Inspector refresh
```

---

## Data Flow

### Selection From Viewport

```text
Mouse click in Viewport
  -> cast picking ray
  -> test object hit
  -> choose nearest hit object
  -> update SelectionState
  -> sync Outliner current item
  -> sync Inspector fields
  -> show gizmo on selected object
```

### Manipulation

```text
Mouse press on gizmo handle
  -> GizmoController enters drag state
  -> map mouse movement to transform delta
  -> update SceneObject.localTransform
  -> rebuild affected bounds if needed
  -> refresh viewport
  -> refresh Inspector
```

### Selection From Outliner

```text
Outliner selection change
  -> update SelectionState
  -> ViewportWidget receives selected object
  -> selection overlay + gizmo move to object
```

---

## Design Pattern

Pattern chinh:

* `SelectionState` = source of truth cho selected object
* `PickingAdapter` = adapter tu mouse click -> selected object id
* `GizmoController` = adapter tu mouse drag -> transform delta
* `ViewportRenderer` = render gizmo va overlay

Rule:

* `MainWindow` khong tu tinh picking math
* `ViewportRenderer` khong tu thay doi scene data
* `GizmoController` khong giu source of truth selection
* transform update phai di qua scene object seam ro rang

---

## Module Boundaries

`MainWindow`

* so huu selection sync giua panels
* so huu current transform mode UI

`ViewportWidget`

* nhan input chuot
* phan biet camera navigation vs picking vs gizmo drag
* goi `PickingAdapter` / `GizmoController`

`PickingAdapter`

* nhan ray
* tra selected `SceneObject::Id`

`GizmoController`

* biet mode `Translate / Rotate / Scale`
* biet dang drag handle nao
* tinh transform delta

`ViewportRenderer`

* ve gizmo
* ve selected overlay
* khong doi `Scene`

`Scene`

* giu `SceneObject.localTransform`
* la source of truth cho transform

---

## Picking Strategy

Phase 1 strategy:

* ray cast tu screen point vao world
* hit test voi `world bounds` truoc
* chon object co hit gan camera nhat

Uu diem:

* de lam
* it risk
* hop scene beta hien tai

Han che:

* bounds picking khong chinh xac bang triangle picking
* object overlap co the chon chua dep

Moc sau neu can:

* mesh-level ray intersection

---

## Gizmo Strategy

Phase 1:

* 1 gizmo co 3 mode
* `Translate` uu tien truoc
* `Rotate` va `Scale` theo sau, co the lam gian luoc

Visual:

* 3 truc `X/Y/Z`
* mau quen thuoc:
  * `X` = red
  * `Y` = green
  * `Z` = blue

Interaction:

* click handle -> active axis
* drag chuot -> delta theo axis

Khuyen nghi rollout:

1. picking object
2. gizmo render only
3. translate drag
4. rotate drag
5. scale drag

---

## Transform Rules

Phase 1 rule:

* user thao tac tren `localTransform`
* update `translation / rotation / scale` cua selected object
* bounds world can duoc rebuild sau transform

Can chot som:

* beta scene hien tai da bake mesh positions theo world luc import
* neu van giu rule nay, transform runtime se khong "that" theo kien truc lau dai

=> Feature nay co kha nang bat buoc mo ADR / refactor scene-representation truoc khi lam dung.

Day la risk lon nhat.

---

## Risks

1. `Scene` hien tai khong that su san sang cho transform editing
Mesh imported dang duoc dua ve `world position` som trong importer. Dieu nay xung dot voi nhu cau move/rotate/scale object sach se sau import.

2. camera navigation xung dot voi gizmo interaction
Can tach ro Alt-navigation vs click-select vs drag-gizmo.

3. picking bang bounds co the cho UX chua dep
Nhung van hop ly cho phase 1.

4. renderer va scene sync co the tang coupling
Neu update transform khong co seam ro, code se roi vao `ViewportWidget`.

---

## Open Questions

1. Co nen mo feature nay ngay bay gio, hay truoc do phai refactor scene/import de mesh khong bi bake world som?

2. Co nen rollout theo 2 phase:
   * phase A: viewport picking + selection sync
   * phase B: gizmo manipulation

3. Mode dau tien nen chi lam `Translate` truoc de giam risk, hay buoc dau da phai co du ca `Translate / Rotate / Scale`?

4. Selection source of truth nen dat o `MainWindow`, hay nen tao module rieng `SelectionState`?
