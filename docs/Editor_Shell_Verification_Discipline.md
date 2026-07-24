# Editor Shell Verification Discipline

Last updated: July 24, 2026

## Goal

Khoa verify discipline cho ticket co touch `EditorShell` implementation:

* [apps/editor/include/MainWindow.h](</E:/Animation Software/apps/editor/include/MainWindow.h>)
* [apps/editor/src/MainWindow.cpp](</E:/Animation Software/apps/editor/src/MainWindow.cpp>)
* [apps/editor/include/MainWindowContexts.h](</E:/Animation Software/apps/editor/include/MainWindowContexts.h>)
* [apps/editor/src/MainWindowContexts.cpp](</E:/Animation Software/apps/editor/src/MainWindowContexts.cpp>)
* editor controller / script binding seam lien quan

## Required Automated Verification

Cho moi ticket co doi code:

1. Run build:

```powershell
powershell -ExecutionPolicy Bypass -File tools\build_debug.ps1
```

2. Run tests:

```powershell
ctest --test-dir build\ninja-msvc-debug --output-on-failure
```

Neu ticket chi doi docs, ghi ro `Documentation-only` trong log verify.

## Script Binding Regression Rule

Neu ticket touch script binding, script execution flow, hoac `ScriptCommandContext` composition:

* full `ctest` van bat buoc
* doc ket qua `editor_ui_tests`
* re-check nhom targeted script editor tests trong [tests/unit/EditorUiTests.cpp](</E:/Animation Software/tests/unit/EditorUiTests.cpp>)

Targeted test surface:

* `scriptEditorExecutesPrimitiveCommand()`
* `scriptEditorLogsChannelBoxChanges()`
* `scriptEditorExecutesTimelineCommands()`
* `scriptEditorExecutesObjectCommands()`
* `scriptEditorExecutesViewToolAndFileCommands()`
* `scriptEditorExecutesSceneFileCommands()`
* `scriptCommandRegistryDispatchesJointHierarchyCommands()`
* `scriptCommandRegistryDispatchesJointOrientationAndBindPoseCommands()`
* `scriptCommandRegistryDispatchesBindSkinCommand()`

Neu co regression trong script binding, khong close ticket du build pass.

## UI Wiring Review Rule

Neu ticket doi `QObject::connect`, viewport callback, action wiring, dock wiring, hoac lambda stored trong shell:

* re-check lambda capture
* re-check object lifetime
* re-check parent ownership cho `QObject` / `QWidget`
* khong capture ref toi temporary data qua async seam
* callback song dai hon stack frame -> chi capture object co owner ro rang

Hot spots can review:

* `MainWindow::MainWindow()`
* `create*Menu()`
* `add*ToolbarSection()`
* `create*Dock()`
* `create*Panel()`
* `MainWindowContexts` lambdas

## Ticket Log Rule

Moi ticket trong [MainWindow_Core_Extraction_Checklist.md](</E:/Animation Software/docs/MainWindow_Core_Extraction_Checklist.md>) nen ghi:

* ticket id
* note ngan ve seam da doi
* verify result

Format verify:

* `tools\build_debug.ps1`, `ctest --test-dir build\ninja-msvc-debug --output-on-failure`
* hoac `Documentation-only`
