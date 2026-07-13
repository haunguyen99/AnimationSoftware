# Project Phoenix Context

Project Phoenix la editor huong `rigging + animation` cho game animator. Tai lieu nay khoa ngon ngu domain de code, docs, va test khong goi cung mot khai niem bang nhieu ten khac nhau.

## Language

**Joint**:
Node rig co pivot, orientation rieng, va co the nam trong `skeleton hierarchy`.
_Avoid_: Bone node, socket node

**Skeleton Hierarchy**:
Cay `joint` parent-child hop le dung de mo ta cau truc rig.
_Avoid_: Bone tree, rig tree

**Joint Orientation**:
Rotation co cau cua `joint`, duoc luu tach rieng khoi `local transform rotation`.
_Avoid_: Animated rotation, local rotate

**Animated Rotation**:
Thanh phan rotation trong `local transform` duoc playback va keyframe danh gia theo thoi gian.
_Avoid_: Joint orientation

**Bind Pose**:
Moc `local transform` cua `joint` duoc chup lai de lam nen cho skinning ve sau.
_Avoid_: Rest cache, viewport cache

## Relationships

- Mot **Skeleton Hierarchy** gom mot hoac nhieu **Joint**
- Moi **Joint** co toi da mot parent **Joint** trong **Skeleton Hierarchy**
- Mot **Joint** co mot **Joint Orientation** va co the co mot **Bind Pose**
- **Animated Rotation** duoc ap len tren **Joint Orientation**, khong thay the no

## Example dialogue

> **Dev:** "Khi user xoay joint trong Channel Box, day la Joint Orientation hay Animated Rotation?"
> **Domain expert:** "Neu dang chinh truong joint orient thi do la Joint Orientation; keyframe van di vao Animated Rotation."

## Flagged ambiguities

- "bone" va "joint" da duoc gop thanh **Joint** trong repo hien tai.
- "rotation" tung bi dung chung cho **Joint Orientation** va **Animated Rotation**; resolve: hai khai niem nay tach rieng.
