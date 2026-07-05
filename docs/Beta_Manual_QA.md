# Beta Manual QA

Tai lieu nay ghi lai cac buoc manual QA cho `Project Phoenix v1 beta`.

## Scope

Beta hien tai chi cover:

* viewport
* import FBX
* orbit / pan / zoom
* frame scene
* grid / axis / shading co ban
* error handling cho import fail

## Test Assets

Dung cac file trong [assets/sample/README.md](/E:/Animation%20Software/assets/sample/README.md):

* [box_static.fbx](/E:/Animation%20Software/assets/sample/box_static.fbx)
* [hierarchy_multi_mesh.fbx](/E:/Animation%20Software/assets/sample/hierarchy_multi_mesh.fbx)
* [corrupt_minimal.fbx](/E:/Animation%20Software/assets/sample/corrupt_minimal.fbx)

## Must Pass

1. Empty scene
Expected:
app launch duoc, viewport hien background + grid + axis, khong crash.

2. Import valid FBX
Action:
import `box_static.fbx`.
Expected:
model hien trong viewport, camera tu frame vao model, status bar hien import stats.

3. Camera interaction
Action:
`Alt+LMB` orbit, `Alt+MMB` pan, wheel zoom.
Expected:
camera phan hoi muot, khong giat lon, grid/axis/model giu dung quan he world-space.

4. Resize viewport
Action:
resize window nho/lon, maximize/restore.
Expected:
viewport redraw dung, khong stretch loi, camera va model van nhin thay duoc.

5. Import second file
Action:
import `hierarchy_multi_mesh.fbx` sau khi da co scene.
Expected:
scene append duoc, nhieu mesh hien dung, app khong crash, `Frame Scene` van hoat dong.

6. Frame Scene
Action:
pan/orbit di xa roi bam `Frame Scene`.
Expected:
camera quay lai framing hop ly quanh toan bo scene.

7. Render options
Action:
toggle `Wireframe`, `Show Axis`, `Backface Culling`.
Expected:
tung option thay doi ngay, khong crash, shading van on dinh.

8. Import invalid FBX
Action:
import `corrupt_minimal.fbx`.
Expected:
app hien loi, khong crash, viewport van tiep tuc dung duoc, sau do import file tot lai van duoc.

## Exit Criteria

Beta manual QA duoc xem la pass khi:

* tat ca `Must Pass` deu dat
* khong co crash
* khong co regression o import, camera, grid/axis, shading
