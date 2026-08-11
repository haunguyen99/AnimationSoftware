# Phoenix Editor — Feature Sheet

> Cập nhật: 2026-08-10 · Branch: `codex/core-scene-step1-seams`
> Cột **Status**: `✅ Done` · `🚧 WIP` · `📋 Planned` · `❌ Missing`

---

## 1. Scene Graph

| # | Tính năng | Class / Namespace | Status | Ghi chú |
|---|-----------|-------------------|--------|---------|
| 1.1 | SceneObject (Transform / Joint) | `SceneObject` | ✅ Done | Kind enum: Transform, Joint |
| 1.2 | Scene container (flat QHash) | `Scene` | ✅ Done | `QHash<Id, SceneObject>` |
| 1.3 | Hierarchy (parent/child) | `Scene` | ✅ Done | parentId + childIds, flat |
| 1.4 | MeshData (vertex + index buffer) | `MeshData` | ✅ Done | stored by mesh handle (int) |
| 1.5 | Transform struct (T/R/S) | `Transform` | ✅ Done | QVector3D + QQuaternion |
| 1.6 | Bounds3D (AABB) | `Bounds3D` | ✅ Done | per-object local + world |
| 1.7 | World transform cache | `Scene::rebuildWorldData` | ✅ Done | rebuilt on demand |
| 1.8 | createObject / createJoint | `Scene` | ✅ Done | auto-increment uint64_t ID |
| 1.9 | reparentObject | `Scene` | ✅ Done | keepWorldTransform option |
| 1.10 | duplicateSubtree | `Scene` | ✅ Done | recursive, new IDs |
| 1.11 | removeObject | `Scene` | ✅ Done | recursive remove |
| 1.12 | optimizeStorage | `Scene` | ✅ Done | compact mesh data |
| 1.13 | debugDump | `Scene` | ✅ Done | string dump toàn scene |
| 1.14 | appendScene (merge) | `Scene` | ✅ Done | dùng khi import |

---

## 2. Animation

| # | Tính năng | Class / Namespace | Status | Ghi chú |
|---|-----------|-------------------|--------|---------|
| 2.1 | TransformKeyframeTrack | `TransformKeyframeTrack` | ✅ Done | map: frame → Transform |
| 2.2 | ObjectAnimationState | `ObjectAnimationState` | ✅ Done | per-object, stored in SceneObject |
| 2.3 | SceneAnimationState | `SceneAnimationState` | ✅ Done | global trong Scene |
| 2.4 | Set keyframe | `Scene::setObjectKeyframe` | ✅ Done | |
| 2.5 | Delete keyframe | `Scene::removeObjectKeyframe` | ✅ Done | |
| 2.6 | Delete keyframes in range | `Scene::removeObjectKeyframesInRange` | ✅ Done | |
| 2.7 | Duplicate keyframe | `Scene::duplicateObjectKeyframe` | ✅ Done | |
| 2.8 | Shift keyframes ±n | `Scene::offsetObjectKeyframes` | ✅ Done | |
| 2.9 | Shift keyframes in range | `Scene::offsetObjectKeyframesInRange` | ✅ Done | |
| 2.10 | Scale all keyframes | `Scene::scaleAllObjectKeyframes` | ✅ Done | |
| 2.11 | Next / Prev keyframe query | `Scene::nextObjectKeyframe` | ✅ Done | |
| 2.12 | Auto Key mode | `EditorAnimationFlowController` | ✅ Done | flag trong EditorAnimationState |
| 2.13 | Playback (timer tick) | `EditorPlaybackController` | ✅ Done | QTimer-based loop |
| 2.14 | Toggle playback | `EditorAnimationFlowController` | ✅ Done | |
| 2.15 | Step frame ±1 | `EditorAnimationFlowController` | ✅ Done | |
| 2.16 | Playback range (start/end frame) | `EditorAnimationState` | ✅ Done | |
| 2.17 | Visible frame range | `EditorAnimationState` | ✅ Done | tách khỏi playback range |
| 2.18 | FPS setting | `EditorAnimationState` | ✅ Done | default 24 |
| 2.19 | Frame range drag-select | `KeyframeTimelineWidget` | ✅ Done | drag trên timeline |
| 2.20 | Jump to keyframe (prev/next) | `EditorAnimationFlowController` | ✅ Done | |
| 2.21 | Curve editor (Graph Editor) | `GraphEditorPanel` | 🚧 WIP | UI shell có, curve edit chưa rõ |
| 2.22 | Easing / interpolation curves | — | 📋 Planned | chưa có trong code |

---

## 3. Rigging

| # | Tính năng | Class / Namespace | Status | Ghi chú |
|---|-----------|-------------------|--------|---------|
| 3.1 | Create Joint | `SceneRiggingController::createJoint` | ✅ Done | Kind::Joint SceneObject |
| 3.2 | Joint hierarchy (parent/unparent) | `EditorRiggingController` | ✅ Done | P / Shift+P shortcuts |
| 3.3 | Mark hierarchy parent | `EditorShell` | ✅ Done | markedHierarchyParentId_ |
| 3.4 | Joint orientation (Euler) | `SceneRiggingController::setJointOrientation` | ✅ Done | QQuaternion |
| 3.5 | Reset joint orientation | `SceneRiggingController::resetJointOrientation` | ✅ Done | |
| 3.6 | Align orient to child | `SceneRiggingController::alignJointOrientationToChild` | ✅ Done | |
| 3.7 | Capture Bind Pose | `SceneRiggingController::captureBindPose` | ✅ Done | flag + bindPoseLocalTransform |
| 3.8 | Capture Bind Pose (recursive) | `SceneRiggingController::captureBindPose` | ✅ Done | recursive=true |
| 3.9 | Skin binding (mesh → skeleton) | `SceneRiggingController::bindObjectToSkeleton` | ✅ Done | auto-weight |
| 3.10 | Manual skin weights | `Scene::setObjectSkinBinding` | ✅ Done | SkinWeightTable |
| 3.11 | Clear skin binding | `SceneRiggingController::clearObjectSkinBinding` | ✅ Done | |
| 3.12 | Linear Blend Skinning | `SkinDeformationOps` | ✅ Done | buildDeformedMesh |
| 3.13 | Bind Pose world transform | `SceneRiggingController::bindPoseWorldTransform` | ✅ Done | |
| 3.14 | Weight painting UI | — | ❌ Missing | chưa có |
| 3.15 | IK solver | — | ❌ Missing | chưa có |

---

## 4. Viewport & Rendering

| # | Tính năng | Class / Namespace | Status | Ghi chú |
|---|-----------|-------------------|--------|---------|
| 4.1 | OpenGL 3.3 Core renderer | `ViewportRenderer` | ✅ Done | |
| 4.2 | Orbit camera (yaw/pitch/distance) | `EditorCamera` | ✅ Done | |
| 4.3 | Perspective projection | `EditorCamera` | ✅ Done | FOV 45° default |
| 4.4 | Orthographic projection | `EditorCamera` | ✅ Done | |
| 4.5 | Camera presets (7 views) | `EditorCamera::ViewPreset` | ✅ Done | Persp/Front/Back/Left/Right/Top/Bottom |
| 4.6 | Frame selected (F) | `EditorCamera::frameBounds` | ✅ Done | |
| 4.7 | Frame entire scene | `EditorCamera::frameScene` | ✅ Done | |
| 4.8 | Quad view (Space toggle) | `ViewportWorkspaceWidget` | ✅ Done | 4 viewport panels |
| 4.9 | Wireframe mode | `ViewportRenderOptions` | ✅ Done | |
| 4.10 | Backface culling | `ViewportRenderOptions` | ✅ Done | |
| 4.11 | World axis display | `ViewportRenderOptions` | ✅ Done | |
| 4.12 | Selection outline | `ViewportRenderOptions` | ✅ Done | selection bounds AABB |
| 4.13 | Preview mesh (pre-commit import) | `ViewportRenderer::setPreviewScene` | ✅ Done | |
| 4.14 | Grid display | `ViewportRenderer` | ✅ Done | gridVertices_ |
| 4.15 | Joint visualization | `ViewportRenderer` | ✅ Done | jointVao_ |
| 4.16 | Zoom sensitivity setting | `EditorCamera` | ✅ Done | |
| 4.17 | Pan | `EditorCamera::pan` | ✅ Done | |
| 4.18 | Zoom (scroll) | `EditorCamera::zoom` | ✅ Done | |
| 4.19 | Multi-viewport sync | `ViewportRenderSync` | ✅ Done | |
| 4.20 | PBR shading | — | ❌ Missing | shader hiện tại rất basic |
| 4.21 | Texture display | — | ❌ Missing | chưa có |
| 4.22 | Shadow | — | ❌ Missing | chưa có |

---

## 5. Gizmos

| # | Tính năng | Class / Namespace | Status | Ghi chú |
|---|-----------|-------------------|--------|---------|
| 5.1 | Translate Gizmo | `TranslateGizmoGeometry` + `TranslateGizmoInteraction` + `TranslateGizmoDrag` | ✅ Done | |
| 5.2 | Rotate Gizmo | `RotateGizmoGeometry` + `RotateGizmoInteraction` + `RotateGizmoDrag` | ✅ Done | |
| 5.3 | Scale Gizmo | `ScaleGizmoGeometry` + `ScaleGizmoInteraction` + `ScaleGizmoDrag` | ✅ Done | |
| 5.4 | World orientation mode | `EditorShell::AxisUiOrientation::World` | ✅ Done | |
| 5.5 | Local orientation mode | `EditorShell::AxisUiOrientation::Local` | ✅ Done | |
| 5.6 | Active axis highlight | `ViewportRenderer::setGizmo` | ✅ Done | activeAxis param |
| 5.7 | Snapping (grid / angle) | — | ❌ Missing | chưa có |
| 5.8 | Multi-object transform | — | ❌ Missing | chỉ single selection |

---

## 6. I/O

| # | Tính năng | Class / Namespace | Status | Ghi chú |
|---|-----------|-------------------|--------|---------|
| 6.1 | Native format (.phoenixscene) save | `PhoenixSceneDocument::saveToFile` | ✅ Done | |
| 6.2 | Native format (.phoenixscene) load | `PhoenixSceneDocument::loadFromFile` | ✅ Done | |
| 6.3 | FBX import | `FbxImporter` → assimp | ✅ Done | |
| 6.4 | FBX scene builder | `FbxSceneBuilder` | ✅ Done | |
| 6.5 | FBX import validation | `FbxImportValidation` | ✅ Done | |
| 6.6 | FBX import messages | `FbxImportMessages` | ✅ Done | user-facing error msgs |
| 6.7 | Assimp generic load | `AssimpSceneDocument` | ✅ Done | |
| 6.8 | Increment & Save | `EditorShell::incrementAndSave` | ✅ Done | auto version suffix |
| 6.9 | Archive Scene | `EditorShell::archiveScene` | ✅ Done | |
| 6.10 | Export All | `EditorShell::exportAll` | ✅ Done | |
| 6.11 | Export Selection | `EditorShell::exportSelection` | ✅ Done | |
| 6.12 | FBX export | — | ❌ Missing | chỉ import, không export |
| 6.13 | USD / glTF | — | ❌ Missing | |
| 6.14 | OBJ import | — | 📋 Planned | assimp hỗ trợ, chưa wire |

---

## 7. Script Command System (MEL-like)

| # | Lệnh | Mô tả | Status |
|---|------|-------|--------|
| 7.1 | `select -cl` | Clear selection | ✅ Done |
| 7.2 | `select <name>` | Select by name | ✅ Done |
| 7.3 | `polyCube -w -h -d` | Create cube primitive | ✅ Done |
| 7.4 | `polySphere` / `polyCylinder` etc. | Other primitives | ✅ Done |
| 7.5 | `rename <old> <new>` | Rename object | ✅ Done |
| 7.6 | `duplicate <name>` | Duplicate object | ✅ Done |
| 7.7 | `group <name>` | Group object | ✅ Done |
| 7.8 | `delete <name>` | Delete object | ✅ Done |
| 7.9 | `parent <child> <parent>` | Parent object | ✅ Done |
| 7.10 | `unparent <child>` | Unparent object | ✅ Done |
| 7.11 | `setAttr "obj.attr" vals` | Set attribute (translate/rotate/scale) | ✅ Done |
| 7.12 | `currentTime <frame>` | Jump to frame | ✅ Done |
| 7.13 | `playbackOptions -min -max` | Set playback range | ✅ Done |
| 7.14 | `play -state on\|off` | Start/stop playback | ✅ Done |
| 7.15 | `autoKeyframe -state on\|off` | Toggle auto key | ✅ Done |
| 7.16 | `setKeyframe <obj> -t <frame>` | Set keyframe | ✅ Done |
| 7.17 | `cutKey <obj> -t <frame>` | Delete keyframe | ✅ Done |
| 7.18 | `copyKey <obj> -t <src> -to <dst>` | Copy keyframe | ✅ Done |
| 7.19 | `shiftKey <obj> -by <n>` | Shift all keyframes | ✅ Done |
| 7.20 | `joint -name <name>` | Create joint | ✅ Done |
| 7.21 | `jointOrient <obj> -euler x y z` | Set joint orientation | ✅ Done |
| 7.22 | `jointOrient -reset <obj>` | Reset joint orientation | ✅ Done |
| 7.23 | `jointOrient <obj> -alignToChild` | Align orientation to child | ✅ Done |
| 7.24 | `bindPose -capture [-recursive] <obj>` | Capture bind pose | ✅ Done |
| 7.25 | `bindSkin <mesh> <joint>` | Bind skin | ✅ Done |
| 7.26 | `setToolTo RotateSuperContext` etc. | Activate tool | ✅ Done |
| 7.27 | `viewFit <name>` | Frame object in viewport | ✅ Done |
| 7.28 | `viewSet -home` | Reset camera | ✅ Done |
| 7.29 | `file -f -new` | New scene | ✅ Done |
| 7.30 | `file -o "<path>"` | Open scene | ✅ Done |
| 7.31 | `file -save "<path>"` | Save scene | ✅ Done |
| 7.32 | `file -import "<path>"` | Import file | ✅ Done |

---

## 8. Editor Shell & UI

| # | Tính năng | Widget / Class | Status | Ghi chú |
|---|-----------|----------------|--------|---------|
| 8.1 | Outliner (scene tree) | `QTreeWidget` | ✅ Done | |
| 8.2 | Drag-to-reparent outliner | `EditorShell` event filter | ✅ Done | left mouse drag |
| 8.3 | Channel Box (transform spinboxes) | `QDoubleSpinBox` x9 | ✅ Done | T/R/S per axis |
| 8.4 | Visibility toggle | `QCheckBox` | ✅ Done | |
| 8.5 | Inspector panel (joint tools) | `EditorInspectorController` | ✅ Done | joint orient + bind pose UI |
| 8.6 | Primitive Palette | `QListWidget` | ✅ Done | cube/sphere/cylinder... |
| 8.7 | Script Editor | `QPlainTextEdit` x2 | ✅ Done | history + input |
| 8.8 | Script execute all / selection | `EditorScriptExecutionController` | ✅ Done | |
| 8.9 | WorkspaceManager (docks) | `WorkspaceManager` | ✅ Done | bọc QDockWidget |
| 8.10 | Preset: Default | `WorkspaceManager::applyDefaultPreset` | ✅ Done | |
| 8.11 | Preset: Animation | `WorkspaceManager::applyAnimationLayout` | ✅ Done | |
| 8.12 | Preset: Rigging | `WorkspaceManager::applyRiggingLayout` | ✅ Done | |
| 8.13 | User layout save/restore/delete | `WorkspaceManager` | 🚧 WIP | placeholder, chưa full implement |
| 8.14 | Undo / Redo | `EditorHistoryController` | ✅ Done | clone Scene snapshots |
| 8.15 | Undo/Redo unlimited | — | 📋 Planned | hiện có giới hạn stack? |
| 8.16 | Command Line panel | `QLabel` status | ✅ Done | display only |
| 8.17 | Status bar messages | `EditorShell::showStatusMessageIfPresent` | ✅ Done | với timeout |
| 8.18 | Preferences save/load | `EditorPreferences` | ✅ Done | QSettings |
| 8.19 | Window title update | `EditorShell::updateWindowTitle` | ✅ Done | show file path |
| 8.20 | Active panel tracking | `EditorShell::ActivePanel` | ✅ Done | status label |
| 8.21 | Graph Editor | `GraphEditorPanel` | 🚧 WIP | UI shell có, editing chưa đầy đủ |
| 8.22 | Rig Panel dock | `WorkspaceManager::kRigPanel` | 🚧 WIP | panel có, content? |
| 8.23 | Maya-like shortcuts (P / Shift+P) | `EditorShell` | ✅ Done | parent/unparent |

---

## 9. Menus & Toolbar

| # | Menu / Section | Actions | Status |
|---|----------------|---------|--------|
| 9.1 | File menu | New / Open / Import FBX / Save / Save As / Increment & Save / Archive / Export / Optimize / Preferences | ✅ Done |
| 9.2 | Edit menu | Undo / Redo | ✅ Done |
| 9.3 | Create menu | Primitives / Joint | ✅ Done |
| 9.4 | Rig menu | Mark Parent / Parent to Marked / Unparent / Bind Skin / Orient / Bind Pose | ✅ Done |
| 9.5 | Animation menu | Set Key / Delete Key / Dup Key / Shift Keys / Prev/Next Key | ✅ Done |
| 9.6 | View menu | Camera presets / Reset / Frame / Wireframe / Axis / Backface | ✅ Done |
| 9.7 | Windows menu | Script Editor / Graph Editor / Workspace presets / Save layout | ✅ Done |
| 9.8 | Transform menu | Translate / Rotate / Scale / World / Local | ✅ Done |
| 9.9 | Toolbar | Import+Create / Rig / View / Transform / Display sections | ✅ Done |

---

## 10. Tests

| # | Test | Scope | Status |
|---|------|-------|--------|
| 10.1 | `createCubeUpdatesOutlinerAndChannelBox` | Integration | ✅ Done |
| 10.2 | `restoreDefaultLayoutResetsFloatingDocks` | Integration | ✅ Done |
| 10.3 | `transformToolbarUpdatesViewportToolState` | Integration | ✅ Done |
| 10.4 | `viewMenuSwitchesViewportCameraPresets` | Integration | ✅ Done |
| 10.5 | `spaceTogglesQuadViewAndMaximizesActiveViewport` | Integration | ✅ Done |
| 10.6 | `editMenuUndoRedoRestoresCreatedPrimitive` | Integration | ✅ Done |
| 10.7 | `scriptEditorExecutesPrimitiveCommand` | Integration | ✅ Done |
| 10.8 | `scriptEditorLogsChannelBoxChanges` | Unit | ✅ Done |
| 10.9 | `scriptEditorExecutesTimelineCommands` | Unit | ✅ Done |
| 10.10 | `scriptEditorExecutesObjectCommands` | Unit | ✅ Done |
| 10.11 | `scriptEditorExecutesViewToolAndFileCommands` | Unit | ✅ Done |
| 10.12 | `scriptEditorExecutesSceneFileCommands` | Unit | ✅ Done |
| 10.13 | `scriptCommandRegistryDispatchesFileImportCommand` | Unit | ✅ Done |
| 10.14 | `scriptCommandRegistryDispatchesSetKeyframeCommand` | Unit | ✅ Done |
| 10.15 | `scriptCommandRegistryDispatchesDeleteKeyCommand` | Unit | ✅ Done |
| 10.16 | `scriptCommandRegistryDispatchesCopyAndShiftKeyCommands` | Unit | ✅ Done |
| 10.17 | `scriptCommandRegistryDispatchesAutoKeyCommand` | Unit | ✅ Done |
| 10.18 | `scriptCommandRegistryDispatchesJointHierarchyCommands` | Unit | ✅ Done |
| 10.19 | `scriptCommandRegistryDispatchesJointOrientationAndBindPoseCommands` | Unit | ✅ Done |
| 10.20 | `scriptCommandRegistryDispatchesBindSkinCommand` | Unit | ✅ Done |
| 10.21 | `hierarchyActionsParentAndUnparentJoints` | Integration | ✅ Done |
| 10.22 | `bindSkinActionBindsMeshToMarkedJoint` | Integration | ✅ Done |
| 10.23 | `hierarchyActionsExposeMayaLikeShortcuts` | Unit | ✅ Done |
| 10.24 | `leftMouseDragReparentsOutlinerItems` | Integration | ✅ Done |
| 10.25 | `jointInspectorEditsOrientationAndCapturesBindPose` | Integration | ✅ Done |
| 10.26 | `timelineUiShowsKeyframeFeedback` | Integration | ✅ Done |
| 10.27 | `timelineUiSupportsFrameRangeSelection` | Integration | ✅ Done |
| 10.28 | `playbackControllerRoutesTimeIntentsAndTicks` | Unit | ✅ Done |
| 10.29 | `animationEngineFacadeAppliesKeyEditsAndScriptBindings` | Unit | ✅ Done |
| 10.30 | `sceneRuntimeControllerAppliesSceneFrameAndSelection` | Unit | ✅ Done |
| 10.31 | `viewportSceneControllerRoutesSceneMutations` | Unit | ✅ Done |
| 10.32 | SceneTransformTests | Unit (scene math) | ✅ Done |

---

## 11. Tính năng còn thiếu (Gap Summary)

| # | Tính năng | Priority | Ghi chú |
|---|-----------|----------|---------|
| G1 | PBR / texture shading | Cao | shader hiện tại unlit |
| G2 | FBX export | Cao | chỉ có import |
| G3 | Weight painting UI | Cao | rigging cần |
| G4 | IK solver | Trung | FK only hiện tại |
| G5 | Easing curves (Graph Editor) | Trung | UI panel có nhưng chưa functional |
| G6 | Multi-object selection & transform | Trung | single-select only |
| G7 | Snapping (grid / angle) | Trung | gizmo không có snap |
| G8 | USD / glTF I/O | Thấp | |
| G9 | OBJ import (qua assimp) | Thấp | assimp đã hỗ trợ, chưa wire |
| G10 | User layout (WorkspaceManager) | Thấp | placeholder chưa implement đầy đủ |
| G11 | Unlimited undo stack | Thấp | |
| G12 | Rig Panel content | ? | dock có nhưng nội dung? |

---

*Chỉnh sửa trực tiếp file này — mỗi dòng là 1 feature entry.*
