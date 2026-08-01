# Architecture Scaling Roadmap

Tai lieu nay chot roadmap kien truc tiep theo cho repo sau khi da tach boundary chinh cua:

* `Core`
* `Engine`
* `Scene`
* `Rendering`
* `Animation`
* `Rigging`

Muc tieu:

* giu codebase tiep tuc mo rong duoc
* tranh bi tro lai mot cum lon quanh `Scene` hoac `EditorShell`
* thong nhat huong refactor cho cac vong tiep theo

Tai lieu lien quan nen doc kem:

* [Architecture_Document_Map.md](</E:/Animation Software/docs/Architecture_Document_Map.md:1>)
* [Scene_Module_Boundary_Map.md](</E:/Animation Software/docs/Scene_Module_Boundary_Map.md:1>)
* [Rendering_Module_Boundary_Map.md](</E:/Animation Software/docs/Rendering_Module_Boundary_Map.md:1>)
* [Animation_Module_Boundary_Map.md](</E:/Animation Software/docs/Animation_Module_Boundary_Map.md:1>)
* [Rigging_Module_Boundary_Map.md](</E:/Animation Software/docs/Rigging_Module_Boundary_Map.md:1>)

## Trang thai hien tai

Codebase hien tai da co nhieu boundary tot hon truoc:

* domain core da ro hon
* editor shared seam da bat dau ro pattern
* runtime bridge va UI bridge da tach duoc mot phan lon
* tai lieu boundary map da co cho `Rendering`, `Animation`, va `Rigging`

Tuy vay, van con 3 nut that chinh can xu ly neu muon scale tiep.

## Roadmap 1: Lam Mong `Scene`

### Muc tieu

Giam vai tro `Scene` tu "noi moi module deu phai di qua" thanh:

* canonical host cua object graph
* lifecycle owner
* facade mong cho domain seams

### Van de hien tai

`Scene` va `SceneObject` van dang expose nhieu API cua:

* `Animation`
* `Rigging`
* viewport/runtime mutation

Dieu nay hop ly o giai doan hien tai, nhung neu de lau:

* `Scene` se tiep tuc phinh
* boundary giua domain modules se mo dan
* feature moi de quay lai "them vao Scene cho nhanh"

### Huong di

* uu tien dua business logic vao cac seam chuyen biet truoc
* giu `Scene` o vai tro facade, delegate thay vi om logic
* tan dan giam so API module-specific lo truc tiep tren `SceneObject`

### Dau hieu xong tot

* `Scene.cpp` khong tiep tuc to ra dang ke qua moi vong refactor
* mutation/evaluation logic song trong seam rieng, khong quay lai `Scene`
* doc `Scene` thay ro no la host, khong phai cho xu ly tat ca

## Roadmap 2: Chuan Hoa Pattern `Editor Seam`

### Muc tieu

Chot mot pattern lap lai duoc cho cac module editor-facing.

Pattern de xuat hien tai:

* `State`
* `Controller`
* `SceneMutation`
* `FlowController`
* `ViewBuilder`

### Gia tri

Neu pattern nay duoc chot thanh convention:

* nguoi moi doc code de doan dung cho can sua
* feature moi it lam lech kien truc
* refactor giua cac module co the lap lai cach lam thay vi moi lan nghi lai tu dau

### Huong di

* giu `Controller` cho state math / local policy don gian
* giu `SceneMutation` cho scene snapshot -> scene result
* giu `FlowController` cho workflow + validation + result shaping
* giu `ViewBuilder` cho projection sang UI model
* tranh de widget/shell nhan lai logic cua cac lop tren

### Ung vien ap dung tiep

* `Scene`
* `Selection`
* `Scripting`
* mot phan `Rigging UI`
* mot phan `Viewport/Inspector` workflow

### Dau hieu xong tot

* module moi vao repo co structure de doan duoc
* file ten giong nhau nhung ownership khong con nham lan
* giam so controller "tap ky" trong `apps/editor`

## Roadmap 3: Tach Ro `UI Projection` Khoi `Runtime Orchestration`

### Muc tieu

Dam bao panel/widget chi con la noi:

* nhan view model
* hien thi state
* phat callback/action

Con runtime orchestration se chiu trach nhiem:

* apply scene
* undo/redo
* selection refresh
* script/result flow
* dong bo giua editor state va runtime state

### Van de hien tai

Codebase da tien bo nhieu, nhung van con mot so bridge lon:

* `EditorShell`
* `EditorSceneRuntimeController`
* `EditorViewportSceneController`

Neu them nhieu feature cung luc, day la cac diem de day logic quay tro lai rat nhanh.

### Huong di

* tiep tuc day projection sang `ViewBuilder` va UI model
* tiep tuc giu widget/panel la noi wiring callback
* giu apply scene / apply state / history refresh o lop runtime orchestration
* tranh de panel moi tu goi mutation graph truc tiep

### Loi ich mo rong

Buoc nay se mo duong de ho tro tot hon cho:

* multi-document editor
* detachable panel/workspace
* alternate shell UI
* headless runtime/test harness
* command automation va script integration sau hon

### Dau hieu xong tot

* widget/panel moi chu yeu la view + callback wiring
* runtime layer xu ly state transition va side-effect tap trung
* UI co the thay doi ma it anh huong domain/runtime

## Thu Tu Uu Tien

Thu tu de xuat:

1. `Lam mong Scene`
2. `Chuan hoa editor seam`
3. `Tach UI projection khoi runtime orchestration`

Ly do:

* neu `Scene` khong duoc lam mong, moi module moi deu co nguy co quay lai day logic vao cung mot cho
* khi pattern `editor seam` ro, moi refactor sau de lam hon
* sau cung moi toi uu UI/runtime split de mo duong scale shell va workspace

## Cach Su Dung Roadmap Nay

Tai lieu nay khong bat buoc phai lam mot lan.

Nen dung no nhu:

* moc review truoc khi bat dau refactor lon
* checklist khi phat hien mot module dang day tro lai `Scene` hoac `EditorShell`
* reference de quyet dinh file moi nen dat o domain, editor seam, UI, hay engine/runtime

## Ket luan

Mo hinh hien tai da du tot de phat trien tiep, nhung de scale ben vung hon:

* `Scene` can tiep tuc mong di
* `editor seam` can tro thanh pattern chuan
* `UI projection` va `runtime orchestration` can tiep tuc tach ro

Neu giu dung 3 huong nay, codebase se mo rong de hon ma khong can quay lai don nhung cum lon nhu truoc.
