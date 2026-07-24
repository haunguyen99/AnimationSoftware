# Feature: Animation Tools v0.6 Roadmap

## Status

`Step 1 - Roadmap Locked`

Source anchors:

* [Development_Roadmap.md](</E:/Animation Software/docs/Development_Roadmap.md>)
* [CURRENT_STATE.md](</E:/Animation Software/CURRENT_STATE.md>)
* [CONTEXT.md](</E:/Animation Software/CONTEXT.md>)

---

## Goal

Day `Project Phoenix` tu moc `animation data co ban` len muc `animation authoring` thuc dung hon cho animator.

`v0.6` khong nham lam full `Graph Editor` hay full `Dope Sheet`.

`v0.6` nham khoa 1 lop cong cu giua:

* edit key nhanh hon
* doi timing an toan hon
* giam phu thuoc vao viec sua transform tay tung frame
* mo nen cho `v0.7 Graph Editor`

---

## Current Baseline

Da co san trong repo:

* `Set Key`
* `Delete Key`
* `Auto Key`
* playback range
* current frame control
* timeline key markers
* transform playback evaluation
* script commands cho frame, playback, key dat/delete co ban

Chua co:

* duplicate keys
* offset keys theo frame delta
* shift key range
* key copy/paste workflow
* timeline selection model
* dope sheet panel
* constraints
* mirror animation

Ket luan:

* seam san sang nhat hien tai = `key editing actions`
* seam chua san sang = `constraints`, `mirror`, `full dope sheet`

---

## Product Decision

`v0.6` duoc chia thanh 3 phase.

### Phase 1 - Key Editing Core

Muc tieu:

* khoa nhom thao tac co gia tri cao, risk thap

Scope:

* duplicate key tai current frame hoac duplicate ca track theo frame offset
* offset keyframe range theo delta frame
* shift keys trai / phai nhanh
* improve frame stepping / jump playback controls
* script command support cho key editing actions
* timeline status feedback ro hon khi co key move/copy

Exit:

* user doi timing co ban ma khong phai dat lai tung key thu cong

### Phase 2 - Timeline Interaction Foundation

Muc tieu:

* mo nen cho `Dope Sheet` sau nay ma chua can panel rieng

Scope:

* timeline selection cho keyframe theo frame
* selected frame range feedback
* thao tac key actions dua tren range thay vi chi current frame
* UI hooks de sau nay nang cap thanh `Dope Sheet`

Exit:

* key actions co the chay tren frame range ro rang

### Phase 3 - Playback And Authoring Polish

Muc tieu:

* lam workflow animation tron hon truoc khi mo `v0.7`

Scope:

* step to next/previous keyed frame
* jump to first/last keyed frame cua object
* playback UX polish cho animator
* doc/manual QA cho `v0.6`

Exit:

* user duyet key va timing nhanh hon trong editor

---

## Explicit Non-Goals For v0.6

Khong dua vao `v0.6`:

* `Graph Editor`
* tangent editing
* interpolation UI nang cao
* `Constraints`
* `IK/FK`
* `Mirror Animation`
* animation layers
* weight painting

Ly do:

* nhung item nay hoac thuoc `v0.7+`, hoac can them scene/eval complexity lon

---

## Recommended Build Order

1. Them key editing API vao scene/object seam
2. Them script commands cho duplicate/offset/shift
3. Them UI action/menu cho key editing
4. Nang timeline widget de hien selected/range state toi thieu
5. Them playback jump actions theo key
6. Them unit/UI tests
7. Manual QA + doc sync

---

## Technical Direction

### Data seam

Khuyen nghi giu `TransformKeyframeTrack` lam canonical data.

Khong them model animation moi trong `v0.6`.

Chi can them helper ops:

* copy key
* move key
* clone key range
* query next/previous key

### UI seam

Khuyen nghi tiep tuc dung:

* `MainWindow`
* `KeyframeTimelineWidget`
* `ScriptCommandSystem`

Khong mo panel `Dope Sheet` rieng trong phase dau.

### Command seam

Khuyen nghi them command nhom:

* `copyKey`
* `pasteKey`
* `shiftKey`
* `scaleKey` chi neu can sau

Ban dau:

* uu tien `duplicate/offset/shift`
* chua can `scaleKey`

---

## Risks

### Risk 1 - V0.6 bi phinh scope

Neu dua ca `Dope Sheet`, `Constraints`, `Mirror`, feature se vo nhip.

Control:

* khoa phase 1 vao `Key Editing Core`

### Risk 2 - Timeline widget bi o nhiem qua som

Neu ep no thanh `Dope Sheet` day du ngay, code UI se debt nhanh.

Control:

* chi them selection/range state toi thieu

### Risk 3 - Key edit ops de gay data bug

Duplicate/move key co the de:

* de key trung frame
* mat thu tu sort
* ghi de key khong ro rule

Control:

* moi op phai di qua helper seam co normalize + sort

### Risk 4 - Script/UI behavior lech nhau

Control:

* moi action UI phai route qua seam dung chung voi script command

---

## Validation Gates

`v0.6 phase 1` dat khi:

* duplicate key dung
* shift/offset key dung
* timeline feedback cap nhat dung
* script command va UI action cho ket qua nhu nhau
* `ctest` pass
* runtime QA pass tren scene animation don gian

`v0.6` full dat khi:

* user edit timing co ban nhanh hon ro ret
* co jump/step workflow dua tren key
* docs + QA checklist da sync

---

## Recommended Task Breakdown

### Task 1 - Keyframe Editing APIs

* them helper duplicate/move/query keyframes
* khoa rule sort/overwrite/merge

### Task 2 - Script Commands

* mo command set cho key editing
* standardize result text

### Task 3 - Menu And Timeline Actions

* expose action trong menu
* keyboard shortcut neu seam ro, risk thap

### Task 4 - Timeline Range Foundation

* co selected frame/range state toi thieu
* tiep suc cho range-based key ops

### Task 5 - Playback Navigation By Keys

* next key
* previous key
* jump first/last key

### Task 6 - Tests And Manual QA

* scene tests
* script command tests
* editor UI tests
* manual QA checklist

---

## Definition of Done

`v0.6` coi la xong khi:

* key editing workflow co gia tri thuc te cho animator
* timeline khong chi la view marker nua, ma ho tro timing edits co ban
* playback navigation theo key co mat
* code path duoc test
* docs trang thai + QA duoc cap nhat

---

## After v0.6

Neu `v0.6` khoa on:

* `v0.7` mo vao `Graph Editor`
* `Dope Sheet` co the tach thanh panel rieng dua tren timeline selection foundation da co
