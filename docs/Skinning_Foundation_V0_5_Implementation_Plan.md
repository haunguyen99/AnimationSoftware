# Feature: Skinning Foundation v0.5 Implementation Plan

## Status

`Step 3 - Complete`

Design source:

* [Development_Roadmap.md](</E:/Animation Software/docs/Development_Roadmap.md>)
* [Rigging_Foundation_V0_4_Design.md](</E:/Animation Software/docs/Rigging_Foundation_V0_4_Design.md>)
* [Rigging_Foundation_V0_4_Implementation_Plan.md](</E:/Animation Software/docs/Rigging_Foundation_V0_4_Implementation_Plan.md>)

Gate status:

* `v0.4` da xong
* co `joint`, `bindPose`, hierarchy workflow
* da co skin data model, bind rule, deformation runtime, bind UI/command, weight normalization
* runtime QA da pass trong app
* doc da sync

Progress snapshot:

* done: `Task 1` -> `Task 8`

---

## Task Breakdown

### Task 1 - Skin Data Model

Muc tieu:

* them `SkinCluster` hoac equivalent vao scene model
* luu quan he `mesh <-> joints`
* luu weight theo vertex

Output:

* scene model du cho bind va deform mesh

Status:

* done

---

### Task 2 - Bind Workflow Rules

Muc tieu:

* chot rule `bind selected mesh to selected skeleton`
* seed weight mac dinh an toan
* chot cach xu ly neu mesh da bind truoc do

Output:

* co seam ro de tao skin cluster tu UI va script

Status:

* done

---

### Task 3 - Serialization Upgrade

Muc tieu:

* save/load skin cluster
* save/load joint list va weights
* giu backward compatibility voi scene khong co skin

Output:

* scene file giu duoc skinning foundation data

Status:

* done

---

### Task 4 - Deformation Evaluation

Muc tieu:

* tinh vertex deformation tu joint transforms
* dung `bindPose` lam moc
* update viewport mesh theo frame playback

Output:

* mesh deform dung theo joint animation trong viewport

Status:

* done

---

### Task 5 - Viewport Upload Path

Muc tieu:

* cap nhat renderer de nhan vertex da deform
* tranh pha vo path render mesh tinh hien tai
* giu selected feedback va bounds hop le

Output:

* viewport hien mesh skinning on dinh

Status:

* done o muc foundation
* con space cho bounds / perf polish

---

### Task 6 - Bind Commands And UI

Muc tieu:

* them action `Bind Skin`
* them command script cho bind workflow
* expose thong tin skin toi thieu trong UI

Output:

* user co duong thao tac skinning co ban

Status:

* done

---

### Task 7 - Weight Normalization

Muc tieu:

* chot tong weight moi vertex = `1.0`
* bo qua weight rat nho neu can
* validate index / count an toan

Output:

* deformation khong bi vo vi weight xau

Status:

* done

---

### Task 8 - Tests And QA

Muc tieu:

* unit test cho bind va weight normalize
* unit test cho save/load skin data
* runtime test cho playback deformation
* manual QA cho bind scene don gian

Output:

* `v0.5` co xac nhan build + test + manual QA

Status:

* done

---

## Implementation Order

1. `Skin Data Model`
2. `Bind Workflow Rules`
3. `Serialization Upgrade`
4. `Weight Normalization`
5. `Deformation Evaluation`
6. `Viewport Upload Path`
7. `Bind Commands And UI`
8. `Tests And QA`

Reason:

* phai khoa scene data truoc UI
* save/load nen co som de tranh doi format muon
* deformation can di tren data model on dinh
* viewport polish theo sau runtime core

---

## Dependencies

Phu thuoc san co:

* `Scene`
* `SceneObject`
* `PhoenixSceneDocument`
* `ViewportRenderer`
* `ViewportWidget`
* `MainWindow`
* `bindPose` data tu `v0.4`
* `jointOrientation` split theo `ADR-009`

Phu thuoc moi:

* khong nen them third-party moi cho `v0.5` neu chua can

---

## Validation Checklist

* bind duoc 1 mesh vao skeleton don gian
* scene co skin cluster data hop le
* moi vertex co tong weight = `1.0`
* playback joint animation lam mesh deform trong viewport
* save/load giu duoc skinning data
* scene cu khong co skin van mo duoc
* app build duoc
* `ctest` pass

---

## Test Plan

### Build

* build bang flow hien co cua project

### Automated Tests

1. data model test:
   * tao skin cluster
   * bind mesh vao joints
   * normalize weight
   * reject invalid joint / vertex mapping

2. serialization test:
   * save/load scene co skin cluster
   * save/load weights
   * backward compatibility voi scene cu

3. deformation test:
   * joint identity -> mesh khong lech
   * joint move/rotate -> vertex position doi theo expected co ban
   * playback frame doi -> mesh cap nhat

4. UI / command test:
   * bind skin bang action
   * bind skin bang command
   * scene panels van sync dung

### Manual QA

1. mo app scene co mesh + skeleton
2. bind mesh vao skeleton
3. set key cho joint
4. playback
5. xac nhan mesh deform
6. save scene
7. mo lai scene
8. xac nhan deform van dung

---

## Risks

* CPU deformation co the cham neu upload full mesh moi frame
* data model co the no nhanh neu weight luu qua tho
* bind pose sai -> mesh deform vo
* bounds sau deformation co the sai neu khong rebuild dung
* skinning evaluation de mo duong cho debt renderer som

Risk control:

* uu tien linear blend skinning co ban
* gioi han scope vao bind + playback deform
* chua lam weight paint trong `v0.5`
* test ky scene it joint truoc scene lon

---

## Definition of Done

`v0.5` duoc xem la done khi:

* co skin cluster data trong scene
* user bind duoc mesh vao skeleton hop le
* weights normalize an toan
* playback joint animation lam mesh deform trong viewport
* save/load giu duoc skinning data
* command/UI path co cho bind workflow
* test va QA pass

Current read:

* `Task 1` done
* `Task 2` done
* `Task 3` done
* `Task 4` done
* `Task 5` foundation done
* `Task 6` done
* `Task 7` done
* `Task 8` done

---

## Next Step After This Plan

Neu plan nay duoc duyet:

* vao `Step 3 - Implementation` cho `Skinning Foundation v0.5`

Feature tiep theo sau khi xong item nay:

* `v0.6 Animation Tools`
