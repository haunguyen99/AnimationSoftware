# Architecture Document Map

Tai lieu nay la diem vao chung cho cum tai lieu kien truc hien tai trong `docs/`.

Muc tieu:

* cho mot noi bat dau de doc kien truc
* gom cac tai lieu boundary map va roadmap thanh mot cum ro rang
* giam viec moi lan can review kien truc lai phai tu tim file

## Nen Doc Theo Thu Tu Nay

1. [Architecture_Scaling_Roadmap.md](</E:/Animation Software/docs/Architecture_Scaling_Roadmap.md:1>)
2. [Module_Dependency_Diagram.md](</E:/Animation Software/docs/Module_Dependency_Diagram.md:1>)
3. [Module_To_Code_Map.md](</E:/Animation Software/docs/Module_To_Code_Map.md:1>)
4. Boundary maps theo module

## Boundary Maps Hien Co

* [Scene_Module_Boundary_Map.md](</E:/Animation Software/docs/Scene_Module_Boundary_Map.md:1>)
* [Rendering_Module_Boundary_Map.md](</E:/Animation Software/docs/Rendering_Module_Boundary_Map.md:1>)
* [Animation_Module_Boundary_Map.md](</E:/Animation Software/docs/Animation_Module_Boundary_Map.md:1>)
* [Rigging_Module_Boundary_Map.md](</E:/Animation Software/docs/Rigging_Module_Boundary_Map.md:1>)

## Tai Lieu Nen Dung Kem

* [Product_Architecture_Blueprint.md](</E:/Animation Software/docs/Product_Architecture_Blueprint.md:1>)
* [Development_History.md](</E:/Animation Software/docs/Development_History.md:1>)
* [Editor_Shell_Ownership_Note.md](</E:/Animation Software/docs/Editor_Shell_Ownership_Note.md:1>)
* [MainWindow_Core_Extraction_Checklist.md](</E:/Animation Software/docs/MainWindow_Core_Extraction_Checklist.md:1>)

## Cach Dung Cum Tai Lieu Nay

Khi can review mot module:

* doc `Architecture_Scaling_Roadmap.md` de nho huong scale tong the
* doc boundary map cua module do de thay ownership hien tai
* doi chieu `Module_Dependency_Diagram.md` neu can xem module do dung voi ai

Khi can mo rong feature moi:

* xac dinh feature do thuoc domain, editor seam, UI, hay engine/runtime
* tim boundary map gan nhat de dat file dung ownership
* neu module chua co boundary map, xem day la ung vien can bo sung tai lieu

## Ket Luan

Cum tai lieu kien truc hien tai nen duoc doc nhu:

* `Roadmap` = huong scale tong the
* `Boundary maps` = ownership hien tai theo module
* `Dependency/map docs` = boi canh lien module

Neu giu cap nhat cum tai lieu nay sau moi dot refactor lon, codebase se de dieu huong hon rat nhieu.
