# ADR-002: Beta Editor Shell Uses Qt Widgets with OpenGL Viewport Host

* Status: `Accepted`
* Date: `2026-07-04`

## Context

Vision tổng thể nhắm `Qt 6 + QML`. Nhưng `v1 beta` scope rất nhỏ:

* main window
* import `FBX`
* viewport
* orbit/pan/zoom

Nếu dựng full `QML` shell từ đầu, beta có thể chậm vì:

* thêm lớp integration `QML <-> C++`
* tăng số moving parts
* chưa mang lại giá trị rõ cho scope hiện tại

Ta cần chọn shell UI nhỏ nhất để ship beta nhanh.

## Decision

Cho `v1 beta`, editor shell dùng:

* `Qt Widgets` cho main window, menu, toolbar, status bar
* `QOpenGLWidget` hoặc OpenGL host tương đương cho viewport

`QML` chưa là lớp UI chính trong beta.

Khi sang phase lớn hơn, team có thể:

* giữ `Widgets` cho shell
* hoặc chuyển dần panel/UI mới sang `QML`

## Consequences

Tốt:

* giảm complexity giai đoạn đầu
* viewport embedding tự nhiên hơn
* file dialog, menu, action wiring nhanh
* ít bridge code hơn

Đổi lại:

* khác nhẹ với vision `Qt + QML` dài hạn
* nếu sau này chuyển nhiều UI sang `QML`, cần thêm integration plan

## Alternatives Considered

### QML ngay từ đầu

Không chọn cho beta:

* hợp vision sản phẩm hơn
* nhưng nặng setup hơn nhu cầu hiện tại

### Dear ImGui shell

Không chọn:

* rất nhanh cho prototype
* nhưng không hợp hướng sản phẩm/editor desktop dài hạn

