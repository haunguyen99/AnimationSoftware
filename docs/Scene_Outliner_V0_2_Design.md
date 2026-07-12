# Feature: Scene Outliner v0.2

## Goal

Them `Scene Outliner` de user thay duoc hierarchy scene sau import.

`Outliner` trong `v0.2` phai:

* hien object tree de doc
* support single selection
* dong bo voi scene sau import
* an bot wrapper node gay nhieu cho user

---

## Feature Summary

`Scene Outliner v0.2` = read-only tree view cua `SceneObject` hierarchy, phuc vu inspect scene va lam nen cho `Selection Flow`.

Feature nay khong doi source of truth cua `Scene`.
Feature nay chi them presentation seam cho hierarchy.

---

## Scope

In scope:

* hien tree tu `Scene`
* root object listing
* child hierarchy listing
* single selection trong tree
* refresh sau import success
* empty state khi scene rong
* presentation policy an wrapper `RootNode`

---

## Out-of-scope

* drag-drop reparent
* rename object
* multi-select
* context menu
* visibility toggle
* lock/freeze
* viewport picking
* doi `Scene` data model de phuc vu outliner

---

## Use Cases

1. User import `FBX` -> `Outliner` hien object tree de user biet scene co gi.

2. User import nhieu file -> `Outliner` van doc duoc tung nhanh scene append.

3. User gap file co wrapper `RootNode` -> `Outliner` hien node con meaningful thay vi lap lai `RootNode` gay roi.

4. Feature `Selection Flow` ve sau noi vao `Outliner` de chon object.

---

## Constraints

* khong doi importer chi de dep outliner
* khong sua `SceneObject` hierarchy goc
* policy `hide wrapper RootNode` chi song o presentation layer
* bam `ADR-003`: scene model beta van la tree-based
* bam `ADR-004`: `FBX` la external format, internal `Scene` la canonical

---

## UML

```text
MainWindow
  |- OutlinerPanel
       |- OutlinerTreeBuilder
            -> Scene
            -> SceneObject
```

Quan he:

```text
OutlinerPanel --> OutlinerTreeBuilder
OutlinerTreeBuilder --> Scene (read-only)
OutlinerTreeBuilder --> SceneObject (read-only)
```

---

## Data Flow

```text
Import FBX success
  -> ViewportWidget cap nhat Scene
  -> MainWindow goi refresh outliner
  -> OutlinerTreeBuilder doc Scene roots
  -> apply presentation policy
  -> tao tree items
  -> render tree
```

```text
Wrapper RootNode case
  -> Scene van giu RootNode
  -> OutlinerTreeBuilder kiem tra node
  -> neu node la wrapper-only
       -> bo qua node presentation
       -> day con cua no len tree level hien thi
```

```text
Scene rong
  -> tree hien empty state item
```

---

## Design Pattern

Pattern chinh:

* `OutlinerPanel` = presentation module
* `OutlinerTreeBuilder` = adapter tu `Scene` -> tree items
* `Scene` = source of truth

Rule:

* outliner khong nam business logic import
* outliner khong chinh scene
* wrapper-node filtering la presentation policy, khong phai domain rule

---

## Module Boundaries

`MainWindow`

* host panel
* trigger refresh

`OutlinerPanel`

* render tree
* phat signal selection sau nay

`OutlinerTreeBuilder`

* doc `Scene`
* xac dinh node nao hien
* xay tree item data

`Scene`

* van giu hierarchy canonical

---

## Presentation Policy: Hide Wrapper RootNode

Node duoc xem la wrapper-only khi:

* ten = `RootNode`
* khong co mesh meaningful
* role chinh la parent container

Xu ly:

* khong hien node `RootNode` do trong outliner
* hien truc tiep children cua no

Khong lam:

* khong xoa node khoi `Scene`
* khong doi parent/child canonical data
* khong ap policy nay ra ngoai outliner

Reason:

* giu locality cho UX cleanup
* importer van trung thuc voi file nguon
* risk thap hon sua import/domain data

---

## Risks

* heuristic `RootNode` co the an nham node user thuc su muon thay
* policy presentation neu viet vo `MainWindow` se thanh shallow utility kho mo rong
* append nhieu file co the lam tree crowded neu khong co rule hien thi nhat quan

Risk control:

* gioi han heuristic ban dau rat hep
* chi an `RootNode` wrapper-only
* neu nghi ngo -> uu tien hien node thay vi an

---

## Open Questions

1. `RootNode` heuristic co nen dua them dieu kien `chi co 1 child` khong?

2. Khi append nhieu file, co can top-level group theo lan import khong, hay cu de flat roots?

3. Co nen tach `OutlinerTreeBuilder` thanh helper rieng ngay trong vong dau, hay de logic nam tam trong panel roi tach sau?
