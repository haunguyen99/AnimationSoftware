# Product Architecture Blueprint

## Purpose

Tai lieu nay chot kien truc san pham muc cao cho `Project Phoenix` de team co chung mot khung khi quyet dinh:

* roadmap san pham
* boundaries giua module
* uu tien beta va future platform
* cach mo rong sang `plugin`, `Python`, `AI`

Tai lieu nay khong thay the ADR ky thuat chi tiet.
No dong vai tro `product architecture map` noi giua vision, milestone, va implementation.

---

## Product Thesis

`Project Phoenix` la editor `rigging + animation` cho `game animator`, uu tien:

* `animation-first`
* `workflow-first`
* `plugin-ready`
* `AI-ready`
* `khong om full DCC pipeline`

Gia tri cot loi:

* thao tac key, pose, timing nhanh hon
* skeleton va skinning du de hoan thanh animation workflow co ban
* scene data on dinh, de save/load/import/export
* kien truc du de studio them cong cu rieng sau nay

Khong nam trong product core gan han:

* modeling
* sculpt
* UV
* shading authoring
* final rendering
* VFX node graph

---

## Product Stack

Phoenix nen duoc nhin nhu 5 tang san pham:

### 1. Workflow Surface

Day la nhung gi user thay va dung hang ngay:

* viewport
* outliner
* channel box / inspector
* timeline
* playback controls
* script editor
* future dope sheet / graph editor

Muc tieu cua tang nay:

* thao tac nhanh
* phan hoi ro
* it friction cho animator

### 2. Authoring Capabilities

Day la lop nghiep vu ma workflow surface kich hoat:

* scene authoring
* selection va transform editing
* joint authoring
* skeleton hierarchy editing
* bind pose capture
* skin binding
* keyframe editing
* playback navigation
* file import/export

Day la tang dinh nghia `Phoenix co lam duoc viec gi`.

### 3. Canonical Domain Model

Day la source of truth cua du lieu:

* `Scene`
* `SceneObject`
* `Transform`
* `Joint`
* `Skeleton Hierarchy`
* `AnimationData`
* `SkinningData`
* `PlaybackState`
* `PhoenixSceneDocument`

Nguyen tac:

* khong de UI tro thanh source of truth
* khong de `FBX` tro thanh internal model
* `joint orientation` tach khoi `animated rotation`

### 4. Runtime Evaluation

Tang nay bien du lieu tac gia thanh ket qua co the xem va playback:

* hierarchy evaluation
* animation sampling
* skin deformation
* viewport render prep
* selection/highlight feedback

Muc tieu:

* dung
* on dinh
* du nhanh cho editor tuong tac

### 5. Platform Extension

Tang nay chua gia tri mo rong:

* script commands
* undo/redo sau nay
* plugin API
* Python API
* AI assistant hooks
* future automation/macros

Nguyen tac:

* mo rong qua seam ro rang
* plugin bo sung workflow, khong duoc pha vo domain core

---

## Focused Module Stack

De giu product architecture gon va hop voi roadmap hien tai, Phoenix nen tap trung truoc vao 6 `module` goc sau:

```text
AnimationStudio/
├── Core/
├── Engine/
├── Scene/
├── Rendering/
├── Animation/
├── Rigging/
```

Day khong chi la cay thu muc.
Day la 6 `module` cap san pham ma moi feature gan han nen map vao.

### 1. Core

Vai tro:

* app lifecycle
* event flow
* command routing
* undo/redo orchestration
* logging
* settings

Nen chua:

* `Application`
* `Command System`
* `Event System`
* `Undo / Redo`
* `Logger`
* `Settings`

Khong nen chua:

* scene business rules
* render backend logic
* rigging math dac thu

### 2. Engine

Vai tro:

* runtime coordination giua data, evaluation, va editor update loop
* time stepping
* playback tick
* service bootstrapping
* future task scheduling / memory policy neu can

Nen chua:

* update loop
* evaluation orchestration
* service registry nho gon
* playback clock

Khong nen chua:

* UI widget code
* importer/exporter adapters

Ghi chu:

`Engine` o Phoenix nen la `editor runtime engine`, khong phai game engine day du.

### 3. Scene

Vai tro:

* canonical scene data
* object identity
* transforms
* hierarchy
* selection state co nghia nghiep vu
* object-level animation ownership

Nen chua:

* `Scene`
* `SceneObject`
* `Transform`
* bounds
* hierarchy rules
* object metadata can cho save/load va viewport

Khong nen chua:

* renderer state
* `FBX` parsing detail

Ghi chu:

`Scene` la source of truth.
`FBX` va UI khong duoc tro thanh source of truth thay cho no.

### 4. Rendering

Vai tro:

* viewport draw pipeline
* camera-view interaction support
* selection/highlight overlays
* skeleton and mesh draw
* future abstraction cho backend rendering

Nen chua:

* `ViewportRenderer`
* shader utilities
* draw data upload
* grid / overlay / highlight
* bridge tu evaluated scene sang GPU

Khong nen chua:

* scene authoring rules
* keyframe editing logic

Ghi chu:

`Rendering` nen dung sau mot seam mong de domain van portable nhu ADR da khoa.

### 5. Animation

Vai tro:

* keyframe data
* playback state
* key editing ops
* timing workflow
* future `Dope Sheet` va `Graph Editor` foundation

Nen chua:

* transform tracks
* set/delete/duplicate/shift key ops
* frame range queries
* next/previous key navigation
* interpolation metadata sau nay

Khong nen chua:

* timeline widget policy thuần UI
* rig-specific orientation rules

Ghi chu:

`Animation` la `module` product core hien tai.
Moi quyet dinh roadmap gan han nen uu tien tang chieu sau cho `module` nay.

### 6. Rigging

Vai tro:

* `Joint`
* `Skeleton Hierarchy`
* `Joint Orientation`
* `Bind Pose`
* skin bind foundations
* deformation dependencies can cho animation playback

Nen chua:

* joint data va contracts
* parent/unparent rules cho skeleton
* bind pose capture
* skin bind data can thiet

Khong nen chua:

* `IK`, `Constraint`, `Auto Rig` neu chua co seam that su can
* weight paint UX o giai doan nay

Ghi chu:

`Rigging` phai dung dung domain language trong [CONTEXT.md](</E:/Animation Software/CONTEXT.md>).
`Joint Orientation` va `Animated Rotation` khong duoc tron vao nhau.

---

## Recommended Dependency Direction

De giu `module` sach, huong phu thuoc nen uu tien nhu sau:

```text
Core -> Engine -> Scene -> Animation -> Rigging -> Rendering
```

Va thuc te trong code co the cho phep:

* `Core` dieu phoi `Engine`
* `Engine` update `Scene`, `Animation`, `Rigging`
* `Rendering` doc du lieu da evaluate de ve

Nguyen tac quan trong:

* `Rendering` khong dua ra luat nghiep vu cho `Scene`
* `UI` khong so huu state canonical cua `Animation`
* `Rigging` mo rong `Scene`, khong tao mot scene model song song

---

## Product Capability Map

Kien truc san pham nen chia theo 4 cot capability lon.

### A. Scene Foundation

Chuc nang:

* tao/mo/save/export scene
* append/import asset
* quan ly object tree
* selection va framing

Vai tro trong san pham:

* la nen cho moi workflow con lai

Trang thai hien tai:

* da co va da dung duoc

### B. Rigging Foundation

Chuc nang:

* tao `joint`
* sua `skeleton hierarchy`
* parent/unparent
* `joint orientation`
* `bind pose`

Vai tro trong san pham:

* cho phep tu scene editor di vao character workflow

Trang thai hien tai:

* da co foundation

### C. Animation Authoring

Chuc nang:

* set/delete/auto key
* scrub/playback
* duplicate/shift keys
* range-based timing edits
* future dope sheet
* future graph editor

Vai tro trong san pham:

* day la product core quan trong nhat

Trang thai hien tai:

* dang la vung uu tien so 1

### D. Extensibility And Automation

Chuc nang:

* script commands
* command routing
* macro/undo-redo seam
* plugin SDK
* Python API
* AI task assistance

Vai tro trong san pham:

* bien Phoenix thanh mot nen tang, khong chi la app dong

Trang thai hien tai:

* foundation da co, product layer chua mo chinh thuc

---

## Recommended Module Boundaries

De tranh scope drift, moi thay doi nen map vao 1 trong 7 module sau.

### 1. Editor Shell

Bao gom:

* `EditorShell`
* docking layout
* panel hosting
* menu/toolbar/action wiring

Khong nen chua:

* scene business rules
* animation evaluation logic

### 2. Scene Domain

Bao gom:

* scene tree
* object identity
* transforms
* bounds
* parenting rules

Khong nen chua:

* renderer-specific state

### 3. Rigging Domain

Bao gom:

* joint data
* skeleton hierarchy contracts
* joint orientation
* bind pose

Khong nen chua:

* animation editor UI policy

### 4. Animation Domain

Bao gom:

* keyframe tracks
* key editing ops
* playback state
* next/previous key queries
* future interpolation metadata

Khong nen chua:

* timeline widget state phu thuoc UI toolkit

### 5. Deformation Runtime

Bao gom:

* skin bind data
* deformation eval
* mesh update for viewport

Khong nen chua:

* file format parsing

### 6. IO And Serialization

Bao gom:

* `FBX` import/export adapters
* validation
* `PhoenixSceneDocument`
* message formatting

Khong nen chua:

* canonical business rules bi duplicate lai

### 7. Extensibility Layer

Bao gom:

* command registry
* script surface
* plugin contracts
* future Python bridge

Khong nen chua:

* direct mutation paths bo qua domain validation

---

## Primary User Flows

Kien truc san pham nen toi uu 4 luong chinh.

### Flow 1: Import To Inspect

`Import FBX -> Scene build -> Outliner + Viewport display -> Select -> Frame`

Gia tri:

* user vao scene nhanh
* fail path ro va recover duoc

### Flow 2: Rig To Bind

`Create joint -> Build hierarchy -> Adjust joint orientation -> Capture bind pose -> Bind mesh`

Gia tri:

* cho phep dung skeleton va mesh cho animation

### Flow 3: Animate To Preview

`Select object/joint -> Set key -> Scrub -> Edit key timing -> Playback -> Save scene`

Gia tri:

* day la flow quan trong nhat cua `v0.6 -> v1`

### Flow 4: Script To Extend

`Trigger command -> Reuse same domain seam -> Produce repeatable workflow`

Gia tri:

* mo duong cho automation, plugin, va AI

---

## Architecture Priorities By Phase

### Now: Beta Core

Tu ngay `2026-07-21`, uu tien kien truc nen la:

1. cung co `Animation Authoring`
2. giu domain seams sach giua `scene`, `rigging`, `animation`
3. mo rong `command system` de moi thao tac UI co the script duoc
4. khong mo scope sang plugin SDK day du qua som

### Next: Production Usability

Sau khi `v0.6` on:

* them timeline range foundation vung chac
* mo `Dope Sheet`
* chuan bi metadata cho `Graph Editor`
* undo/redo stack thanh first-class system

### Later: Platformization

Khi core authoring da vung:

* plugin SDK
* Python API
* macro automation
* AI assistant
* cloud/collab chi nen vao sau khi local authoring workflow da that su tot

---

## Product Architecture Principles

### Principle 1: Animation Depth Beats Feature Breadth

Khong nen them nhieu module ngang hang neu key/timing/pose workflow van nong.

### Principle 2: One Canonical Data Path

UI, script, va future plugin phai di qua cung domain seam khi sua scene, rig, animation.

### Principle 3: Workflow Features Sit On Stable Foundations

`Dope Sheet`, `Graph Editor`, `AI`, `Plugin SDK` deu nen dung tren:

* scene model on dinh
* animation ops ro rang
* command contracts thong nhat

### Principle 4: Preserve Domain Language

Docs, code, va test phai dung dung ngon ngu da khoa trong [CONTEXT.md](</E:/Animation Software/CONTEXT.md>), dac biet:

* `Joint`
* `Skeleton Hierarchy`
* `Joint Orientation`
* `Animated Rotation`
* `Bind Pose`

### Principle 5: Separate Beta Necessity From Future Ambition

Neu 1 thanh phan chua can cho animator hoan thanh workflow co gia tri trong beta, no khong nen keo cham core.

---

## Risks And Controls

### Risk 1: Product bi chia doi giua editor shell va animation tool

Control:

* xem `Animation Authoring` la product core
* shell chi phuc vu workflow nay

### Risk 2: Plugin-first qua som

Control:

* xay seam mo rong san, nhung chi public khi domain contracts da on

### Risk 3: UI tao ra logic rieng

Control:

* moi action UI route qua command hoac domain helper dung chung

### Risk 4: Roadmap phinh ngang

Control:

* khoa thu tu capability:
  * scene foundation
  * rigging foundation
  * animation depth
  * extensibility
  * AI/platform

---

## Recommended Decision Frame

Moi feature moi nen duoc danh gia qua 5 cau hoi:

1. no phuc vu flow chinh nao?
2. no thuoc module nao?
3. no sua canonical data hay chi la presentation?
4. no co mo them seam dung lai duoc cho script/plugin khong?
5. no co lam cham `animation authoring core` khong?

Neu khong tra loi ro 5 cau hoi nay, feature do chua san sang de vao implementation.

---

## Concrete Recommendation For Current Stage

Trong giai doan hien tai, product architecture nen chot nhu sau:

* `Animation Authoring` la trung tam cua roadmap
* `Scene`, `Rigging`, `Skinning` la foundation da du tot de ho tro no
* `Script Command System` la seam chien luoc cho extensibility
* `Dope Sheet` va `Graph Editor` nen duoc mo ra tu timeline/data seam hien co, khong rebuild song song
* `Plugin SDK`, `Python API`, `AI Assistant` nen duoc thiet ke boundary tu bay gio, nhung public sau khi core workflow vung

Noi ngan gon:

`Phoenix` khong nen duoc thiet ke nhu "mot DCC day du".  
No nen duoc thiet ke nhu "mot animation authoring platform gon, mo rong duoc, va rat gioi o workflow key/pose/timing".
