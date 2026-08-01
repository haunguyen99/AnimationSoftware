# Feature: Application Shell v0.2

## Goal

Giu `app shell` on dinh khi mo rong `v0.2`.

`Application Shell` trong phase nay phai:

* tiep tuc launch on
* giu `Viewport` lam trung tam
* cho phep them `Outliner`, `Inspector`, action moi
* khong lam vo import flow beta

---

## Feature Summary

`Application Shell v0.2` = mo rong editor shell hien tai tu layout beta toi thieu thanh layout co kha nang chua panel va action cho phase inspect scene.

Feature nay khong nham them nang luc domain moi. Feature nay nham tao cho UI shell on dinh de cac feature sau gan vao.

---

## Scope

In scope:

* xac dinh layout shell cho `v0.2`
* vi tri `Viewport`, `Outliner`, `Inspector`, `Status Bar`, `Toolbar`
* rule cho action `Import FBX`, `Reset Camera`, `Frame Scene`, `Frame Selected`
* rule cap nhat shell sau import
* rule shell khi scene rong
* rule shell khi chua co selection

---

## Out-of-scope

* viewport object picking
* selection highlight rendering
* editable inspector
* save/restore workspace layout
* multi-window
* docking customization sau
* animation UI
* rigging UI

---

## Use Cases

1. User mo app -> thay `Viewport` o giua, panel o hai ben, status bar o duoi.

2. User import `FBX` -> shell van on, panel duoc cap nhat theo scene moi.

3. User chua import gi -> `Outliner` va `Inspector` hien trang thai rong an toan.

4. Feature sau nhu `Selection`, `Inspector`, `Import Hardening` can noi vao shell -> khong can doi layout lon nua.

---

## Constraints

* bam `ADR-002`: `Qt Widgets + QOpenGLWidget`
* khong doi core app startup flow dang on
* khong dua them dependency moi
* khong lam `ViewportWidget` ganh them qua nhieu UI policy
* panel moi phai la `Qt dock/widget` don gian, de debug

---

## UML

```text
EditorShell
  |- MenuBar
  |- ToolBar
  |- StatusBar
  |- OutlinerPanel
  |- InspectorPanel
  |- ViewportWidget
```

Quan he:

```text
EditorShell --> ViewportWidget
EditorShell --> OutlinerPanel
EditorShell --> InspectorPanel
OutlinerPanel --> Scene read model
InspectorPanel --> selected object read model
```

---

## Data Flow

```text
App start
  -> EditorShell tao shell layout
  -> ViewportWidget khoi tao viewport
  -> panels vao trang thai empty

Import FBX success
  -> ViewportWidget cap nhat Scene
  -> EditorShell nhan import success
  -> EditorShell refresh shell panels
  -> Outliner doc Scene de hien tree
  -> Inspector ve "no selection" hoac state mac dinh
```

```text
Khong co scene
  -> Outliner hien empty state
  -> Inspector hien no selection
  -> action phu thuoc selection bi disable
```

---

## Design Pattern

Pattern chinh:

* `EditorShell` = `module` shell cap `Core/Application`
* `EditorShell` = adapter `Qt Widgets` hien tai dang implement `EditorShell`
* `ViewportWidget` = viewport adapter
* `Outliner` / `Inspector` = read-only presentation modules trong `v0.2`

Rule:

* shell state orchestration nam o `EditorShell`, hien duoc host boi `EditorShell`
* scene source of truth van nam o `ViewportWidget` / `Scene`
* panel khong tu sua domain data trong phase nay

---

## Module Boundaries

`EditorShell` / `EditorShell`

* so huu layout shell
* so huu action UI muc app-shell
* refresh panel state
* enable/disable action theo shell state

Current ownership note:

* ve `module`, day la phan cua `Core/Application`
* ve implementation, `EditorShell` la concrete adapter hien tai
* khong nen doc no nhu feature module host cho `Scene`, `Animation`, `Rigging`, hay `Rendering`

`ViewportWidget`

* render viewport
* giu scene hien tai
* expose read-only seam du cho shell can doc scene

`OutlinerPanel`

* chi hien tree tu scene
* khong doi scene

`InspectorPanel`

* chi hien thong tin object / shell state
* khong edit data trong `v0.2`

---

## Risks

* `EditorShell` thanh module qua shallow neu om ca shell policy + selection policy + import policy
* shell state co the bi sync sai sau import fail
* panel update truc tiep tu `ViewportWidget` co the dan toi coupling tang dan
* neu them panel nhanh ma khong khoa boundary, feature sau se tiep tuc don logic vao `EditorShell`

---

## Open Questions

1. Trong `v0.2`, shell co nen co `Frame Selected` action ngay tu `Application Shell`, hay de sang feature `Selection`?

2. Empty state cua `Outliner` / `Inspector` co can text huong dan nhe cho user, hay chi can de rong?

3. Co can tach `OutlinerPanel` va `InspectorPanel` thanh class rieng ngay tu dau, hay de `EditorShell` host truc tiep trong vong dau?
