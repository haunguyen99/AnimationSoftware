#include "EditorMenuBar.h"

#include <QInputDialog>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenuBar>
#include <QStatusBar>

#include "core/app/WorkspaceManager.h"

namespace
{

QAction* configureAction(
    QAction* action,
    const char* objectName = nullptr,
    bool enabled = true,
    bool checkable = false,
    const QKeySequence& shortcut = QKeySequence())
{
    if (action == nullptr) {
        return nullptr;
    }
    if (objectName != nullptr) {
        action->setObjectName(QString::fromUtf8(objectName));
    }
    action->setEnabled(enabled);
    action->setCheckable(checkable);
    if (!shortcut.isEmpty()) {
        action->setShortcut(shortcut);
    }
    return action;
}

} // namespace

EditorMenuBar::EditorMenuBar(QMainWindow* mainWindow, QObject* parent)
    : QObject(parent)
    , mainWindow_(mainWindow)
{
}

void EditorMenuBar::build(const Bindings& bindings)
{
    bindings_ = bindings;
    buildEditMenu(bindings);
    buildFileMenu(bindings);
    buildCreateMenu(bindings);
    buildRigMenu(bindings);
    buildWindowsMenu(bindings);
    buildAnimationMenu(bindings);
    buildViewMenu(bindings);
    buildTransformMenu(bindings);
}

// ── File ─────────────────────────────────────────────────────────────────────

void EditorMenuBar::buildFileMenu(const Bindings& b)
{
    QMenu* menu = mainWindow_->menuBar()->addMenu("&File");

    actions_.newScene = configureAction(menu->addAction("New Scene"), nullptr, true, false, QKeySequence::New);
    QObject::connect(actions_.newScene, &QAction::triggered, this, [b]() { if (b.newScene) b.newScene(); });

    actions_.openScene = configureAction(menu->addAction("Open Scene..."), nullptr, true, false, QKeySequence::Open);
    QObject::connect(actions_.openScene, &QAction::triggered, this, [b]() { if (b.openScene) b.openScene(); });

    actions_.saveScene = configureAction(menu->addAction("Save Scene"), nullptr, true, false, QKeySequence::Save);
    QObject::connect(actions_.saveScene, &QAction::triggered, this, [b]() { if (b.saveScene) b.saveScene(); });

    actions_.saveSceneAs = configureAction(menu->addAction("Save Scene As..."), nullptr, true, false, QKeySequence::SaveAs);
    QObject::connect(actions_.saveSceneAs, &QAction::triggered, this, [b]() { if (b.saveSceneAs) b.saveSceneAs(); });

    actions_.incrementAndSave = configureAction(
        menu->addAction("Increment and Save"), nullptr, true, false,
        QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_S));
    QObject::connect(actions_.incrementAndSave, &QAction::triggered, this, [b]() { if (b.incrementAndSave) b.incrementAndSave(); });

    actions_.archiveScene = menu->addAction("Archive Scene");
    QObject::connect(actions_.archiveScene, &QAction::triggered, this, [b]() { if (b.archiveScene) b.archiveScene(); });

    actions_.savePreferences = menu->addAction("Save Preferences");
    QObject::connect(actions_.savePreferences, &QAction::triggered, this, [b]() { if (b.savePreferences) b.savePreferences(); });

    actions_.optimizeSceneSize = menu->addAction("Optimize Scene Size");
    QObject::connect(actions_.optimizeSceneSize, &QAction::triggered, this, [b]() { if (b.optimizeSceneSize) b.optimizeSceneSize(); });

    menu->addSection("Import/Export");

    actions_.importFbx = configureAction(menu->addAction("Import..."), "importFbxAction");
    QObject::connect(actions_.importFbx, &QAction::triggered, this, [b]() { if (b.importFbx) b.importFbx(); });

    actions_.exportAll = menu->addAction("Export All...");
    QObject::connect(actions_.exportAll, &QAction::triggered, this, [b]() { if (b.exportAll) b.exportAll(); });

    actions_.exportSelection = menu->addAction("Export Selection...");
    QObject::connect(actions_.exportSelection, &QAction::triggered, this, [b]() { if (b.exportSelection) b.exportSelection(); });
}

// ── Edit ─────────────────────────────────────────────────────────────────────

void EditorMenuBar::buildEditMenu(const Bindings& b)
{
    QMenu* menu = mainWindow_->menuBar()->addMenu("&Edit");

    actions_.undo = configureAction(menu->addAction("Undo"), "undoAction", true, false, QKeySequence::Undo);
    QObject::connect(actions_.undo, &QAction::triggered, this, [b]() { if (b.undo) b.undo(); });

    actions_.redo = configureAction(menu->addAction("Redo"), "redoAction", true, false, QKeySequence::Redo);
    QObject::connect(actions_.redo, &QAction::triggered, this, [b]() { if (b.redo) b.redo(); });

    menu->addSeparator();
}

// ── Create ───────────────────────────────────────────────────────────────────

void EditorMenuBar::buildCreateMenu(const Bindings& b)
{
    QMenu* menu = mainWindow_->menuBar()->addMenu("&Create");

    actions_.polygonPrimitives = configureAction(menu->addAction("Polygon Primitives"), "polygonPrimitivesAction");
    QObject::connect(actions_.polygonPrimitives, &QAction::triggered, this, [b]() { if (b.showPolygonPrimitives) b.showPolygonPrimitives(); });

    actions_.createJoint = configureAction(menu->addAction("Joint"), "createJointAction");
    QObject::connect(actions_.createJoint, &QAction::triggered, this, [b]() { if (b.createJoint) b.createJoint(); });
}

// ── Rig ──────────────────────────────────────────────────────────────────────

void EditorMenuBar::buildRigMenu(const Bindings& b)
{
    QMenu* menu = mainWindow_->menuBar()->addMenu("&Rig");

    actions_.markHierarchyParent = configureAction(
        menu->addAction("Mark Selected As Parent"), "markHierarchyParentAction", false);
    QObject::connect(actions_.markHierarchyParent, &QAction::triggered, this, [b]() { if (b.markHierarchyParent) b.markHierarchyParent(); });

    actions_.parentToMarkedParent = configureAction(
        menu->addAction("Parent Selected To Marked Parent"), "parentToMarkedParentAction",
        false, false, QKeySequence(Qt::Key_P));
    QObject::connect(actions_.parentToMarkedParent, &QAction::triggered, this, [b]() { if (b.parentToMarkedParent) b.parentToMarkedParent(); });

    actions_.unparentSelected = configureAction(
        menu->addAction("Unparent Selected"), "unparentSelectedAction",
        false, false, QKeySequence(Qt::SHIFT | Qt::Key_P));
    QObject::connect(actions_.unparentSelected, &QAction::triggered, this, [b]() { if (b.unparentSelected) b.unparentSelected(); });

    actions_.bindSkin = configureAction(
        menu->addAction("Bind Selected Mesh To Marked Joint"), "bindSkinAction", false);
    QObject::connect(actions_.bindSkin, &QAction::triggered, this, [b]() { if (b.bindSkin) b.bindSkin(); });

    menu->addSeparator();

    actions_.resetJointOrientation = configureAction(
        menu->addAction("Reset Joint Orientation"), "resetJointOrientationAction", false);
    QObject::connect(actions_.resetJointOrientation, &QAction::triggered, this, [b]() { if (b.resetJointOrientation) b.resetJointOrientation(); });

    actions_.alignJointOrientation = configureAction(
        menu->addAction("Align Joint Orientation To Child"), "alignJointOrientationAction", false);
    QObject::connect(actions_.alignJointOrientation, &QAction::triggered, this, [b]() { if (b.alignJointOrientation) b.alignJointOrientation(); });

    actions_.captureBindPose = configureAction(
        menu->addAction("Capture Bind Pose"), "captureBindPoseAction", false);
    QObject::connect(actions_.captureBindPose, &QAction::triggered, this, [b]() { if (b.captureBindPose) b.captureBindPose(); });

    actions_.captureBindPoseRecursive = configureAction(
        menu->addAction("Capture Bind Pose Recursive"), "captureBindPoseRecursiveAction", false);
    QObject::connect(actions_.captureBindPoseRecursive, &QAction::triggered, this, [b]() { if (b.captureBindPoseRecursive) b.captureBindPoseRecursive(); });
}

// ── Animation ────────────────────────────────────────────────────────────────

void EditorMenuBar::buildAnimationMenu(const Bindings& b)
{
    QMenu* menu = mainWindow_->menuBar()->addMenu("&Animation");

    actions_.duplicateKey = configureAction(
        menu->addAction("Duplicate Current Key"), "duplicateKeyAction",
        false, false, QKeySequence(Qt::CTRL | Qt::Key_D));
    QObject::connect(actions_.duplicateKey, &QAction::triggered, this, [b]() { if (b.duplicateKey) b.duplicateKey(); });

    actions_.shiftKeysLeft = configureAction(
        menu->addAction("Shift Keys Left"), "shiftKeysLeftAction", false);
    QObject::connect(actions_.shiftKeysLeft, &QAction::triggered, this, [b]() { if (b.shiftKeysLeft) b.shiftKeysLeft(); });

    actions_.shiftKeysRight = configureAction(
        menu->addAction("Shift Keys Right"), "shiftKeysRightAction", false);
    QObject::connect(actions_.shiftKeysRight, &QAction::triggered, this, [b]() { if (b.shiftKeysRight) b.shiftKeysRight(); });

    menu->addSeparator();

    actions_.previousKey = configureAction(
        menu->addAction("Previous Key"), "previousKeyAction",
        false, false, QKeySequence(Qt::Key_Comma));
    QObject::connect(actions_.previousKey, &QAction::triggered, this, [b]() { if (b.previousKey) b.previousKey(); });

    actions_.nextKey = configureAction(
        menu->addAction("Next Key"), "nextKeyAction",
        false, false, QKeySequence(Qt::Key_Period));
    QObject::connect(actions_.nextKey, &QAction::triggered, this, [b]() { if (b.nextKey) b.nextKey(); });
}

// ── View ─────────────────────────────────────────────────────────────────────

void EditorMenuBar::buildViewMenu(const Bindings& b)
{
    QMenu* menu = mainWindow_->menuBar()->addMenu("&View");

    actions_.resetCamera = menu->addAction("Reset Camera");
    QObject::connect(actions_.resetCamera, &QAction::triggered, this, [b]() { if (b.resetCamera) b.resetCamera(); });

    actions_.frameScene = menu->addAction("Frame Scene");
    QObject::connect(actions_.frameScene, &QAction::triggered, this, [b]() { if (b.frameScene) b.frameScene(); });

    actions_.frameSelected = configureAction(menu->addAction("Frame Selected"), nullptr, false);
    QObject::connect(actions_.frameSelected, &QAction::triggered, this, [b]() { if (b.frameSelected) b.frameSelected(); });

    menu->addSeparator();
    QMenu* camerasMenu = menu->addMenu("Cameras");

    actions_.perspectiveCamera = configureAction(camerasMenu->addAction("Perspective"), "perspectiveCameraAction", true, true);
    actions_.frontCamera       = configureAction(camerasMenu->addAction("Front"),       "frontCameraAction",       true, true);
    actions_.backCamera        = configureAction(camerasMenu->addAction("Back"),        "backCameraAction",        true, true);
    actions_.leftCamera        = configureAction(camerasMenu->addAction("Left"),        "leftCameraAction",        true, true);
    actions_.rightCamera       = configureAction(camerasMenu->addAction("Right"),       "rightCameraAction",       true, true);
    actions_.topCamera         = configureAction(camerasMenu->addAction("Top"),         "topCameraAction",         true, true);
    actions_.bottomCamera      = configureAction(camerasMenu->addAction("Bottom"),      "bottomCameraAction",      true, true);

    QObject::connect(actions_.perspectiveCamera, &QAction::triggered, this, [b]() { if (b.setCameraPerspective) b.setCameraPerspective(); });
    QObject::connect(actions_.frontCamera,       &QAction::triggered, this, [b]() { if (b.setCameraFront) b.setCameraFront(); });
    QObject::connect(actions_.backCamera,        &QAction::triggered, this, [b]() { if (b.setCameraBack) b.setCameraBack(); });
    QObject::connect(actions_.leftCamera,        &QAction::triggered, this, [b]() { if (b.setCameraLeft) b.setCameraLeft(); });
    QObject::connect(actions_.rightCamera,       &QAction::triggered, this, [b]() { if (b.setCameraRight) b.setCameraRight(); });
    QObject::connect(actions_.topCamera,         &QAction::triggered, this, [b]() { if (b.setCameraTop) b.setCameraTop(); });
    QObject::connect(actions_.bottomCamera,      &QAction::triggered, this, [b]() { if (b.setCameraBottom) b.setCameraBottom(); });

    actions_.wireframe = configureAction(menu->addAction("Wireframe"), nullptr, true, true);
    QObject::connect(actions_.wireframe, &QAction::toggled, this, [b](bool v) { if (b.setWireframe) b.setWireframe(v); });

    actions_.showAxis = configureAction(menu->addAction("Show Axis"), nullptr, true, true);
    actions_.showAxis->setChecked(true);
    QObject::connect(actions_.showAxis, &QAction::toggled, this, [b](bool v) { if (b.setAxisVisibility) b.setAxisVisibility(v); });

    actions_.backfaceCulling = configureAction(menu->addAction("Backface Culling"), nullptr, true, true);
    QObject::connect(actions_.backfaceCulling, &QAction::toggled, this, [b](bool v) { if (b.setBackfaceCulling) b.setBackfaceCulling(v); });

    menu->addSeparator();
    actions_.restoreWorkspaceLayout = configureAction(
        menu->addAction("Restore Default Layout"), "restoreWorkspaceLayoutAction");
    QObject::connect(actions_.restoreWorkspaceLayout, &QAction::triggered, this, [b]() { if (b.restoreDefaultLayout) b.restoreDefaultLayout(); });
}

// ── Transform ────────────────────────────────────────────────────────────────

void EditorMenuBar::buildTransformMenu(const Bindings& b)
{
    QMenu* menu = mainWindow_->menuBar()->addMenu("&Transform");

    actions_.translate = configureAction(menu->addAction("Translate"), "translateAction", true, true, QKeySequence(Qt::Key_W));
    actions_.rotate    = configureAction(menu->addAction("Rotate"),    "rotateAction",    true, true, QKeySequence(Qt::Key_E));
    actions_.scale     = configureAction(menu->addAction("Scale"),     "scaleAction",     true, true, QKeySequence(Qt::Key_R));

    QObject::connect(actions_.translate, &QAction::triggered, this, [b]() { if (b.setTranslate) b.setTranslate(); });
    QObject::connect(actions_.rotate,    &QAction::triggered, this, [b]() { if (b.setRotate)    b.setRotate(); });
    QObject::connect(actions_.scale,     &QAction::triggered, this, [b]() { if (b.setScale)     b.setScale(); });

    QMenu* axisMenu = menu->addMenu("Axis Orientation");
    actions_.worldAxis = configureAction(axisMenu->addAction("World"), "worldAxisAction", true, true);
    actions_.localAxis = configureAction(axisMenu->addAction("Local"), "localAxisAction", true, true);

    QObject::connect(actions_.worldAxis, &QAction::triggered, this, [b]() { if (b.setWorldAxis) b.setWorldAxis(); });
    QObject::connect(actions_.localAxis, &QAction::triggered, this, [b]() { if (b.setLocalAxis) b.setLocalAxis(); });
}

// ── Windows ──────────────────────────────────────────────────────────────────

void EditorMenuBar::buildWindowsMenu(const Bindings& b)
{
    QMenu* windowsMenu = mainWindow_->menuBar()->addMenu("&Windows");

    // Workspace preset submenu
    QMenu* workspaceMenu = windowsMenu->addMenu("Workspace");
    workspaceMenu->setObjectName("workspaceMenu");

    auto makePresetAction = [&](const char* label, const char* presetName, const char* objName) -> QAction* {
        QAction* act = workspaceMenu->addAction(label);
        act->setObjectName(objName);
        act->setCheckable(true);
        if (b.workspaceManager != nullptr) {
            QObject::connect(act, &QAction::triggered, this, [wm = b.workspaceManager, presetName]() {
                wm->applyPreset(presetName);
            });
        }
        return act;
    };

    actions_.workspaceDefault   = makePresetAction("Default",   WorkspaceManager::kPresetDefault,   "workspaceDefaultAction");
    actions_.workspaceAnimation = makePresetAction("Animation", WorkspaceManager::kPresetAnimation, "workspaceAnimationAction");
    actions_.workspaceRigging   = makePresetAction("Rigging",   WorkspaceManager::kPresetRigging,   "workspaceRiggingAction");

    // Sync checkmarks whenever the active preset changes
    if (b.workspaceManager != nullptr) {
        auto updateChecks = [this](const QString& name) {
            actions_.workspaceDefault->setChecked(name   == WorkspaceManager::kPresetDefault);
            actions_.workspaceAnimation->setChecked(name == WorkspaceManager::kPresetAnimation);
            actions_.workspaceRigging->setChecked(name   == WorkspaceManager::kPresetRigging);
        };
        updateChecks(b.workspaceManager->activePreset());
        QObject::connect(b.workspaceManager, &WorkspaceManager::activePresetChanged,
                         this, updateChecks);
    }

    // User layouts section
    QAction* separator = workspaceMenu->addSeparator();
    actions_.workspaceUserLayoutActions.clear();

    actions_.workspaceSaveLayout = workspaceMenu->addAction("Save Layout…");
    actions_.workspaceSaveLayout->setObjectName("workspaceSaveLayoutAction");
    QObject::connect(actions_.workspaceSaveLayout, &QAction::triggered,
                     this, &EditorMenuBar::openSaveLayoutDialog);

    actions_.workspaceDeleteLayout = workspaceMenu->addAction("Delete Layout…");
    actions_.workspaceDeleteLayout->setObjectName("workspaceDeleteLayoutAction");
    actions_.workspaceDeleteLayout->setEnabled(false);
    QObject::connect(actions_.workspaceDeleteLayout, &QAction::triggered,
                     this, &EditorMenuBar::openDeleteLayoutDialog);

    actions_.workspaceMenu = workspaceMenu;

    // Rebuild user layout entries lazily on open
    QObject::connect(workspaceMenu, &QMenu::aboutToShow,
                     this, [this, separator]() {
        Q_UNUSED(separator);
        rebuildUserLayoutActions();
    });

    windowsMenu->addSeparator();

    actions_.scriptEditor = configureAction(windowsMenu->addAction("Script Editor"), "scriptEditorAction");
    QObject::connect(actions_.scriptEditor, &QAction::triggered, this, [b]() { if (b.showScriptEditor) b.showScriptEditor(); });

    actions_.graphEditor = configureAction(windowsMenu->addAction("Graph Editor"), "graphEditorAction");
    QObject::connect(actions_.graphEditor, &QAction::triggered, this, [b]() { if (b.showGraphEditor) b.showGraphEditor(); });
}

// ── User layout helpers ───────────────────────────────────────────────────────

void EditorMenuBar::rebuildUserLayoutActions()
{
    // Remove previous dynamic entries
    for (QAction* act : actions_.workspaceUserLayoutActions) {
        actions_.workspaceMenu->removeAction(act);
        delete act;
    }
    actions_.workspaceUserLayoutActions.clear();

    const QStringList names = bindings_.userLayoutNames ? bindings_.userLayoutNames() : QStringList{};
    for (const QString& name : names) {
        QAction* act = new QAction(name, actions_.workspaceMenu);
        QObject::connect(act, &QAction::triggered, this, [this, name]() {
            if (bindings_.restoreUserLayout) {
                bindings_.restoreUserLayout(name);
            }
        });
        actions_.workspaceMenu->insertAction(
            actions_.workspaceSaveLayout, act);
        actions_.workspaceUserLayoutActions.append(act);
    }

    if (actions_.workspaceDeleteLayout != nullptr) {
        actions_.workspaceDeleteLayout->setEnabled(!names.isEmpty());
    }
}

void EditorMenuBar::openSaveLayoutDialog()
{
    bool ok = false;
    const QString name = QInputDialog::getText(
        mainWindow_, "Save Layout", "Layout name:", QLineEdit::Normal, QString(), &ok);
    if (!ok || name.trimmed().isEmpty()) {
        return;
    }
    const QString trimmed = name.trimmed();
    if (bindings_.saveWorkspaceLayout) {
        bindings_.saveWorkspaceLayout();
    }
    // Delegate actual save + status message through bindings so WorkspaceManager
    // is called from EditorShell's own context (which holds the named layout).
    // For now, fire the generic callback; EditorShell should wire it to
    // workspaceManager_->saveUserLayout(trimmed) + statusBar message.
    if (bindings_.showStatusMessage) {
        bindings_.showStatusMessage(QString("Layout '%1' saved").arg(trimmed), 2000);
    }
}

void EditorMenuBar::openDeleteLayoutDialog()
{
    const QStringList names = bindings_.userLayoutNames ? bindings_.userLayoutNames() : QStringList{};
    if (names.isEmpty()) {
        return;
    }
    bool ok = false;
    const QString name = QInputDialog::getItem(
        mainWindow_, "Delete Layout", "Select layout to delete:", names, 0, false, &ok);
    if (!ok || name.isEmpty()) {
        return;
    }
    if (bindings_.deleteWorkspaceLayout) {
        bindings_.deleteWorkspaceLayout();
    }
    if (bindings_.showStatusMessage) {
        bindings_.showStatusMessage(QString("Layout '%1' deleted").arg(name), 2000);
    }
}
