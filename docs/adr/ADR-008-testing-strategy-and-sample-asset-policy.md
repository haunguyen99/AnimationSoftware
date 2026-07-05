# ADR-008: Testing Strategy and Sample Asset Policy

* Status: `Accepted`
* Date: `2026-07-04`

## Context

Beta có 2 rủi ro lớn:

* camera/render math sai
* asset import fail do file test không ổn định

Project cần policy sớm cho test và sample assets để tránh:

* test phụ thuộc asset ngẫu nhiên
* file test quá nặng
* repo khó chia sẻ

## Decision

Test strategy cho beta:

* `unit tests` cho camera math, bounds, transform basics
* `integration tests` cho import pipeline mức scene result
* `manual QA checklist` cho viewport interaction

Sample asset policy:

* dùng tập asset nhỏ, có chủ đích trong `assets/sample/`
* ưu tiên file nhẹ, dễ commit, dễ chia sẻ nội bộ
* mỗi asset có note ngắn mô tả mục đích test
* asset test phải hợp pháp để lưu trong repo hoặc có hướng dẫn lấy ngoài repo

## Consequences

Tốt:

* test tập trung đúng chỗ rủi ro
* asset test có kiểm soát
* QA lặp lại dễ hơn

Đổi lại:

* cần duy trì metadata cho sample assets
* một số test import thật có thể nặng hơn unit test thuần

## Alternatives Considered

### Chỉ manual test

Không chọn:

* nhanh lúc đầu
* nhưng lỗi math/import dễ quay lại

### Dùng asset ngẫu nhiên từ internet/dev machine

Không chọn:

* không tái lập
* rủi ro bản quyền
* khó CI về sau

