# MainWindow Core Extraction Checklist

Last updated: July 24, 2026

## Purpose

Track the ongoing work to extract feature logic out of `MainWindow` and reduce it to a `Core/Application` shell.

Use this file as the single progress log:
- Tick a checkbox when a ticket is done.
- Add a short note in `Progress Log`.
- Record verification results after each completed ticket.

## Target Architecture

- `MainWindow` belongs to `Core/Application`
- `MainWindow` acts as:
  - application shell
  - workspace shell
  - composition root
- Feature and domain logic should live in their matching modules:
  - `Core`
  - `Engine`
  - `Scene`
  - `Rendering`
  - `Animation`
  - `Rigging`

## Status Legend

- `Todo`: not started
- `Doing`: currently in progress
- `Done`: implemented and verified
- `Blocked`: needs a decision or prerequisite

## Epic A: Shrink MainWindow To Core/Application

- [x] `A1` Split `createMenus()` into section helpers: `createFileMenu()`, `createCreateMenu()`, `createRigMenu()`, `createWindowsMenu()`, `createAnimationMenu()`
- [x] `A2` Keep `createMenus()` as high-level orchestration only
- [x] `A3` Split `createToolbar()` into section helpers: import/create, rig, view, transform, display
- [x] `A4` Split `createDocks()` into panel helpers: outliner, inspector, primitive palette, script editor, timeline
- [x] `A5` Review and remove remaining repeated plumbing patterns in `MainWindow`
- [x] `A6` Confirm `MainWindow` is reduced to composition root plus UI shell responsibilities

## Epic B: File/Core Shell Decomposition

- [x] `B1` Extract `File` menu wiring from `createMenus()`
- [x] `B2` Move `Optimize Scene Size` shell behavior behind a dedicated helper if needed
- [x] `B3` Ensure file actions only orchestrate controller/service calls
- [x] `B4` Clean up preferences and application-shell level save behavior

## Epic C: Create/Scene Shell Decomposition

- [x] `C1` Extract `Create` menu wiring from `createMenus()`
- [x] `C2` Extract primitive palette dock builder from `createDocks()`
- [x] `C3` Move primitive palette interactions behind `Scene/Creation` seams where practical
- [x] `C4` Verify joint and primitive shell flows are orchestration-only

## Epic D: Rigging Shell Decomposition

- [x] `D1` Extract `Rig` menu wiring from `createMenus()`
- [x] `D2` Extract rig-related toolbar section from `createToolbar()`
- [x] `D3` Consolidate rig action enable/disable refresh paths if practical
- [x] `D4` Verify all rig operations route through `EditorRiggingController`

## Epic E: Animation Shell Decomposition

- [x] `E1` Extract `Animation` menu wiring from `createMenus()`
- [x] `E2` Extract timeline dock builder from `createDocks()`
- [x] `E3` Move remaining timeline panel wiring into `Animation` seams where practical
- [x] `E4` Verify playback and keyframe shell actions route through animation controllers

## Epic F: View/Rendering Shell Decomposition

- [x] `F1` Keep `createViewMenu()` and `createTransformMenu()` as separate section helpers
- [x] `F2` Extract display/view toolbar section from `createToolbar()`
- [x] `F3` Reduce long inline view/display lambdas where practical
- [x] `F4` Verify camera/view/display ownership stays in `Rendering` or viewport UI seams

## Epic G: Selection/Inspector/Outliner Shell Decomposition

- [x] `G1` Extract outliner dock builder from `createDocks()`
- [x] `G2` Extract inspector/channel box dock builder from `createDocks()`
- [x] `G3` Review remaining selection refresh paths in `MainWindow`
- [x] `G4` Consolidate scene refresh and reselection orchestration helpers if useful

## Epic H: Context And Composition Cleanup

- [x] `H1` Audit all remaining `create*Context()` helpers
- [x] `H2` Move context factories to `MainWindowContexts` or `EditorContextFactory` if the count keeps growing
- [x] `H3` Reduce inline lambda composition in `MainWindow` where practical
- [x] `H4` Keep context factories owned by `Core/Application`, not by feature modules

## Epic I: Ownership And Documentation

- [x] `I1` Document `MainWindow` as part of `Core/Application`
- [x] `I2` Decide whether to rename the concept in docs to `EditorShell` or `EditorMainWindow`
- [x] `I3` Update ownership docs so `MainWindow` is no longer treated as a feature module host
- [x] `I4` Add a short architectural note that `MainWindow` is the composition root plus UI shell

## Epic J: Verification Discipline

- [x] `J1` Run `tools\build_debug.ps1` after each completed ticket
- [x] `J2` Run `ctest --test-dir build\ninja-msvc-debug --output-on-failure` after each completed ticket
- [x] `J3` Add targeted script editor checks when a ticket touches script bindings
- [x] `J4` Re-check lambda capture and object lifetime when a ticket changes UI wiring

## Recommended Execution Order

1. `A1`, `B1`, `C1`, `D1`, `E1`
2. `A3`, `F2`, `D2`
3. `A4`, `C2`, `E2`, `G1`, `G2`
4. `H1`, `H2`, `H3`
5. `A5`, `G3`, `G4`
6. `I1`, `I2`, `I3`, `I4`

## Progress Log

| Date | Ticket | Status | Notes | Verify |
|---|---|---|---|---|
| 2026-07-23 | `F1` | Done | Extracted `createViewMenu()` and `createTransformMenu()` from `createMenus()` | `tools\build_debug.ps1`, `ctest --test-dir build\ninja-msvc-debug --output-on-failure` |
| 2026-07-23 | `A1` | Done | Extracted `File`, `Create`, `Rig`, `Windows`, and `Animation` menu sections into dedicated helpers | `tools\build_debug.ps1`, `ctest --test-dir build\ninja-msvc-debug --output-on-failure` |
| 2026-07-23 | `A2` | Done | Reduced `createMenus()` to high-level section orchestration only | `tools\build_debug.ps1`, `ctest --test-dir build\ninja-msvc-debug --output-on-failure` |
| 2026-07-23 | `A3` | Done | Split toolbar assembly into import/create, rig, view, transform, and display section helpers | `tools\build_debug.ps1`, `ctest --test-dir build\ninja-msvc-debug --output-on-failure` |
| 2026-07-23 | `A4` | Done | Split dock assembly into outliner, inspector, primitive palette, script editor, and timeline dock helpers | `tools\build_debug.ps1`, `ctest --test-dir build\ninja-msvc-debug --output-on-failure` |
| 2026-07-23 | `A5` | Done | Consolidated repeated MainWindow shell plumbing with shared QAction and QDockWidget configuration helpers | `tools\build_debug.ps1`, `ctest --test-dir build\ninja-msvc-debug --output-on-failure` |
| 2026-07-23 | `A6` | Done | Audited remaining MainWindow responsibilities and confirmed they are now primarily UI shell, composition, and orchestration concerns | No code change; based on post-refactor ownership review after verified `A1`-`A5` |
| 2026-07-23 | `B1` | Done | `File` menu wiring is isolated in `createFileMenu()` | Verified during Epic A and retained after Epic B cleanup |
| 2026-07-23 | `B2` | Done | Moved `Optimize Scene Size` action behavior into `optimizeSceneStorage()` | `tools\build_debug.ps1`, `ctest --test-dir build\ninja-msvc-debug --output-on-failure` |
| 2026-07-23 | `B3` | Done | Simplified file actions to shell-level orchestration using `applyDocumentSceneLoad(...)` and `showDocumentOperationFailure(...)` | `tools\build_debug.ps1`, `ctest --test-dir build\ninja-msvc-debug --output-on-failure` |
| 2026-07-23 | `B4` | Done | Consolidated preferences persistence behind `EditorPreferencesState`, `readPreferencesState()`, and `writePreferencesState(...)` | `tools\build_debug.ps1`, `ctest --test-dir build\ninja-msvc-debug --output-on-failure` |
| 2026-07-24 | `C1` | Done | `Create` menu wiring already lives in `createCreateMenu()` | Verified during Epic A and retained through Epic C cleanup |
| 2026-07-24 | `C2` | Done | Primitive palette dock remains isolated in `createPrimitivePaletteDock()` with a dedicated `createPrimitivePalettePanel()` builder | `tools\build_debug.ps1`, `ctest --test-dir build\ninja-msvc-debug --output-on-failure` |
| 2026-07-24 | `C3` | Done | Moved primitive palette catalog into `EditorCreationController::primitivePaletteEntries()` and replaced inline palette lambdas with named shell methods | `tools\build_debug.ps1`, `ctest --test-dir build\ninja-msvc-debug --output-on-failure` |
| 2026-07-24 | `C4` | Done | Re-verified primitive and joint creation shell flows only orchestrate UI-side effects around `EditorCreationController` results | `tools\build_debug.ps1`, `ctest --test-dir build\ninja-msvc-debug --output-on-failure` |
| 2026-07-24 | `D1` | Done | `Rig` menu wiring already lives in `createRigMenu()` | Ownership review after Epic A refactor |
| 2026-07-24 | `D2` | Done | Rig toolbar actions already live in `addRigToolbarSection()` | Ownership review after Epic A refactor |
| 2026-07-24 | `D3` | Done | Rig action enable/disable refresh is already consolidated in `EditorInspectorController::updateSelectionActions(...)` | Ownership review against current code paths |
| 2026-07-24 | `D4` | Done | Mark parent, reparent, bind skin, joint orientation, and bind pose flows route through `EditorRiggingController` | Ownership review against current code paths |
| 2026-07-24 | `E1` | Done | `Animation` menu wiring already lives in `createAnimationMenu()` | Ownership review against current code paths |
| 2026-07-24 | `E2` | Done | Timeline dock builder already lives in `createTimelineDock()` | Ownership review against current code paths |
| 2026-07-24 | `E3` | Done | Timeline panel wiring is already concentrated in `createTimeSliderPanel()` and `AnimationTimelinePanel` owns widget-level timeline behavior | Ownership review against current code paths |
| 2026-07-24 | `E4` | Done | Playback and keyframe shell actions route through `EditorAnimationFlowController` and `EditorAnimationController` | Ownership review against current code paths |
| 2026-07-24 | `F2` | Done | View and display toolbar sections already live in `addViewToolbarSection()` and `addDisplayToolbarSection()` | Ownership review after Epic A refactor |
| 2026-07-24 | `F3` | Done | Replaced long inline view/display lambdas with named shell methods for camera reset, frame scene, and display toggles | `tools\build_debug.ps1`, `ctest --test-dir build\ninja-msvc-debug --output-on-failure` |
| 2026-07-24 | `F4` | Done | Camera/view/display shell ownership remains in `MainWindow` as shell wiring, while rendering/viewport behavior routes through viewport UI seams | Ownership review against current code paths |
| 2026-07-24 | `G1` | Done | Outliner dock builder already lives in `createOutlinerDock()` with panel composition in `createOutlinerPanel()` | Ownership review after Epic A refactor |
| 2026-07-24 | `G2` | Done | Inspector/channel box dock builder already lives in `createInspectorDock()` with panel composition in `createInspectorPanel()` | Ownership review after Epic A refactor |
| 2026-07-24 | `G3` | Done | Reviewed remaining selection refresh paths and concentrated them around `refreshScenePanels()`, `restoreSelectionAfterSceneRefresh(...)`, and selection shell methods | `tools\build_debug.ps1`, `ctest --test-dir build\ninja-msvc-debug --output-on-failure` |
| 2026-07-24 | `G4` | Done | Added shared reselection orchestration helpers and reused them across history, animation, rigging, and script-driven scene mutation flows | `tools\build_debug.ps1`, `ctest --test-dir build\ninja-msvc-debug --output-on-failure` |
| 2026-07-24 | `H1` | Done | Audited remaining shell context helpers and identified the shallow adapter cluster around selection, channel box, rigging, outliner, viewport UI, and animation flow composition | Ownership review before extraction |
| 2026-07-24 | `H2` | Done | Moved context factory implementation into `MainWindowContexts` so `MainWindow` no longer owns the low-level adapter assembly details | `tools\build_debug.ps1`, `ctest --test-dir build\ninja-msvc-debug --output-on-failure` |
| 2026-07-24 | `H3` | Done | Reduced inline lambda composition in `MainWindow.cpp` by delegating context assembly to `MainWindowContexts` and keeping only thin shell wrappers | `tools\build_debug.ps1`, `ctest --test-dir build\ninja-msvc-debug --output-on-failure` |
| 2026-07-24 | `H4` | Done | Kept context factory ownership in `Core/Application` by introducing `apps/editor/.../MainWindowContexts.*` instead of pushing adapters into feature modules | `tools\build_debug.ps1`, `ctest --test-dir build\ninja-msvc-debug --output-on-failure` |
| 2026-07-24 | `I1` | Done | Documented `MainWindow` ownership under `Core/Application` in the new editor shell ownership note and aligned existing shell docs | Documentation-only ownership review |
| 2026-07-24 | `I2` | Done | Chose `EditorShell` as the architectural concept name, while keeping `MainWindow` as the concrete Qt class name | Documentation-only naming decision |
| 2026-07-24 | `I3` | Done | Updated ownership-facing docs so `MainWindow` is described as an editor shell adapter rather than a feature module host | Documentation-only ownership review |
| 2026-07-24 | `I4` | Done | Added a short architectural note clarifying that `MainWindow` is the composition root plus UI shell | Documentation-only architectural note |
| 2026-07-24 | `J1` | Done | Formalized build verification rule for code-touching editor shell tickets in `Editor_Shell_Verification_Discipline.md` | `tools\build_debug.ps1`, `ctest --test-dir build\ninja-msvc-debug --output-on-failure` |
| 2026-07-24 | `J2` | Done | Formalized test verification rule for code-touching editor shell tickets in `Editor_Shell_Verification_Discipline.md` | `tools\build_debug.ps1`, `ctest --test-dir build\ninja-msvc-debug --output-on-failure` |
| 2026-07-24 | `J3` | Done | Recorded targeted script editor regression surface around `EditorUiTests` and script command registry dispatch tests | `tools\build_debug.ps1`, `ctest --test-dir build\ninja-msvc-debug --output-on-failure` |
| 2026-07-24 | `J4` | Done | Recorded lambda capture and object lifetime review rule for UI wiring changes in editor shell seams | `tools\build_debug.ps1`, `ctest --test-dir build\ninja-msvc-debug --output-on-failure` |

## Current Snapshot

- `MainWindow` has already been reduced significantly in:
  - script binding composition
  - result application plumbing
  - view/transform menu shell decomposition
  - full menu section decomposition
  - toolbar section decomposition
  - dock section decomposition
  - shared action and dock shell configuration helpers
- `MainWindow` now mostly owns:
  - shell-level menu, toolbar, dock, and panel construction
  - Qt signal hookup
  - context and dependency composition
  - controller orchestration
  - result application and shell-level status/log feedback
- `EditorShell` ownership is now documented explicitly:
  - `EditorShell` la ten `module` kien truc trong docs
  - `MainWindow` la adapter `Qt Widgets` hien tai cua `EditorShell`
  - `MainWindow` va `MainWindowContexts` thuoc `Core/Application`
  - `MainWindow` khong duoc xem la feature module host
- Verification discipline is now documented explicitly:
  - build + test la bat buoc cho ticket co doi code trong editor shell
  - ticket touch script binding phai re-check targeted script editor test surface
  - ticket touch UI wiring phai re-check lambda capture va object lifetime
- `File/Core` shell flow is now cleaner:
  - file menu wiring lives in `createFileMenu()`
  - optimize-scene behavior uses a named shell helper
  - document load and failure handling use dedicated shell helpers
  - preferences persistence uses a dedicated shell state object and read/write helpers
- `Create/Scene` shell flow is now cleaner:
  - create menu wiring lives in `createCreateMenu()`
  - primitive palette dock uses a dedicated panel builder
  - primitive palette catalog lives in `EditorCreationController`
  - primitive palette interactions use named shell methods instead of inline lambdas
- `Rigging` shell flow is now cleaner:
  - rig menu wiring lives in `createRigMenu()`
  - rig toolbar wiring lives in `addRigToolbarSection()`
  - rig action enablement is centralized in `EditorInspectorController::updateSelectionActions(...)`
  - rig mutations route through `EditorRiggingController`
- `Animation` shell flow is now cleaner:
  - animation menu wiring lives in `createAnimationMenu()`
  - timeline dock wiring lives in `createTimelineDock()`
  - timeline panel composition lives in `createTimeSliderPanel()`
  - widget-level timeline behavior lives in `AnimationTimelinePanel`
  - playback and keyframe flows route through animation controllers
- `View/Rendering` shell flow is now cleaner:
  - view menu wiring lives in `createViewMenu()`
  - transform menu wiring lives in `createTransformMenu()`
  - view and display toolbar sections live in dedicated toolbar helpers
  - long view/display lambdas have been replaced by named shell methods
  - rendering behavior still routes through viewport UI seams and viewport widget methods
- Remaining work is no longer about removing large domain logic from `MainWindow`; it is mostly follow-up refinement under later epics:
  - no open extraction epic in checklist
