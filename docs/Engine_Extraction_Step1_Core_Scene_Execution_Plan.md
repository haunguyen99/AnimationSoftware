# Engine Extraction Step 1 - Core To Scene Execution Plan

## Purpose

Tai lieu nay chot ke hoach thuc thi cho buoc `1` trong thu tu tach `Engine`:

`Core -> Scene`

Muc tieu:

* giam dependency truc tiep tu `MainWindow` vao `Scene`
* chuyen scene lifecycle va scene snapshot orchestration ve mot seam phu hop hon
* giu app luon build duoc sau moi buoc nho

Ngay lap ke hoach: `2026-07-22`

---

## Scope Of Step 1

Buoc nay chi tap trung vao canh:

* `Core -> Scene`

Khong lam trong buoc nay:

* tach playback/time khoi `MainWindow`
* tach key editing khoi `MainWindow`
* tach workflow `Rigging`
* redesign `Rendering`

---

## Current Friction

Hien tai `MainWindow` dang chua nhieu hanh vi le ra khong nen biet scene detail sau:

* scene snapshot cho undo/redo
* save/export scene document
* export selection bang cach copy subtree thu cong
* outliner population doc `Scene` truc tiep
* scene validity checks nhu `contains(...)`

Code neo chinh:

* [apps/editor/include/MainWindow.h](</E:/Animation Software/apps/editor/include/MainWindow.h>)
* [apps/editor/src/MainWindow.cpp](</E:/Animation Software/apps/editor/src/MainWindow.cpp>)

Hotspots cu the:

* [apps/editor/src/MainWindow.cpp](</E:/Animation Software/apps/editor/src/MainWindow.cpp:1152>) `saveSceneToPath`
* [apps/editor/src/MainWindow.cpp](</E:/Animation Software/apps/editor/src/MainWindow.cpp:1234>) `exportAll`
* [apps/editor/src/MainWindow.cpp](</E:/Animation Software/apps/editor/src/MainWindow.cpp:1261>) `exportSelection`
* [apps/editor/src/MainWindow.cpp](</E:/Animation Software/apps/editor/src/MainWindow.cpp:2291>) `captureHistoryState`
* [apps/editor/src/MainWindow.cpp](</E:/Animation Software/apps/editor/src/MainWindow.cpp:2300>) `restoreHistoryState`
* [apps/editor/src/MainWindow.cpp](</E:/Animation Software/apps/editor/src/MainWindow.cpp:2360>) `populateOutliner`
* [apps/editor/src/MainWindow.cpp](</E:/Animation Software/apps/editor/src/MainWindow.cpp:3072>) `buildExportSceneForObject`

---

## Refactor Target

Sau buoc nay, `MainWindow` nen:

* gui intent
* nhan ket qua
* refresh UI theo ket qua

`MainWindow` khong nen:

* tu so huu `Scene` snapshot logic
* tu clone subtree scene
* tu biet scene save/export chi tiet

Seam de deepening:

* mot `module` dieu phoi tam thoi trong `apps/editor` hoac `src/app` sau nay
* ten tam thoi de planning: `EditorDocumentController`

Luu y:

* day la ten planning
* chua can sua `CONTEXT.md` vi no la ten kien truc, khong phai term domain

---

## Proposed Extraction Slices

### Slice 1 - Extract scene document operations

Files:

* [apps/editor/src/MainWindow.cpp](</E:/Animation Software/apps/editor/src/MainWindow.cpp>)
* [src/io/PhoenixSceneDocument.h](</E:/Animation Software/src/io/PhoenixSceneDocument.h>)
* [src/io/PhoenixSceneDocument.cpp](</E:/Animation Software/src/io/PhoenixSceneDocument.cpp>)

Change:

* tao mot seam nho cho:
  * save current scene
  * export all scene
  * export selection scene
* `MainWindow` khong goi `PhoenixSceneDocument::saveToFile(...)` truc tiep nua

Small deliverable:

* them 1 helper class hoac free functions trong mot file moi, vi du:
  * `apps/editor/include/EditorDocumentController.h`
  * `apps/editor/src/EditorDocumentController.cpp`

Why first:

* risk thap
* tang `locality` cho file workflow
* tach duoc mot cum lon dependency ma khong dung vao playback hay timeline

### Slice 2 - Extract export-selection subtree copy

Files:

* [apps/editor/src/MainWindow.cpp](</E:/Animation Software/apps/editor/src/MainWindow.cpp>)

Change:

* dua `buildExportSceneForObject(...)`
* dua `copyObjectSubtreeToScene(...)`
* ra khoi `MainWindow`

Preferred destination:

* cung `EditorDocumentController`
* hoac mot helper thuoc `Scene export` seam neu thay hop ly hon

Why second:

* day la logic sau hon `UI`
* deletion test ro: xoa khoi `MainWindow`, complexity khong mat ma tap trung ve noi dung hon

### Slice 3 - Extract scene history snapshots

Files:

* [apps/editor/include/MainWindow.h](</E:/Animation Software/apps/editor/include/MainWindow.h>)
* [apps/editor/src/MainWindow.cpp](</E:/Animation Software/apps/editor/src/MainWindow.cpp>)

Change:

* dua `EditorHistoryState`
* dua `captureHistoryState()`
* dua `restoreHistoryState(...)`
* dua `recordUndoState()`, `undoLastChange()`, `redoLastChange()` policy neu can

Preferred destination:

* mot seam nho nhu `EditorHistoryController`
* hoac gop vao `EditorDocumentController` neu interface van nho

Why third:

* no van cham vao UI state nhu selection, current frame, marked parent
* can tach sau khi scene document flow da ro hon

### Slice 4 - Extract read-only scene queries for outliner

Files:

* [apps/editor/src/MainWindow.cpp](</E:/Animation Software/apps/editor/src/MainWindow.cpp>)

Change:

* giam viec `MainWindow` tu doc `Scene` cho outliner population
* co the them 1 seam read model nho cho:
  * root ids
  * display nodes
  * promoted node decision inputs

Why fourth:

* day la canh `Core -> Scene` muc read-only
* co gia tri, nhung khong can cat som hon document/history lifecycle

### Slice 5 - Leave UI refresh calls in MainWindow

Files:

* [apps/editor/src/MainWindow.cpp](</E:/Animation Software/apps/editor/src/MainWindow.cpp>)

Keep in place:

* `statusBar()->showMessage(...)`
* `QMessageBox`
* dock/widget refresh
* script history append

Why:

* day van la trach nhiem hop ly cua `Core`
* buoc 1 khong nen co gang “purify” het `MainWindow`

---

## Recommended Commit-Sized Steps

### Commit 1

Goal:

* tao `EditorDocumentController` toi thieu

Change:

* them header/source moi
* move save/export-all helper logic vao do
* `MainWindow` goi qua seam moi

Verification:

* build pass
* save scene van chay
* export all van chay

### Commit 2

Goal:

* move export-selection subtree clone

Change:

* dua `buildExportSceneForObject(...)`
* dua `copyObjectSubtreeToScene(...)`
* `MainWindow::exportSelection()` goi seam moi

Verification:

* build pass
* export selection van giu dung hierarchy, mesh, animation, rigging data

### Commit 3

Goal:

* move history snapshot logic

Change:

* extract `EditorHistoryState`
* move capture/restore/undo/redo orchestration ra khoi `MainWindow`

Verification:

* build pass
* undo/redo van pass cho:
  * transform changes
  * import
  * parenting
  * key edits

### Commit 4

Goal:

* make `MainWindow` doc scene it hon cho outliner/selection helpers

Change:

* tao read helpers hoac read-model nho
* giam so call site `viewport_->scene().findObject(...)` o nhom outliner workflow

Verification:

* build pass
* outliner van populate dung
* promoted root behavior van dung

---

## Suggested Interface Direction

Chua khoa exact API, nhung seam moi nen co huong nhu sau:

* `saveScene(...)`
* `exportAll(...)`
* `exportSelection(...)`
* `captureEditorState(...)`
* `restoreEditorState(...)`

Nguyen tac:

* `interface` nen noi theo editor intent
* khong expose qua nhieu `Scene` mutation detail cho `MainWindow`

---

## Risks

### Risk 1 - Extract qua to mot lan

Control:

* giu theo commit-sized slices
* moi commit chi chuyen 1 cum hanh vi

### Risk 2 - Undo/redo vo ngầm

Control:

* de history extraction sau document extraction
* verify tren import, transform, parenting, key edit

### Risk 3 - Export selection mat data

Control:

* giu nguyen logic clone hien tai, chi doi placement
* test scene co mesh + joint + keyframe + skin bind

### Risk 4 - MainWindow van mang interface qua rong

Control:

* buoc 1 chi nham cat canh `Core -> Scene`
* khong co gang xu ly ca playback/animation trong cung pha

---

## Definition Of Done For Step 1

Buoc `Core -> Scene` duoc xem la xong khi:

* `MainWindow` khong goi truc tiep `PhoenixSceneDocument::saveToFile(...)`
* logic export selection khong con nam trong `MainWindow`
* scene snapshot/history logic da duoc dong goi sau mot seam rieng
* so call site `viewport_->scene()` trong `MainWindow` giam ro ret o nhom document/history/export
* build pass
* smoke test pass cho:
  * save
  * export all
  * export selection
  * undo/redo

---

## Recommended Start Point

Neu bat dau ngay, diem mo khoi dau tot nhat la:

1. tao `EditorDocumentController`
2. move `saveSceneToPath(...)` va `exportAll()`
3. move `buildExportSceneForObject(...)` va `copyObjectSubtreeToScene(...)`

Ly do:

* leverage cao
* risk thap
* tao hinh dang dau tien cho seam moi truoc khi dung vao undo/redo
