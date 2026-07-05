# Feature Workflow

## Purpose

Chuẩn hóa cách xây dựng feature cho `Project Phoenix`.

Mục tiêu:

* không nhảy vào code quá sớm
* giảm rework
* giữ kiến trúc sạch
* giúp AI và người làm việc cùng quy trình

---

## Rule

`Design trước. Code sau.`

Mọi feature mới đi qua các bước dưới đây.

---

## Step 1 - Design

Ở bước này:

* không viết code production
* không scaffold implementation
* không merge logic tạm

Chỉ làm:

* mục tiêu feature
* scope
* out-of-scope
* use cases
* assumptions
* constraints
* UML
* data flow
* design pattern
* interface boundary
* risk analysis
* ADR nếu có quyết định kiến trúc mới

## Required Outputs for Step 1

Mỗi feature design phải có ít nhất:

* `Feature Summary`
* `Scope / Out-of-scope`
* `UML`
* `Data Flow`
* `Design Pattern`
* `Risks`
* `Open Questions`

## Design Review Gate

Chỉ được sang step 2 khi:

* design được duyệt
* không còn mơ hồ lớn ở data flow
* boundary giữa module đã rõ

---

## Step 2 - Implementation Plan

Sau khi design được duyệt:

* chia task nhỏ
* xác định thứ tự làm
* xác định dependency
* định nghĩa test plan
* định nghĩa `Definition of Done`

Output:

* task breakdown
* implementation milestones
* validation checklist

---

## Step 3 - Implementation

Chỉ ở bước này mới:

* viết code
* scaffold module
* thêm dependency
* nối UI/backend/runtime

Nguyên tắc:

* bám design đã duyệt
* nếu lệch design -> quay lại cập nhật design/ADR trước

---

## Step 4 - Verification

Phải kiểm:

* build
* unit test nếu có
* integration test nếu có
* manual QA flow
* regression risk

---

## Step 5 - Documentation Update

Sau mỗi feature đáng kể:

* update `CURRENT_STATE.md`
* update spec nếu scope đổi
* update ADR nếu quyết định kiến trúc đổi
* ghi `next steps`

---

## Design Template

Khi bắt đầu 1 feature, dùng skeleton này:

```md
# Feature: <name>

## Goal

## Scope

## Out-of-scope

## Use Cases

## Constraints

## UML

## Data Flow

## Design Pattern

## Module Boundaries

## Risks

## Open Questions
```

---

## Working Agreement for AI

Nếu user yêu cầu `thiết kế`:

* chỉ làm design artifacts
* không tự viết code

Nếu user chưa duyệt design:

* không tự chuyển sang implementation

Nếu implementation làm lộ vấn đề kiến trúc mới:

* dừng
* cập nhật design/ADR
* rồi mới tiếp tục

