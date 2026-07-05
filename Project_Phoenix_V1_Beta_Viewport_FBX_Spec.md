# Project Phoenix v1 Beta Spec

## 1. Goal

`v1 beta` = bản chạy được nhỏ nhất để chứng minh 3 năng lực:

* có `Viewport`
* import được `FBX` vào `Scene`
* user xoay, pan, zoom để view model

Beta này không nhằm làm tool animation hoàn chỉnh. Beta này nhằm dựng nền kỹ thuật đầu tiên.

---

## 2. Scope

## In Scope

* App khởi động được
* Có cửa sổ editor cơ bản
* Có `Viewport` 3D
* Có `Scene` in-memory tối thiểu
* Import `FBX` model vào scene
* Hiển thị model trong viewport
* Camera controls:
  * orbit
  * pan
  * zoom
* Hiển thị grid cơ bản
* Fit camera to imported model
* Thông báo lỗi import khi file lỗi

## Out of Scope

* Rigging tools
* Animation
* Timeline
* Keyframe
* Graph Editor
* Plugin SDK
* Save/Load project
* Export FBX
* Material editor
* Skinning tools
* Tablet/pen optimization sâu

---

## 3. User Story

User mở app -> import file `FBX` -> model xuất hiện trong viewport -> user orbit/pan/zoom để kiểm tra model.

Nếu flow này ổn -> beta đạt mục tiêu.

---

## 4. Success Criteria

Beta đạt khi:

* App mở ổn định trên máy dev mục tiêu.
* User import được ít nhất 1 file `FBX` model tĩnh hoặc skeletal mesh đơn giản.
* Model hiển thị đúng vị trí/tỷ lệ cơ bản trong viewport.
* Camera orbit/pan/zoom mượt, không giật lớn.
* Có thể focus hoặc fit model sau import.
* Lỗi import được báo rõ, app không crash.

---

## 5. Technical Scope

## 5.1 Minimum Architecture

Giữ nhỏ:

* `Application Layer`
* `Viewport Layer`
* `Scene Layer`
* `FBX Import Layer`
* `Rendering Layer`

Không cần full architecture lớn của sản phẩm cuối ở beta này.

## 5.2 Recommended Stack

* `C++20/23`
* `CMake`
* `Qt 6 + QML` hoặc `Qt Widgets + OpenGL host`
* `OpenGL`
* `GLM`
* `spdlog`

---

## 6. Functional Requirements

## 6.1 Application

App phải:

* mở được main window
* có menu hoặc button `Import FBX`
* có vùng viewport chiếm phần lớn UI

## 6.2 Scene

Scene bản beta chỉ cần:

* giữ danh sách object imported
* giữ transform world/local tối thiểu
* giữ mesh data tham chiếu tới renderer
* giữ bounding box scene

Không cần:

* hierarchy phức tạp
* animation state
* selection framework đầy đủ

## 6.3 FBX Import

Importer phải:

* đọc file `FBX`
* trích mesh data cơ bản
* trích transform cơ bản
* đưa data vào scene format nội bộ

Ưu tiên beta:

* mesh geometry
* node transform
* scene bounds

Không bắt buộc beta:

* material fidelity cao
* skinning data hoàn chỉnh
* animation clips

## 6.4 Viewport

Viewport phải:

* render grid
* render imported model
* render background rõ
* resize đúng theo window
* redraw khi camera đổi hoặc scene đổi

## 6.5 Camera Controls

Camera phải hỗ trợ:

* `Orbit`: quay quanh target
* `Pan`: kéo ngang/dọc
* `Zoom`: tiến/lùi hoặc dolly

Khuyến nghị mapping desktop:

* `Alt + LMB` -> orbit
* `Alt + MMB` -> pan
* `Mouse Wheel` -> zoom

Hoặc mapping khác nếu team muốn, miễn nhất quán.

## 6.6 Fit to Model

Sau khi import:

* camera tự focus vào model
* target camera = center bounding box
* distance camera tính theo bounding sphere hoặc AABB extents

---

## 7. Non-Functional Requirements

## 7.1 Stability

* Import file lỗi không làm app crash
* Không dereference null khi scene rỗng
* Viewport mở được cả khi chưa import gì

## 7.2 Performance

Mục tiêu beta:

* thao tác camera cảm giác mượt với model test cơ bản
* import file demo trong thời gian chấp nhận được trên máy dev

Không cần tối ưu production-grade ở giai đoạn này.

## 7.3 Debuggability

Log tối thiểu:

* app startup
* import start/end
* import fail reason
* viewport init fail

---

## 8. Internal Data Model

## 8.1 Scene

```text
Scene
  - objects[]
  - bounds
```

## 8.2 Scene Object

```text
SceneObject
  - id
  - name
  - localTransform
  - meshHandle / meshDataRef
  - bounds
```

## 8.3 Camera

```text
EditorCamera
  - target
  - distance
  - yaw
  - pitch
  - fov
  - nearPlane
  - farPlane
```

---

## 9. Rendering Requirements

## 9.1 Beta Render Features

Phải có:

* perspective camera
* depth testing
* grid lines
* solid mesh render cơ bản

Nice-to-have, không bắt buộc:

* wireframe toggle
* axis gizmo
* simple lighting

## 9.2 Material Rendering

Beta render có thể đơn giản:

* 1 default material/shader
* màu xám trung tính hoặc vertex color nếu có

Mục tiêu beta = nhìn được model, không phải shading đẹp.

---

## 10. UI Layout

UI beta tối giản:

* top menu hoặc toolbar
* nút/menu `Import FBX`
* vùng viewport lớn
* optional status bar hiển thị file import/log ngắn

Không cần:

* outliner
* inspector
* timeline
* dock system phức tạp

---

## 11. Error Handling

Khi import fail:

* hiện message ngắn, rõ
* ghi log chi tiết hơn
* scene hiện tại không bị hỏng

Ví dụ lỗi:

* file không tồn tại
* format không đọc được
* mesh data rỗng
* importer parse fail

---

## 12. Testing

## 12.1 Manual Test Cases

1. Mở app khi scene rỗng -> viewport hiện grid.
2. Import `FBX` hợp lệ -> model hiện đúng.
3. Orbit/pan/zoom sau import -> camera hoạt động đúng.
4. Resize window -> viewport không méo sai.
5. Import file lỗi -> app báo lỗi, không crash.
6. Import lại file khác -> scene cập nhật đúng.

## 12.2 Technical Tests

Nên có test cho:

* camera math
* bounding box calculation
* import result validation

---

## 13. Milestones

## Milestone 1 - App Shell

* tạo main window
* tạo viewport host
* init OpenGL context
* render grid

## Milestone 2 - Camera

* orbit
* pan
* zoom
* viewport resize handling

## Milestone 3 - Scene Model

* scene container tối thiểu
* mesh reference
* bounds calculation

## Milestone 4 - FBX Import

* file picker
* parse FBX
* convert sang scene object
* push sang renderer

## Milestone 5 - Polish Beta

* fit-to-model
* error dialog/logging
* basic QA pass

---

## 14. Definition of Done

Beta done khi:

* main window chạy ổn
* viewport render được grid
* import được `FBX` test file
* model hiện trong scene
* user orbit/pan/zoom để xem model
* import lỗi không crash app

---

## 15. Recommended Next Step After Beta

Sau beta này, phase kế tiếp hợp lý:

* `Outliner`
* `Selection`
* `Transform gizmo`
* scene hierarchy rõ hơn
* save/load scene

