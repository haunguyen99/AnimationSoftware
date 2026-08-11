# UI Redesign Implementation Plan

## Locked Decisions

* Timing: after v0.6 Phase 1 QA ✓ (2026-08-06)
* Scope: all 4 steps in one cohesive effort
* Engine: keep QDockWidget + QMainWindow; WorkspaceManager is a management layer on top
* Rig Panel: separate panel, extracted from Channel Box, appears in Rigging preset

---

## Step 1 — WorkspaceManager + Panel Extraction (IN PROGRESS)

**Goal:** EditorShell stops owning dock widgets. WorkspaceManager owns them.

**New files:**
* `src/core/app/WorkspaceManager.h`
* `src/core/app/WorkspaceManager.cpp`

**What moves out of EditorShell:**
* 8 `QDockWidget*` member vars → WorkspaceManager owns them
* `createDocks()` + all `create*Dock()` methods → WorkspaceManager
* `bindCompactDockTitleBehavior()` and dock title helpers → WorkspaceManager (static helpers)
* `restoreDefaultWorkspaceLayout()` body → `WorkspaceManager::applyDefaultPreset()`
* `restoreBottomPanelLayout()` body → `WorkspaceManager::restoreBottomPanelLayout()`
* `showPolygonPrimitivesWindow()` body → delegates to WorkspaceManager

**What stays in EditorShell:**
* Panel content widget creation (`createOutlinerPanel()`, `createInspectorPanel()`, etc.)
* `installPanelActivationTracking()` — needs EditorShell members (activePanel_, status bar)
* `WorkspaceManager* workspaceManager_` member
* Thin delegating stubs for `restoreDefaultWorkspaceLayout()`, `restoreBottomPanelLayout()`
* Dock member vars kept as cache pointers assigned from WorkspaceManager post-registration

**Strategy:** Low-friction: EditorShell keeps its `outlinerDock_` etc. member vars but they're
assigned from `workspaceManager_->dock("Outliner")` after panel registration instead of being
created directly. All use sites stay unchanged.

**Panel IDs:**
| ID | Dock Title | Area | Compact Title |
|---|---|---|---|
| `"Outliner"` | Outliner | Left | No |
| `"ChannelBox"` | Channel Box | Right | No |
| `"PrimitivePalette"` | Polygon Primitives | Right (floating) | No |
| `"ScriptEditor"` | Script Editor | Bottom (floating) | No |
| `"Timeline"` | Time Slider | Bottom | Yes |
| `"RangeSlider"` | Range Slider | Bottom | Yes |
| `"CommandLine"` | Command Line | Bottom | Yes |
| `"GraphEditor"` | Graph Editor | Bottom (tabbed, hidden) | No |

**Status:** in progress

---

## Step 2 — Layout Persistence

**Goal:** Save/restore dock layout across sessions via QSettings.

**What:**
* `WorkspaceManager::saveSessionLayout()` — writes `QMainWindow::saveState()` to QSettings
* `WorkspaceManager::restoreSessionLayout()` — reads and restores; falls back to default
* Called in `EditorShell::savePreferences()` and `EditorShell::loadPreferences()`

**Status:** not started

---

## Step 3 — Built-in Presets

**Goal:** Window → Workspace menu to switch between Default / Animation / Rigging layouts.

**What:**
* `WorkspaceManager::applyPreset(const QString& name)`
* Presets: Default, Animation (larger bottom area), Rigging (Rig Panel visible)
* Preset switch resets dock arrangement; user moves are preserved until next switch
* Active preset remembered in QSettings

**Status:** not started

---

## Step 4 — User-Defined Layouts

**Goal:** User can save, name, and recall custom dock layouts.

**What:**
* `WorkspaceManager::saveUserLayout(const QString& name)`
* `WorkspaceManager::restoreUserLayout(const QString& name)`
* `WorkspaceManager::deleteUserLayout(const QString& name)`
* `WorkspaceManager::userLayoutNames() const`
* UI: Window → Workspace → Save Layout... / manage dialogs
* Stored in QSettings under `workspace/userLayouts/<name>`

**Status:** not started

---

## Step 5 — Rig Panel

**Goal:** Separate RigPanel extracted from Channel Box / Inspector.

**What:**
* New `RigPanel` widget (joint orient, bind pose, skin binding weights)
* Registered as panel ID `"RigPanel"` in WorkspaceManager
* Visible by default in Rigging preset, hidden in Default/Animation presets
* Channel Box retains transform/attribute editing only

**Status:** not started

---

## Definition of Done

* EditorShell.cpp under 2000 lines (was 2961)
* Layout persists across sessions
* Window → Workspace menu with 3 presets works
* User-defined layout save/restore works
* Rig Panel separate and visible in Rigging workspace
* `ctest` still 2/2 ✓
