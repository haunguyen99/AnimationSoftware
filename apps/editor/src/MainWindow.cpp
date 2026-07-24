#include "MainWindow.h"

#include <QAction>
#include <QApplication>
#include <QDockWidget>
#include <QFileDialog>
#include <QFormLayout>
#include <QCheckBox>
#include <QDateTime>
#include <QDoubleSpinBox>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QListWidget>
#include <QFileInfo>
#include <QMouseEvent>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QSettings>
#include <QSignalBlocker>
#include <QSlider>
#include <QSpinBox>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>
#include <QTreeWidgetItem>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "AnimationTimelinePanel.h"
#include "EditorAnimationFlowController.h"
#include "EditorDocumentController.h"
#include "EditorChannelBoxController.h"
#include "EditorFileFlowController.h"
#include "EditorInspectorController.h"
#include "EditorOutlinerController.h"
#include "EditorRiggingController.h"
#include "EditorScriptExecutionController.h"
#include "EditorSelectionController.h"
#include "EditorViewportUiController.h"
#include "EditorScriptLogController.h"
#include "EditorSceneQueryController.h"
#include "EditorSceneMutationController.h"
#include "EditorViewportCommandController.h"
#include "MainWindowContexts.h"
#include "ViewportWorkspaceWidget.h"
#include "io/PhoenixSceneDocument.h"
#include "logging/LogCategories.h"
#include "scene/PrimitiveMeshFactory.h"
#include "scene/Scene.h"
#include "scene/SceneObject.h"

namespace
{
constexpr auto kPrimitiveTypeRole = Qt::UserRole + 101;
using EditorSceneQueryController::objectDisplayName;
constexpr auto kSettingsOrganization = "ProjectPhoenix";
constexpr auto kSettingsApplication = "PhoenixEditor";
constexpr auto kMainWindowGeometryKey = "mainWindow/geometry";
constexpr auto kMainWindowStateKey = "mainWindow/state";
constexpr auto kAnimationAutoKeyEnabledKey = "animation/autoKeyEnabled";

struct EditorPreferencesState
{
    QByteArray geometry;
    QByteArray windowState;
    bool autoKeyEnabled = false;
};

QDoubleSpinBox* createChannelSpinBox(QWidget* parent)
{
    QDoubleSpinBox* spinBox = new QDoubleSpinBox(parent);
    spinBox->setDecimals(3);
    spinBox->setRange(-999999.0, 999999.0);
    spinBox->setSingleStep(0.1);
    spinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    spinBox->setAlignment(Qt::AlignLeft);
    spinBox->setStyleSheet("QDoubleSpinBox { min-height: 24px; }");
    return spinBox;
}

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

void configureDockWidget(
    QDockWidget* dock,
    const char* objectName,
    Qt::DockWidgetAreas allowedAreas,
    QDockWidget::DockWidgetFeatures features)
{
    if (dock == nullptr) {
        return;
    }

    dock->setObjectName(QString::fromUtf8(objectName));
    dock->setAllowedAreas(allowedAreas);
    dock->setFeatures(features);
}

EditorPreferencesState readPreferencesState()
{
    QSettings settings(kSettingsOrganization, kSettingsApplication);
    EditorPreferencesState state;
    state.geometry = settings.value(kMainWindowGeometryKey).toByteArray();
    state.windowState = settings.value(kMainWindowStateKey).toByteArray();
    state.autoKeyEnabled = settings.value(kAnimationAutoKeyEnabledKey, false).toBool();
    return state;
}

void writePreferencesState(const EditorPreferencesState& state)
{
    QSettings settings(kSettingsOrganization, kSettingsApplication);
    settings.setValue(kMainWindowGeometryKey, state.geometry);
    settings.setValue(kMainWindowStateKey, state.windowState);
    settings.setValue(kAnimationAutoKeyEnabledKey, state.autoKeyEnabled);
}
}

MainWindow::MainWindow()
{
    setWindowTitle("Phoenix Editor Beta");
    resize(1440, 820);
    setDockNestingEnabled(true);
    setDockOptions(QMainWindow::AllowNestedDocks
        | QMainWindow::AllowTabbedDocks
        | QMainWindow::GroupedDragging
        | QMainWindow::AnimatedDocks);

    viewport_ = new ViewportWorkspaceWidget(this);
    playbackTimer_ = new QTimer(this);
    playbackTimer_->setInterval(1000 / 24);
    QObject::connect(playbackTimer_, &QTimer::timeout, this, &MainWindow::advancePlayback);
    viewport_->setSelectionChangedCallback([this](SceneObject::Id objectId) {
        selectObject(objectId, true);
        if (objectId == 0) {
            EditorScriptLogController::logSelectionCleared(scriptHistoryTextEdit_);
            statusBar()->showMessage("Selection cleared", 2000);
            return;
        }

        if (const SceneObject* object = viewport_->findObject(objectId)) {
            EditorScriptLogController::logSelection(scriptHistoryTextEdit_, object);
            statusBar()->showMessage(QString("Selected: %1").arg(objectDisplayName(*object)), 2000);
        }
    });
    viewport_->setObjectTransformChangedCallback([this](SceneObject::Id objectId) {
        selectObject(objectId, true);
        statusBar()->showMessage("Object transform updated", 1500);
    });
    viewport_->setBeforeSceneMutationCallback([this]() {
        if (!restoringHistory_) {
            recordUndoState();
        }
    });

    createMenus();
    createDocks();
    loadPreferences();
    updateWindowTitle();
    clearInspector();
    updateUndoRedoActions();
    applyAnimationState(animationState_);
    statusBar()->showMessage("Ready");
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
    if (outlinerTree_ != nullptr && watched == outlinerTree_->viewport() && event != nullptr) {
        if (event->type() == QEvent::MouseButtonPress) {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                if (QTreeWidgetItem* item = outlinerTree_->itemAt(mouseEvent->pos())) {
                    hierarchyDragSourceId_ = item->data(0, Qt::UserRole).toULongLong();
                    hierarchyDragStartPos_ = mouseEvent->pos();
                    hierarchyDragActive_ = hierarchyDragSourceId_ != 0;
                    if (hierarchyDragActive_) {
                        outlinerTree_->setCurrentItem(item);
                        outlinerTree_->viewport()->setCursor(Qt::ClosedHandCursor);
                        return true;
                    }
                }
            }
        } else if (event->type() == QEvent::MouseMove) {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (hierarchyDragActive_ && (mouseEvent->buttons() & Qt::LeftButton)) {
                return true;
            }
        } else if (event->type() == QEvent::MouseButtonRelease) {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (hierarchyDragActive_ && mouseEvent->button() == Qt::LeftButton) {
                outlinerTree_->viewport()->unsetCursor();
                const QPoint delta = mouseEvent->pos() - hierarchyDragStartPos_;
                const bool dragged = delta.manhattanLength() >= QApplication::startDragDistance();
                std::uint64_t targetParentId = 0;
                if (dragged) {
                    if (QTreeWidgetItem* targetItem = outlinerTree_->itemAt(mouseEvent->pos())) {
                        targetParentId = targetItem->data(0, Qt::UserRole).toULongLong();
                    }
                    reparentObjectInUi(hierarchyDragSourceId_, targetParentId);
                }

                hierarchyDragSourceId_ = 0;
                hierarchyDragActive_ = false;
                return true;
            }
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::createMenus()
{
    createEditMenu();
    createFileMenu();
    createCreateMenu();
    createRigMenu();
    createWindowsMenu();
    createAnimationMenu();
    createViewMenu();
    createTransformMenu();
}

void MainWindow::createEditMenu()
{
    QMenu* editMenu = menuBar()->addMenu("&Edit");
    undoAction_ = configureAction(editMenu->addAction("Undo"), "undoAction", true, false, QKeySequence::Undo);
    QObject::connect(undoAction_, &QAction::triggered, this, &MainWindow::undoLastChange);

    redoAction_ = configureAction(editMenu->addAction("Redo"), "redoAction", true, false, QKeySequence::Redo);
    QObject::connect(redoAction_, &QAction::triggered, this, &MainWindow::redoLastChange);

    editMenu->addSeparator();
}

void MainWindow::createFileMenu()
{
    QMenu* fileMenu = menuBar()->addMenu("&File");
    newSceneAction_ = configureAction(fileMenu->addAction("New Scene"), nullptr, true, false, QKeySequence::New);
    QObject::connect(newSceneAction_, &QAction::triggered, this, &MainWindow::newScene);

    openSceneAction_ = configureAction(fileMenu->addAction("Open Scene..."), nullptr, true, false, QKeySequence::Open);
    QObject::connect(openSceneAction_, &QAction::triggered, this, &MainWindow::openScene);

    saveSceneAction_ = configureAction(fileMenu->addAction("Save Scene"), nullptr, true, false, QKeySequence::Save);
    QObject::connect(saveSceneAction_, &QAction::triggered, this, &MainWindow::saveScene);

    saveSceneAsAction_ = configureAction(fileMenu->addAction("Save Scene As..."), nullptr, true, false, QKeySequence::SaveAs);
    QObject::connect(saveSceneAsAction_, &QAction::triggered, this, &MainWindow::saveSceneAs);

    incrementAndSaveAction_ = configureAction(
        fileMenu->addAction("Increment and Save"),
        nullptr,
        true,
        false,
        QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_S));
    QObject::connect(incrementAndSaveAction_, &QAction::triggered, this, &MainWindow::incrementAndSave);

    archiveSceneAction_ = fileMenu->addAction("Archive Scene");
    QObject::connect(archiveSceneAction_, &QAction::triggered, this, &MainWindow::archiveScene);

    savePreferencesAction_ = fileMenu->addAction("Save Preferences");
    QObject::connect(savePreferencesAction_, &QAction::triggered, this, &MainWindow::savePreferences);

    optimizeSceneSizeAction_ = fileMenu->addAction("Optimize Scene Size");
    QObject::connect(optimizeSceneSizeAction_, &QAction::triggered, this, &MainWindow::optimizeSceneStorage);

    fileMenu->addSection("Import/Export");
    importFbxAction_ = configureAction(fileMenu->addAction("Import..."), "importFbxAction");
    QObject::connect(importFbxAction_, &QAction::triggered, this, &MainWindow::importFbx);

    exportAllAction_ = fileMenu->addAction("Export All...");
    QObject::connect(exportAllAction_, &QAction::triggered, this, &MainWindow::exportAll);

    exportSelectionAction_ = fileMenu->addAction("Export Selection...");
    QObject::connect(exportSelectionAction_, &QAction::triggered, this, &MainWindow::exportSelection);
}

void MainWindow::createCreateMenu()
{
    QMenu* createMenu = menuBar()->addMenu("&Create");
    polygonPrimitivesAction_ = configureAction(createMenu->addAction("Polygon Primitives"), "polygonPrimitivesAction");
    QObject::connect(polygonPrimitivesAction_, &QAction::triggered, this, &MainWindow::showPolygonPrimitivesWindow);
    createJointAction_ = configureAction(createMenu->addAction("Joint"), "createJointAction");
    QObject::connect(createJointAction_, &QAction::triggered, this, &MainWindow::createJoint);
}

void MainWindow::createRigMenu()
{
    QMenu* rigMenu = menuBar()->addMenu("&Rig");
    markHierarchyParentAction_ = configureAction(
        rigMenu->addAction("Mark Selected As Parent"),
        "markHierarchyParentAction",
        false);
    QObject::connect(markHierarchyParentAction_, &QAction::triggered, this, &MainWindow::markSelectionAsHierarchyParent);

    parentToMarkedParentAction_ = configureAction(
        rigMenu->addAction("Parent Selected To Marked Parent"),
        "parentToMarkedParentAction",
        false,
        false,
        QKeySequence(Qt::Key_P));
    QObject::connect(parentToMarkedParentAction_, &QAction::triggered, this, &MainWindow::parentSelectionToMarkedParent);

    unparentSelectedAction_ = configureAction(
        rigMenu->addAction("Unparent Selected"),
        "unparentSelectedAction",
        false,
        false,
        QKeySequence(Qt::SHIFT | Qt::Key_P));
    QObject::connect(unparentSelectedAction_, &QAction::triggered, this, &MainWindow::unparentSelection);

    bindSkinAction_ = configureAction(rigMenu->addAction("Bind Selected Mesh To Marked Joint"), "bindSkinAction", false);
    QObject::connect(bindSkinAction_, &QAction::triggered, this, &MainWindow::bindSelectedMeshToMarkedJoint);

    rigMenu->addSeparator();
    resetJointOrientationAction_ = configureAction(
        rigMenu->addAction("Reset Joint Orientation"),
        "resetJointOrientationAction",
        false);
    QObject::connect(resetJointOrientationAction_, &QAction::triggered, this, &MainWindow::resetSelectedJointOrientation);

    alignJointOrientationAction_ = configureAction(
        rigMenu->addAction("Align Joint Orientation To Child"),
        "alignJointOrientationAction",
        false);
    QObject::connect(alignJointOrientationAction_, &QAction::triggered, this, &MainWindow::alignSelectedJointOrientationToChild);

    captureBindPoseAction_ = configureAction(rigMenu->addAction("Capture Bind Pose"), "captureBindPoseAction", false);
    QObject::connect(captureBindPoseAction_, &QAction::triggered, this, &MainWindow::captureSelectedBindPose);

    captureBindPoseRecursiveAction_ = configureAction(
        rigMenu->addAction("Capture Bind Pose Recursive"),
        "captureBindPoseRecursiveAction",
        false);
    QObject::connect(captureBindPoseRecursiveAction_, &QAction::triggered, this, &MainWindow::captureSelectedBindPoseRecursive);
}

void MainWindow::createWindowsMenu()
{
    QMenu* windowsMenu = menuBar()->addMenu("&Windows");
    scriptEditorAction_ = configureAction(windowsMenu->addAction("Script Editor"), "scriptEditorAction");
    QObject::connect(scriptEditorAction_, &QAction::triggered, this, &MainWindow::showScriptEditorWindow);
}

void MainWindow::createAnimationMenu()
{
    QMenu* animationMenu = menuBar()->addMenu("&Animation");
    duplicateKeyAction_ = configureAction(
        animationMenu->addAction("Duplicate Current Key"),
        "duplicateKeyAction",
        false,
        false,
        QKeySequence(Qt::CTRL | Qt::Key_D));
    QObject::connect(duplicateKeyAction_, &QAction::triggered, this, [this]() { duplicateCurrentKeyForSelection(true); });

    shiftKeysLeftAction_ = configureAction(animationMenu->addAction("Shift Keys Left"), "shiftKeysLeftAction", false);
    QObject::connect(shiftKeysLeftAction_, &QAction::triggered, this, [this]() { shiftSelectedObjectKeyframes(-1, true); });

    shiftKeysRightAction_ = configureAction(animationMenu->addAction("Shift Keys Right"), "shiftKeysRightAction", false);
    QObject::connect(shiftKeysRightAction_, &QAction::triggered, this, [this]() { shiftSelectedObjectKeyframes(1, true); });

    animationMenu->addSeparator();
    previousKeyAction_ = configureAction(
        animationMenu->addAction("Previous Key"),
        "previousKeyAction",
        false,
        false,
        QKeySequence(Qt::Key_Comma));
    QObject::connect(previousKeyAction_, &QAction::triggered, this, [this]() { jumpToSelectedObjectKeyframe(false, true); });

    nextKeyAction_ = configureAction(
        animationMenu->addAction("Next Key"),
        "nextKeyAction",
        false,
        false,
        QKeySequence(Qt::Key_Period));
    QObject::connect(nextKeyAction_, &QAction::triggered, this, [this]() { jumpToSelectedObjectKeyframe(true, true); });
}

void MainWindow::createViewMenu()
{
    QMenu* viewMenu = menuBar()->addMenu("&View");

    resetCameraAction_ = viewMenu->addAction("Reset Camera");
    QObject::connect(resetCameraAction_, &QAction::triggered, this, &MainWindow::resetSceneCamera);

    frameSceneAction_ = viewMenu->addAction("Frame Scene");
    QObject::connect(frameSceneAction_, &QAction::triggered, this, &MainWindow::frameEntireScene);

    frameSelectedAction_ = configureAction(viewMenu->addAction("Frame Selected"), nullptr, false);
    QObject::connect(frameSelectedAction_, &QAction::triggered, this, &MainWindow::frameSelectedObject);

    viewMenu->addSeparator();
    QMenu* camerasMenu = viewMenu->addMenu("Cameras");
    perspectiveCameraAction_ = configureAction(camerasMenu->addAction("Perspective"), "perspectiveCameraAction", true, true);
    frontCameraAction_ = configureAction(camerasMenu->addAction("Front"), "frontCameraAction", true, true);
    backCameraAction_ = configureAction(camerasMenu->addAction("Back"), "backCameraAction", true, true);
    leftCameraAction_ = configureAction(camerasMenu->addAction("Left"), "leftCameraAction", true, true);
    rightCameraAction_ = configureAction(camerasMenu->addAction("Right"), "rightCameraAction", true, true);
    topCameraAction_ = configureAction(camerasMenu->addAction("Top"), "topCameraAction", true, true);
    bottomCameraAction_ = configureAction(camerasMenu->addAction("Bottom"), "bottomCameraAction", true, true);

    QObject::connect(perspectiveCameraAction_, &QAction::triggered, this, [this]() { setViewCameraPreset(ViewCameraUiPreset::Perspective); });
    QObject::connect(frontCameraAction_, &QAction::triggered, this, [this]() { setViewCameraPreset(ViewCameraUiPreset::Front); });
    QObject::connect(backCameraAction_, &QAction::triggered, this, [this]() { setViewCameraPreset(ViewCameraUiPreset::Back); });
    QObject::connect(leftCameraAction_, &QAction::triggered, this, [this]() { setViewCameraPreset(ViewCameraUiPreset::Left); });
    QObject::connect(rightCameraAction_, &QAction::triggered, this, [this]() { setViewCameraPreset(ViewCameraUiPreset::Right); });
    QObject::connect(topCameraAction_, &QAction::triggered, this, [this]() { setViewCameraPreset(ViewCameraUiPreset::Top); });
    QObject::connect(bottomCameraAction_, &QAction::triggered, this, [this]() { setViewCameraPreset(ViewCameraUiPreset::Bottom); });
    setViewCameraPreset(ViewCameraUiPreset::Perspective);

    wireframeAction_ = configureAction(viewMenu->addAction("Wireframe"), nullptr, true, true);
    QObject::connect(wireframeAction_, &QAction::toggled, this, &MainWindow::setWireframeDisplayEnabled);

    showAxisAction_ = configureAction(viewMenu->addAction("Show Axis"), nullptr, true, true);
    showAxisAction_->setChecked(true);
    QObject::connect(showAxisAction_, &QAction::toggled, this, &MainWindow::setAxisVisibilityEnabled);

    backfaceCullingAction_ = configureAction(viewMenu->addAction("Backface Culling"), nullptr, true, true);
    QObject::connect(backfaceCullingAction_, &QAction::toggled, this, &MainWindow::setBackfaceCullingEnabled);

    viewMenu->addSeparator();
    restoreWorkspaceLayoutAction_ = configureAction(
        viewMenu->addAction("Restore Default Layout"),
        "restoreWorkspaceLayoutAction");
    QObject::connect(restoreWorkspaceLayoutAction_, &QAction::triggered, this, &MainWindow::restoreDefaultWorkspaceLayout);
}

void MainWindow::createTransformMenu()
{
    QMenu* transformMenu = menuBar()->addMenu("&Transform");
    translateAction_ = configureAction(transformMenu->addAction("Translate"), "translateAction", true, true, QKeySequence(Qt::Key_W));
    rotateAction_ = configureAction(transformMenu->addAction("Rotate"), "rotateAction", true, true, QKeySequence(Qt::Key_E));
    scaleAction_ = configureAction(transformMenu->addAction("Scale"), "scaleAction", true, true, QKeySequence(Qt::Key_R));

    QObject::connect(translateAction_, &QAction::triggered, this, [this]() { setTransformUiMode(TransformUiMode::Translate); });
    QObject::connect(rotateAction_, &QAction::triggered, this, [this]() { setTransformUiMode(TransformUiMode::Rotate); });
    QObject::connect(scaleAction_, &QAction::triggered, this, [this]() { setTransformUiMode(TransformUiMode::Scale); });
    setTransformUiMode(TransformUiMode::Translate);

    QMenu* axisMenu = transformMenu->addMenu("Axis Orientation");
    worldAxisAction_ = configureAction(axisMenu->addAction("World"), "worldAxisAction", true, true);
    localAxisAction_ = configureAction(axisMenu->addAction("Local"), "localAxisAction", true, true);

    QObject::connect(worldAxisAction_, &QAction::triggered, this, [this]() { setAxisUiOrientation(AxisUiOrientation::World); });
    QObject::connect(localAxisAction_, &QAction::triggered, this, [this]() { setAxisUiOrientation(AxisUiOrientation::Local); });
    setAxisUiOrientation(AxisUiOrientation::World);
}

void MainWindow::createToolbar()
{
    toolbar_ = addToolBar("Viewport");
    toolbar_->setMovable(false);
    addImportCreateToolbarSection();
    addRigToolbarSection();
    addViewToolbarSection();
    addTransformToolbarSection();
    addDisplayToolbarSection();
}

void MainWindow::addImportCreateToolbarSection()
{
    toolbar_->addAction(importFbxAction_);
    toolbar_->addAction(polygonPrimitivesAction_);
    toolbar_->addAction(createJointAction_);
}

void MainWindow::addRigToolbarSection()
{
    toolbar_->addSeparator();
    toolbar_->addAction(markHierarchyParentAction_);
    toolbar_->addAction(parentToMarkedParentAction_);
    toolbar_->addAction(unparentSelectedAction_);
    toolbar_->addAction(alignJointOrientationAction_);
    toolbar_->addAction(captureBindPoseAction_);
}

void MainWindow::addViewToolbarSection()
{
    toolbar_->addSeparator();
    toolbar_->addAction(resetCameraAction_);
    toolbar_->addAction(frameSceneAction_);
    toolbar_->addAction(frameSelectedAction_);
}

void MainWindow::addTransformToolbarSection()
{
    toolbar_->addSeparator();
    toolbar_->addAction(translateAction_);
    toolbar_->addAction(rotateAction_);
    toolbar_->addAction(scaleAction_);
    toolbar_->addAction(worldAxisAction_);
    toolbar_->addAction(localAxisAction_);
}

void MainWindow::addDisplayToolbarSection()
{
    toolbar_->addSeparator();
    toolbar_->addAction(wireframeAction_);
    toolbar_->addAction(showAxisAction_);
    toolbar_->addAction(backfaceCullingAction_);
}

void MainWindow::createDocks()
{
    viewport_->setObjectName("viewportWidget");
    setCentralWidget(viewport_);
    viewportDock_ = nullptr;
    createOutlinerDock();
    createInspectorDock();
    resizeDocks({ outlinerDock_, inspectorDock_ }, { 280, 320 }, Qt::Horizontal);
    createPrimitivePaletteDock();
    createScriptEditorDock();
    createTimelineDock();
}

void MainWindow::createOutlinerDock()
{
    outlinerDock_ = new QDockWidget("Outliner", this);
    configureDockWidget(
        outlinerDock_,
        "OutlinerDock",
        Qt::AllDockWidgetAreas,
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    outlinerDock_->setWidget(createOutlinerPanel());
    addDockWidget(Qt::LeftDockWidgetArea, outlinerDock_);
}

void MainWindow::createInspectorDock()
{
    inspectorDock_ = new QDockWidget("Channel Box", this);
    configureDockWidget(
        inspectorDock_,
        "InspectorDock",
        Qt::AllDockWidgetAreas,
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    inspectorDock_->setWidget(createInspectorPanel());
    addDockWidget(Qt::RightDockWidgetArea, inspectorDock_);
}

void MainWindow::createPrimitivePaletteDock()
{
    polygonPrimitivesDock_ = new QDockWidget("Polygon Primitives", this);
    configureDockWidget(
        polygonPrimitivesDock_,
        "PolygonPrimitivesDock",
        Qt::AllDockWidgetAreas,
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);
    polygonPrimitivesDock_->setWidget(createPrimitivePalettePanel());
    addDockWidget(Qt::RightDockWidgetArea, polygonPrimitivesDock_);
    polygonPrimitivesDock_->setFloating(true);
    polygonPrimitivesDock_->hide();
}

void MainWindow::createScriptEditorDock()
{
    scriptEditorDock_ = new QDockWidget("Script Editor", this);
    configureDockWidget(
        scriptEditorDock_,
        "ScriptEditorDock",
        Qt::AllDockWidgetAreas,
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);
    scriptEditorDock_->setWidget(createScriptEditorPanel());
    addDockWidget(Qt::BottomDockWidgetArea, scriptEditorDock_);
    scriptEditorDock_->setFloating(true);
    scriptEditorDock_->hide();
}

void MainWindow::createTimelineDock()
{
    timeSliderDock_ = new QDockWidget("Time Slider", this);
    configureDockWidget(
        timeSliderDock_,
        "TimeSliderDock",
        Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea,
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    timeSliderDock_->setWidget(createTimeSliderPanel());
    addDockWidget(Qt::BottomDockWidgetArea, timeSliderDock_);
    resizeDocks({ timeSliderDock_ }, { 150 }, Qt::Vertical);
}

QWidget* MainWindow::createPrimitivePalettePanel()
{
    QWidget* primitivesPanel = new QWidget(this);
    QVBoxLayout* primitivesLayout = new QVBoxLayout(primitivesPanel);
    primitivesLayout->setContentsMargins(8, 8, 8, 8);

    polygonPrimitivesList_ = new QListWidget(primitivesPanel);
    polygonPrimitivesList_->setObjectName("polygonPrimitivesList");
    for (const EditorCreationController::PrimitivePaletteEntry& entry : EditorCreationController::primitivePaletteEntries()) {
        QListWidgetItem* item = new QListWidgetItem(entry.label, polygonPrimitivesList_);
        item->setData(kPrimitiveTypeRole, static_cast<int>(entry.type));
        if (!entry.implemented) {
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
            item->setText(QString("%1 (coming soon)").arg(entry.label));
        }
    }
    QObject::connect(polygonPrimitivesList_, &QListWidget::itemClicked, this, &MainWindow::handlePrimitivePaletteItemActivated);
    QObject::connect(polygonPrimitivesList_, &QListWidget::itemDoubleClicked, this, &MainWindow::handlePrimitivePaletteItemActivated);

    QPushButton* createPrimitiveButton = new QPushButton("Create Selected Primitive", primitivesPanel);
    createPrimitiveButton->setObjectName("createPrimitiveButton");
    QObject::connect(createPrimitiveButton, &QPushButton::clicked, this, &MainWindow::createPrimitiveFromPalette);

    QCheckBox* interactiveCreationCheckBox = new QCheckBox("Interactive Creation", primitivesPanel);
    interactiveCreationCheckBox->setObjectName("interactiveCreationCheckBox");
    interactiveCreationCheckBox->setChecked(interactivePrimitiveCreationEnabled_);
    QObject::connect(interactiveCreationCheckBox, &QCheckBox::toggled, this, &MainWindow::setInteractivePrimitiveCreationEnabled);

    QCheckBox* exitOnCompletionCheckBox = new QCheckBox("Exit On Completion", primitivesPanel);
    exitOnCompletionCheckBox->setObjectName("exitOnCompletionCheckBox");
    exitOnCompletionCheckBox->setChecked(exitPrimitiveToolOnCompletionEnabled_);
    QObject::connect(exitOnCompletionCheckBox, &QCheckBox::toggled, this, &MainWindow::setExitPrimitiveToolOnCompletionEnabled);

    primitivesLayout->addWidget(polygonPrimitivesList_);
    primitivesLayout->addWidget(createPrimitiveButton);
    primitivesLayout->addWidget(interactiveCreationCheckBox);
    primitivesLayout->addWidget(exitOnCompletionCheckBox);
    return primitivesPanel;
}

void MainWindow::newScene()
{
    viewport_->clearScene();
    setCurrentFrame(viewport_->currentFrame(), false);
    currentSceneFilePath_.clear();
    refreshScenePanels();
    viewport_->resetCamera();
    updateWindowTitle();
    applyFileFlowResult(EditorFileFlowController::buildNewSceneResult(), true, 2000);
}

void MainWindow::openScene()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        "Open Scene",
        currentSceneFilePath_.isEmpty() ? QString() : QFileInfo(currentSceneFilePath_).absolutePath(),
        "Phoenix Scene (*.phoenixscene)");

    if (filePath.isEmpty()) {
        statusBar()->showMessage("Open scene cancelled", 1500);
        return;
    }

    openSceneFromPath(filePath, true);
}

QWidget* MainWindow::createOutlinerPanel()
{
    QWidget* panel = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(8, 8, 8, 8);

    outlinerTree_ = new QTreeWidget(panel);
    outlinerTree_->setObjectName("outlinerTree");
    outlinerTree_->setHeaderLabel("Scene");
    outlinerTree_->setSelectionMode(QAbstractItemView::SingleSelection);
    outlinerTree_->viewport()->installEventFilter(this);
    QObject::connect(outlinerTree_, &QTreeWidget::itemSelectionChanged, this, &MainWindow::handleOutlinerSelectionChanged);
    layout->addWidget(outlinerTree_);

    return panel;
}

QWidget* MainWindow::createInspectorPanel()
{
    QWidget* inspectorPanel = new QWidget(this);
    QVBoxLayout* inspectorLayout = new QVBoxLayout(inspectorPanel);
    inspectorLayout->setContentsMargins(12, 12, 12, 12);

    inspectorEmptyStateLabel_ = new QLabel(inspectorPanel);
    inspectorEmptyStateLabel_->setWordWrap(true);

    inspectorDetailsWidget_ = new QWidget(inspectorPanel);
    QFormLayout* inspectorFormLayout = new QFormLayout(inspectorDetailsWidget_);
    inspectorFormLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    channelObjectNameLabel_ = new QLabel(inspectorDetailsWidget_);
    channelObjectNameLabel_->setObjectName("channelObjectNameLabel");
    channelObjectNameLabel_->setWordWrap(true);
    channelObjectNameLabel_->setStyleSheet("font-weight: 600; padding-bottom: 4px;");

    translateXSpinBox_ = createChannelSpinBox(inspectorDetailsWidget_);
    translateXSpinBox_->setObjectName("translateXSpinBox");
    translateYSpinBox_ = createChannelSpinBox(inspectorDetailsWidget_);
    translateYSpinBox_->setObjectName("translateYSpinBox");
    translateZSpinBox_ = createChannelSpinBox(inspectorDetailsWidget_);
    translateZSpinBox_->setObjectName("translateZSpinBox");
    rotateXSpinBox_ = createChannelSpinBox(inspectorDetailsWidget_);
    rotateXSpinBox_->setObjectName("rotateXSpinBox");
    rotateYSpinBox_ = createChannelSpinBox(inspectorDetailsWidget_);
    rotateYSpinBox_->setObjectName("rotateYSpinBox");
    rotateZSpinBox_ = createChannelSpinBox(inspectorDetailsWidget_);
    rotateZSpinBox_->setObjectName("rotateZSpinBox");
    scaleXSpinBox_ = createChannelSpinBox(inspectorDetailsWidget_);
    scaleXSpinBox_->setObjectName("scaleXSpinBox");
    scaleYSpinBox_ = createChannelSpinBox(inspectorDetailsWidget_);
    scaleYSpinBox_->setObjectName("scaleYSpinBox");
    scaleZSpinBox_ = createChannelSpinBox(inspectorDetailsWidget_);
    scaleZSpinBox_->setObjectName("scaleZSpinBox");
    scaleXSpinBox_->setValue(1.0);
    scaleYSpinBox_->setValue(1.0);
    scaleZSpinBox_->setValue(1.0);
    visibilityCheckBox_ = new QCheckBox("on", inspectorDetailsWidget_);
    visibilityCheckBox_->setObjectName("visibilityCheckBox");

    jointToolsWidget_ = new QWidget(inspectorDetailsWidget_);
    jointToolsWidget_->setObjectName("jointToolsWidget");
    QVBoxLayout* jointToolsLayout = new QVBoxLayout(jointToolsWidget_);
    jointToolsLayout->setContentsMargins(0, 0, 0, 0);
    jointToolsLayout->setSpacing(6);

    QWidget* jointOrientationWidget = new QWidget(jointToolsWidget_);
    QFormLayout* jointOrientationLayout = new QFormLayout(jointOrientationWidget);
    jointOrientationLayout->setContentsMargins(0, 0, 0, 0);
    jointOrientationLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    jointOrientXSpinBox_ = createChannelSpinBox(jointOrientationWidget);
    jointOrientXSpinBox_->setObjectName("jointOrientXSpinBox");
    jointOrientYSpinBox_ = createChannelSpinBox(jointOrientationWidget);
    jointOrientYSpinBox_->setObjectName("jointOrientYSpinBox");
    jointOrientZSpinBox_ = createChannelSpinBox(jointOrientationWidget);
    jointOrientZSpinBox_->setObjectName("jointOrientZSpinBox");
    jointOrientationLayout->addRow("Joint Orient X", jointOrientXSpinBox_);
    jointOrientationLayout->addRow("Joint Orient Y", jointOrientYSpinBox_);
    jointOrientationLayout->addRow("Joint Orient Z", jointOrientZSpinBox_);
    jointToolsLayout->addWidget(jointOrientationWidget);

    bindPoseStatusLabel_ = new QLabel("Bind pose: n/a", jointToolsWidget_);
    bindPoseStatusLabel_->setObjectName("bindPoseStatusLabel");
    bindPoseStatusLabel_->setWordWrap(true);
    jointToolsLayout->addWidget(bindPoseStatusLabel_);

    skinBindingStatusLabel_ = new QLabel("Skin binding: n/a", jointToolsWidget_);
    skinBindingStatusLabel_->setObjectName("skinBindingStatusLabel");
    skinBindingStatusLabel_->setWordWrap(true);
    jointToolsLayout->addWidget(skinBindingStatusLabel_);

    resetJointOrientationButton_ = new QPushButton("Reset Orientation", jointToolsWidget_);
    resetJointOrientationButton_->setObjectName("resetJointOrientationButton");
    alignJointOrientationButton_ = new QPushButton("Align To First Child", jointToolsWidget_);
    alignJointOrientationButton_->setObjectName("alignJointOrientationButton");
    captureBindPoseButton_ = new QPushButton("Capture Bind Pose", jointToolsWidget_);
    captureBindPoseButton_->setObjectName("captureBindPoseButton");
    captureBindPoseRecursiveButton_ = new QPushButton("Capture Bind Pose Recursive", jointToolsWidget_);
    captureBindPoseRecursiveButton_->setObjectName("captureBindPoseRecursiveButton");
    jointToolsLayout->addWidget(resetJointOrientationButton_);
    jointToolsLayout->addWidget(alignJointOrientationButton_);
    jointToolsLayout->addWidget(captureBindPoseButton_);
    jointToolsLayout->addWidget(captureBindPoseRecursiveButton_);

    inspectorFormLayout->addRow(channelObjectNameLabel_);
    inspectorFormLayout->addRow("Translate X", translateXSpinBox_);
    inspectorFormLayout->addRow("Translate Y", translateYSpinBox_);
    inspectorFormLayout->addRow("Translate Z", translateZSpinBox_);
    inspectorFormLayout->addRow("Rotate X", rotateXSpinBox_);
    inspectorFormLayout->addRow("Rotate Y", rotateYSpinBox_);
    inspectorFormLayout->addRow("Rotate Z", rotateZSpinBox_);
    inspectorFormLayout->addRow("Scale X", scaleXSpinBox_);
    inspectorFormLayout->addRow("Scale Y", scaleYSpinBox_);
    inspectorFormLayout->addRow("Scale Z", scaleZSpinBox_);
    inspectorFormLayout->addRow("Visibility", visibilityCheckBox_);
    inspectorFormLayout->addRow("Joint Tools", jointToolsWidget_);

    for (QDoubleSpinBox* spinBox : { translateXSpinBox_, translateYSpinBox_, translateZSpinBox_,
             rotateXSpinBox_, rotateYSpinBox_, rotateZSpinBox_,
             scaleXSpinBox_, scaleYSpinBox_, scaleZSpinBox_ }) {
        QObject::connect(spinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double) {
            applyChannelBoxToSelection();
        });
    }
    for (QDoubleSpinBox* spinBox : { jointOrientXSpinBox_, jointOrientYSpinBox_, jointOrientZSpinBox_ }) {
        QObject::connect(spinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double) {
            applyJointOrientationToSelection();
        });
    }
    QObject::connect(visibilityCheckBox_, &QCheckBox::toggled, this, &MainWindow::applyVisibilityToSelection);
    QObject::connect(resetJointOrientationButton_, &QPushButton::clicked, this, &MainWindow::resetSelectedJointOrientation);
    QObject::connect(alignJointOrientationButton_, &QPushButton::clicked, this, &MainWindow::alignSelectedJointOrientationToChild);
    QObject::connect(captureBindPoseButton_, &QPushButton::clicked, this, &MainWindow::captureSelectedBindPose);
    QObject::connect(captureBindPoseRecursiveButton_, &QPushButton::clicked, this, &MainWindow::captureSelectedBindPoseRecursive);

    frameSelectedButton_ = new QPushButton("Frame Selected", inspectorPanel);
    frameSelectedButton_->setObjectName("frameSelectedButton");
    frameSelectedButton_->setEnabled(false);
    QObject::connect(frameSelectedButton_, &QPushButton::clicked, this, &MainWindow::frameSelectedObject);

    inspectorLayout->addWidget(inspectorEmptyStateLabel_);
    inspectorLayout->addWidget(inspectorDetailsWidget_);
    inspectorLayout->addWidget(frameSelectedButton_);
    inspectorLayout->addStretch();

    return inspectorPanel;
}

QWidget* MainWindow::createTimeSliderPanel()
{
    animationTimelinePanel_ = new AnimationTimelinePanel(this);
    animationTimelinePanel_->bindTimelineActions(
        duplicateKeyAction_,
        shiftKeysLeftAction_,
        shiftKeysRightAction_,
        previousKeyAction_,
        nextKeyAction_);
    animationTimelinePanel_->setPlaybackRangeChangedCallback([this](int startFrame, int endFrame) {
        if (!updatingTimeSlider_) {
            setPlaybackRange(startFrame, endFrame);
        }
    });
    animationTimelinePanel_->setCurrentFrameChangedCallback([this](int frame) {
        if (!updatingTimeSlider_) {
            setCurrentFrame(frame);
        }
    });
    animationTimelinePanel_->setJumpStartCallback([this]() { setCurrentFrame(animationState_.playbackStartFrame); });
    animationTimelinePanel_->setStepBackCallback([this]() { stepFrame(-1); });
    animationTimelinePanel_->setTogglePlaybackCallback([this]() { togglePlayback(); });
    animationTimelinePanel_->setStepForwardCallback([this]() { stepFrame(1); });
    animationTimelinePanel_->setPreviousKeyCallback([this]() { jumpToSelectedObjectKeyframe(false, true); });
    animationTimelinePanel_->setNextKeyCallback([this]() { jumpToSelectedObjectKeyframe(true, true); });
    animationTimelinePanel_->setJumpEndCallback([this]() { setCurrentFrame(animationState_.playbackEndFrame); });
    animationTimelinePanel_->setSetKeyCallback([this]() { setKeyForSelection(true); });
    animationTimelinePanel_->setDeleteKeyCallback([this]() { deleteKeyForSelection(true); });
    animationTimelinePanel_->setDuplicateKeyCallback([this]() { duplicateCurrentKeyForSelection(true); });
    animationTimelinePanel_->setShiftKeysLeftCallback([this]() { shiftSelectedObjectKeyframes(-1, true); });
    animationTimelinePanel_->setShiftKeysRightCallback([this]() { shiftSelectedObjectKeyframes(1, true); });
    animationTimelinePanel_->setAutoKeyChangedCallback([this](bool enabled) { setAutoKeyEnabled(enabled, true); });
    animationTimelinePanel_->setViewModel(buildAnimationTimelineViewModel());
    return animationTimelinePanel_;
}

QWidget* MainWindow::createScriptEditorPanel()
{
    QWidget* panel = new QWidget(this);
    QVBoxLayout* rootLayout = new QVBoxLayout(panel);
    rootLayout->setContentsMargins(6, 6, 6, 6);
    rootLayout->setSpacing(6);

    QMenuBar* menuBar = new QMenuBar(panel);
    QMenu* fileMenu = menuBar->addMenu("File");
    fileMenu->addAction("Clear History", this, &MainWindow::clearScriptHistory);
    QMenu* editMenu = menuBar->addMenu("Edit");
    editMenu->addAction("Execute All", this, &MainWindow::executeScriptEditorAll);
    editMenu->addAction("Execute Selection", this, &MainWindow::executeScriptEditorSelection);
    menuBar->addMenu("History");
    menuBar->addMenu("Command");
    menuBar->addMenu("Tabs");
    menuBar->addMenu("Help");

    QToolBar* toolBar = new QToolBar(panel);
    toolBar->setMovable(false);
    QAction* executeAllAction = toolBar->addAction("Execute All");
    executeAllAction->setObjectName("scriptExecuteAllAction");
    QObject::connect(executeAllAction, &QAction::triggered, this, &MainWindow::executeScriptEditorAll);
    QAction* executeSelectionAction = toolBar->addAction("Execute Selection");
    executeSelectionAction->setObjectName("scriptExecuteSelectionAction");
    QObject::connect(executeSelectionAction, &QAction::triggered, this, &MainWindow::executeScriptEditorSelection);
    QAction* clearHistoryAction = toolBar->addAction("Clear History");
    clearHistoryAction->setObjectName("scriptClearHistoryAction");
    QObject::connect(clearHistoryAction, &QAction::triggered, this, &MainWindow::clearScriptHistory);

    scriptHistoryTextEdit_ = new QPlainTextEdit(panel);
    scriptHistoryTextEdit_->setObjectName("scriptHistoryTextEdit");
    scriptHistoryTextEdit_->setReadOnly(true);
    scriptHistoryTextEdit_->setPlaceholderText("Script history and command output...");
    scriptHistoryTextEdit_->setMinimumHeight(240);

    scriptInputTextEdit_ = new QPlainTextEdit(panel);
    scriptInputTextEdit_->setObjectName("scriptInputTextEdit");
    scriptInputTextEdit_->setPlaceholderText("Enter commands like:\nselect -cl;\npolyCube -w 1 -h 1 -d 1;");
    scriptInputTextEdit_->setMaximumHeight(140);

    rootLayout->setMenuBar(menuBar);
    rootLayout->addWidget(toolBar);
    rootLayout->addWidget(scriptHistoryTextEdit_, 1);
    rootLayout->addWidget(scriptInputTextEdit_);
    return panel;
}

void MainWindow::importFbx()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        "Import FBX",
        QString(),
        "FBX Files (*.fbx)");

    if (filePath.isEmpty()) {
        statusBar()->showMessage("Import cancelled", 2000);
        return;
    }

    importFbxFromPath(filePath, true);
}

bool MainWindow::saveScene()
{
    if (currentSceneFilePath_.isEmpty()) {
        return saveSceneAs();
    }

    return saveSceneToPath(currentSceneFilePath_, true);
}

bool MainWindow::saveSceneAs()
{
    const QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save Scene As",
        currentSceneFilePath_.isEmpty() ? QString("untitled.phoenixscene") : currentSceneFilePath_,
        "Phoenix Scene (*.phoenixscene)");

    if (filePath.isEmpty()) {
        statusBar()->showMessage("Save scene cancelled", 1500);
        return false;
    }

    return saveSceneToPath(filePath, true);
}

bool MainWindow::openSceneFromPath(const QString& filePath, bool logToScript)
{
    const EditorDocumentController::LoadSceneResult result = EditorDocumentController::loadScene(filePath);
    if (!result.success) {
        showDocumentOperationFailure("Open Scene", result.errorMessage, "Open scene failed", 3000);
        return false;
    }

    applyDocumentSceneLoad(result.scene, filePath, true);
    applyFileFlowResult(EditorFileFlowController::buildOpenSceneResult(filePath), logToScript, 3000);
    return true;
}

bool MainWindow::importFbxFromPath(const QString& filePath, bool logToScript)
{
    if (!viewport_->importFbx(filePath)) {
        const QString errorMessage = viewport_->lastImportMessage().isEmpty()
            ? "Unknown FBX import error."
            : viewport_->lastImportMessage();
        showDocumentOperationFailure(
            "Import FBX",
            errorMessage,
            QString("Import failed: %1").arg(QFileInfo(filePath).fileName()),
            5000);
        return false;
    }

    setCurrentFrame(animationState_.currentFrame, false);
    refreshScenePanels();
    applyFileFlowResult(EditorFileFlowController::buildImportSceneResult(filePath), logToScript, 4000);
    return true;
}

bool MainWindow::saveSceneToPath(const QString& filePath, bool logToScript)
{
    const QString targetPath = filePath.isEmpty() ? currentSceneFilePath_ : filePath;
    if (targetPath.isEmpty()) {
        showStatusMessageIfPresent("Save scene failed", 3000);
        return false;
    }

    const EditorDocumentController::OperationResult result = EditorDocumentController::saveScene(viewport_->scene(), targetPath);
    if (!result.success) {
        showDocumentOperationFailure("Save Scene", result.errorMessage, "Save scene failed", 3000);
        return false;
    }

    currentSceneFilePath_ = targetPath;
    updateWindowTitle();
    applyFileFlowResult(EditorFileFlowController::buildSaveSceneResult(currentSceneFilePath_), logToScript, 3000);
    return true;
}

bool MainWindow::incrementAndSave()
{
    currentSceneFilePath_ = EditorFileFlowController::buildIncrementSavePath(currentSceneFilePath_, QDir::currentPath());
    return saveScene();
}

bool MainWindow::archiveScene()
{
    if (!saveScene()) {
        return false;
    }

    QFileInfo info(currentSceneFilePath_);
    const QString archiveDirPath = info.dir().filePath("archive");
    QDir().mkpath(archiveDirPath);
    const QString archivePath = EditorFileFlowController::buildArchivePath(
        currentSceneFilePath_,
        QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));

    QFile::remove(archivePath);
    if (!QFile::copy(currentSceneFilePath_, archivePath)) {
        showDocumentOperationFailure(
            "Archive Scene",
            "Failed to archive current scene file.",
            "Archive scene failed",
            3000);
        return false;
    }

    applyFileFlowResult(EditorFileFlowController::buildArchiveSceneResult(archivePath), true, 3000);
    return true;
}

bool MainWindow::exportAll()
{
    const QString filePath = QFileDialog::getSaveFileName(
        this,
        "Export All",
        "export_all.phoenixscene",
        "Phoenix Scene (*.phoenixscene)");

    if (filePath.isEmpty()) {
        statusBar()->showMessage("Export cancelled", 1500);
        return false;
    }

    const EditorDocumentController::OperationResult result = EditorDocumentController::exportAll(viewport_->scene(), filePath);
    if (!result.success) {
        showDocumentOperationFailure("Export All", result.errorMessage, "Export failed", 3000);
        return false;
    }

    applyFileFlowResult(EditorFileFlowController::buildExportAllResult(filePath), true, 3000);
    return true;
}

bool MainWindow::exportSelection()
{
    if (selectedObjectId_ == 0) {
        statusBar()->showMessage("No selection to export", 2000);
        return false;
    }

    const QString filePath = QFileDialog::getSaveFileName(
        this,
        "Export Selection",
        "export_selection.phoenixscene",
        "Phoenix Scene (*.phoenixscene)");

    if (filePath.isEmpty()) {
        statusBar()->showMessage("Export selection cancelled", 1500);
        return false;
    }

    const EditorDocumentController::OperationResult result =
        EditorDocumentController::exportSelection(viewport_->scene(), selectedObjectId_, filePath);
    if (!result.success) {
        showDocumentOperationFailure("Export Selection", result.errorMessage, "Export selection failed", 3000);
        return false;
    }

    applyFileFlowResult(EditorFileFlowController::buildExportSelectionResult(filePath), true, 3000);
    return true;
}

void MainWindow::optimizeSceneStorage()
{
    viewport_->optimizeSceneStorage();
    refreshScenePanels();
    EditorScriptLogController::appendComment(scriptHistoryTextEdit_, "Scene storage optimized");
    showStatusMessageIfPresent("Scene storage optimized", 2000);
}

void MainWindow::savePreferences()
{
    writePreferencesState(EditorPreferencesState {
        saveGeometry(),
        saveState(),
        animationState_.autoKeyEnabled,
    });
    EditorScriptLogController::appendComment(scriptHistoryTextEdit_, "Saved Phoenix Editor preferences");
    showStatusMessageIfPresent("Preferences saved", 2000);
}

void MainWindow::loadPreferences()
{
    const EditorPreferencesState preferences = readPreferencesState();
    if (!preferences.geometry.isEmpty()) {
        restoreGeometry(preferences.geometry);
    }

    if (!preferences.windowState.isEmpty()) {
        restoreState(preferences.windowState);
    }

    animationState_ = EditorAnimationController::setAutoKeyEnabled(
        animationState_,
        preferences.autoKeyEnabled);
    if (viewport_ != nullptr) {
        viewport_->setAutoKeyEnabled(animationState_.autoKeyEnabled);
    }
}

void MainWindow::handlePrimitivePaletteItemActivated(QListWidgetItem* item)
{
    if (item == nullptr) {
        return;
    }

    createPrimitiveFromPalette();
}

void MainWindow::setInteractivePrimitiveCreationEnabled(bool enabled)
{
    interactivePrimitiveCreationEnabled_ = enabled;
}

void MainWindow::setExitPrimitiveToolOnCompletionEnabled(bool enabled)
{
    exitPrimitiveToolOnCompletionEnabled_ = enabled;
}

void MainWindow::createPrimitiveFromPalette()
{
    if (polygonPrimitivesList_ == nullptr || polygonPrimitivesList_->currentItem() == nullptr) {
        statusBar()->showMessage("Choose a primitive first", 1500);
        return;
    }

    const PrimitiveMeshFactory::Type type = primitiveTypeFromItem(polygonPrimitivesList_->currentItem());
    EditorCreationController::OperationResult result = EditorCreationController::createPrimitive(creationContext(), type);
    if (!result.success) {
        if (!result.errorMessage.isEmpty()) {
            statusBar()->showMessage(result.errorMessage, 2000);
        }
        return;
    }

    const SceneObject::Id objectId = viewport_->createPrimitive(type, result.objectName);
    if (objectId == 0) {
        statusBar()->showMessage("Primitive creation failed", 2000);
        return;
    }

    result.objectId = objectId;
    refreshScenePanels();
    applyCreationResult(result);

    if (interactivePrimitiveCreationEnabled_) {
        setTransformUiMode(TransformUiMode::Translate);
        viewport_->frameObject(objectId);
    }

    if (exitPrimitiveToolOnCompletionEnabled_ && polygonPrimitivesDock_ != nullptr) {
        polygonPrimitivesDock_->hide();
    }
}

void MainWindow::createJoint()
{
    EditorCreationController::OperationResult result =
        EditorCreationController::createJoint(creationContext(), selectedObjectId_);
    if (!result.success) {
        if (!result.errorMessage.isEmpty()) {
            statusBar()->showMessage(result.errorMessage, 2000);
        }
        return;
    }

    const SceneObject::Id parentId = selectedObjectId_ != 0 ? selectedObjectId_ : 0;
    const SceneObject::Id objectId = viewport_->createJoint(result.objectName, parentId);
    if (objectId == 0) {
        statusBar()->showMessage("Joint creation failed", 2000);
        return;
    }

    result.objectId = objectId;
    refreshScenePanels();
    applyCreationResult(result);
}

void MainWindow::markSelectionAsHierarchyParent()
{
    applyRiggingOperationResult(EditorRiggingController::markHierarchyParent(riggingContext(), selectedObjectId_));
}

void MainWindow::parentSelectionToMarkedParent()
{
    if (selectedObjectId_ == 0 || markedHierarchyParentId_ == 0) {
        statusBar()->showMessage("Select an object and mark a parent first", 1500);
        return;
    }

    if (selectedObjectId_ == markedHierarchyParentId_) {
        statusBar()->showMessage("Cannot parent an object to itself", 1500);
        return;
    }

    reparentObjectInUi(selectedObjectId_, markedHierarchyParentId_);
}

void MainWindow::unparentSelection()
{
    if (selectedObjectId_ == 0) {
        statusBar()->showMessage("Select an object to unparent", 1500);
        return;
    }

    const SceneObject* object = viewport_->findObject(selectedObjectId_);
    if (object == nullptr) {
        statusBar()->showMessage("Selected object is no longer available", 1500);
        return;
    }

    if (object->parentId() == 0) {
        statusBar()->showMessage("Selected object is already at root", 1500);
        return;
    }

    reparentObjectInUi(selectedObjectId_, 0);
}

void MainWindow::bindSelectedMeshToMarkedJoint()
{
    applyRiggingOperationResult(
        EditorRiggingController::bindSelectedMeshToMarkedJoint(riggingContext(), selectedObjectId_, markedHierarchyParentId_));
}

void MainWindow::resetSelectedJointOrientation()
{
    applyRiggingOperationResult(EditorRiggingController::resetSelectedJointOrientation(riggingContext(), selectedObjectId_));
}

void MainWindow::alignSelectedJointOrientationToChild()
{
    applyRiggingOperationResult(EditorRiggingController::alignSelectedJointOrientationToChild(riggingContext(), selectedObjectId_));
}

void MainWindow::captureSelectedBindPose()
{
    applyRiggingOperationResult(EditorRiggingController::captureSelectedBindPose(riggingContext(), selectedObjectId_, false));
}

void MainWindow::captureSelectedBindPoseRecursive()
{
    applyRiggingOperationResult(EditorRiggingController::captureSelectedBindPose(riggingContext(), selectedObjectId_, true));
}

bool MainWindow::reparentObjectInUi(std::uint64_t childId, std::uint64_t newParentId, bool logToScript)
{
    const EditorRiggingController::OperationResult result =
        EditorRiggingController::reparentObject(riggingContext(), childId, newParentId);
    applyRiggingOperationResult(result, logToScript);
    return result.success;
}

void MainWindow::showScriptEditorWindow()
{
    if (scriptEditorDock_ == nullptr) {
        return;
    }

    scriptEditorDock_->show();
    scriptEditorDock_->raise();
    EditorScriptLogController::appendComment(scriptHistoryTextEdit_, "Script Editor opened");
}

void MainWindow::executeScriptEditorAll()
{
    if (scriptInputTextEdit_ == nullptr) {
        return;
    }

    const EditorScriptExecutionController::ExecutionOutput output =
        EditorScriptExecutionController::executeLines(
            EditorScriptExecutionController::linesFromDocument(scriptInputTextEdit_->toPlainText()),
            scriptExecutionContext());
    for (const QString& line : output.historyLines) {
        EditorScriptLogController::appendHistoryLine(scriptHistoryTextEdit_, line);
    }
}

void MainWindow::executeScriptEditorSelection()
{
    if (scriptInputTextEdit_ == nullptr) {
        return;
    }

    const EditorScriptExecutionController::ExecutionOutput output =
        EditorScriptExecutionController::executeLines(
            EditorScriptExecutionController::linesFromSelection(scriptInputTextEdit_->textCursor().selectedText()),
            scriptExecutionContext());
    for (const QString& line : output.historyLines) {
        EditorScriptLogController::appendHistoryLine(scriptHistoryTextEdit_, line);
    }
}

void MainWindow::clearScriptHistory()
{
    EditorScriptLogController::clearHistory(scriptHistoryTextEdit_);
}

bool MainWindow::executeScriptCommand(QString commandLine, QString* resultLine)
{
    commandLine = commandLine.trimmed();
    if (commandLine.isEmpty()) {
        return false;
    }

    if (commandLine.startsWith("//")) {
        EditorScriptLogController::appendHistoryLine(scriptHistoryTextEdit_, commandLine);
        return false;
    }

    EditorScriptLogController::appendHistoryLine(scriptHistoryTextEdit_, commandLine);
    ScriptCommandExecution execution;
    const bool handled = scriptCommandRegistry_.execute(commandLine, createScriptCommandContext(), &execution);
    if (resultLine != nullptr) {
        *resultLine = execution.resultLine;
    }
    return handled;
}

ScriptCommandContext MainWindow::createScriptCommandContext()
{
    ScriptCommandContext context;
    EditorDocumentController::ScriptBindings documentBindings;
    EditorViewportCommandController::ScriptBindings viewportBindings;
    EditorAnimationScriptBindings animationBindings;
    documentBindings.newScene = [this]() {
        viewport_->clearScene();
        currentSceneFilePath_.clear();
        refreshScenePanels();
        viewport_->resetCamera();
        updateWindowTitle();
    };
    documentBindings.openSceneFile = [this](const QString& filePath) {
        return openSceneFromPath(filePath, false);
    };
    documentBindings.importSceneFile = [this](const QString& filePath) {
        return importFbxFromPath(filePath, false);
    };
    documentBindings.saveSceneFile = [this](const QString& filePath) {
        return saveSceneToPath(filePath, false);
    };
    viewportBindings.findObjectIdByName = [this](const QString& objectName) {
        return findObjectIdByName(objectName);
    };
    viewportBindings.resetCamera = [this]() {
        viewport_->resetCamera();
    };
    viewportBindings.frameScene = [this]() {
        viewport_->frameScene();
    };
    viewportBindings.frameObject = [this](std::uint64_t objectId) {
        viewport_->frameObject(objectId);
    };
    viewportBindings.setTransformMode = [this](ViewportWorkspaceWidget::TransformMode mode) {
        viewport_->setTransformMode(mode);
    };
    viewportBindings.setToolActionChecks = [this](bool translateChecked, bool rotateChecked, bool scaleChecked) {
        translateAction_->setChecked(translateChecked);
        rotateAction_->setChecked(rotateChecked);
        scaleAction_->setChecked(scaleChecked);
    };
    animationBindings.findObjectIdByName = [this](const QString& objectName) {
        return findObjectIdByName(objectName);
    };
    animationBindings.selectObjectById = [this](std::uint64_t objectId) {
        selectObject(objectId, true);
    };
    animationBindings.sceneSnapshot = [this]() {
        return viewport_->sceneSnapshot();
    };
    animationBindings.animationState = [this]() {
        return animationState_;
    };
    animationBindings.applySceneMutation = [this](const Scene& scene, std::uint64_t objectId, int currentFrame) {
        recordUndoState();
        replaceSceneAndRestoreSelection(scene, objectId, true);
        applyAnimationState(EditorAnimationController::setCurrentFrame(animationState_, currentFrame));
    };
    animationBindings.applyAnimationState = [this](const EditorAnimationState& state) {
        applyAnimationState(state);
    };

    EditorSelectionController::Context selection = selectionContext();
    EditorSelectionController::bindScriptCommands(
        context,
        selection,
        selectedObjectId_,
        markedHierarchyParentId_,
        [this](const QString& objectName) {
            return findObjectIdByName(objectName);
        });
    EditorCreationController::bindScriptCommands(context, createCreationScriptBindings());
    EditorSceneMutationController::bindScriptCommands(context, createSceneScriptBindings());
    EditorDocumentController::bindScriptCommands(context, documentBindings);
    EditorViewportCommandController::bindScriptCommands(context, viewportBindings);
    EditorAnimationController::bindScriptCommands(context, animationBindings);
    return context;
}

EditorCreationController::ScriptBindings MainWindow::createCreationScriptBindings()
{
    EditorCreationController::ScriptBindings bindings;
    bindings.generateUniqueScriptName = [this](const QString& prefix) {
        return generateUniqueScriptName(prefix);
    };
    bindings.generateUniqueObjectName = [this](const QString& baseName, std::uint64_t ignoreObjectId) {
        return generateUniqueObjectName(baseName, ignoreObjectId);
    };
    bindings.selectedObjectId = [this]() {
        return selectedObjectId_;
    };
    bindings.createPrimitive = [this](PrimitiveMeshFactory::Type type, const QString& objectName) {
        return viewport_->createPrimitive(type, objectName);
    };
    bindings.createJoint = [this](const QString& objectName, std::uint64_t parentId) {
        return viewport_->createJoint(objectName, parentId);
    };
    bindings.applyLiveMutation = [this](std::uint64_t objectId, bool clearSelectionAfter) {
        refreshScenePanels();
        if (clearSelectionAfter || objectId == 0) {
            clearInspector();
        } else {
            selectObject(objectId, true);
        }
    };
    return bindings;
}

EditorSceneMutationController::ScriptBindings MainWindow::createSceneScriptBindings()
{
    EditorSceneMutationController::ScriptBindings bindings;
    bindings.findObjectIdByName = [this](const QString& objectName) {
        return findObjectIdByName(objectName);
    };
    bindings.generateUniqueObjectName = [this](const QString& baseName, std::uint64_t ignoreObjectId) {
        return generateUniqueObjectName(baseName, ignoreObjectId);
    };
    bindings.findObject = [this](std::uint64_t objectId) {
        return viewport_->findObject(objectId);
    };
    bindings.sceneSnapshot = [this]() {
        return viewport_->sceneSnapshot();
    };
    bindings.setObjectVisibility = [this](std::uint64_t objectId, bool visible) {
        return viewport_->setObjectVisibility(objectId, visible);
    };
    bindings.setObjectLocalTransform = [this](std::uint64_t objectId, const Transform& transform) {
        return viewport_->setObjectLocalTransform(objectId, transform);
    };
    bindings.setJointOrientation = [this](std::uint64_t objectId, const QQuaternion& orientation) {
        return viewport_->setJointOrientation(objectId, orientation);
    };
    bindings.resetJointOrientation = [this](std::uint64_t objectId) {
        return viewport_->resetJointOrientation(objectId);
    };
    bindings.alignJointOrientationToChild = [this](std::uint64_t objectId) {
        return viewport_->alignJointOrientationToChild(objectId);
    };
    bindings.captureBindPose = [this](std::uint64_t objectId, bool recursive) {
        return viewport_->captureBindPose(objectId, recursive);
    };
    bindings.applySceneMutation = [this](const Scene& scene, std::uint64_t objectId, bool clearSelectionAfter) {
        recordUndoState();
        viewport_->replaceScene(scene);
        refreshScenePanels();
        if (clearSelectionAfter) {
            clearInspector();
            return;
        }

        restoreSelectionAfterSceneRefresh(objectId, true);
    };
    bindings.applyLiveMutation = [this](std::uint64_t objectId, bool clearSelectionAfter) {
        refreshScenePanels();
        if (clearSelectionAfter) {
            clearInspector();
            return;
        }

        restoreSelectionAfterSceneRefresh(objectId, true);
    };
    return bindings;
}

std::uint64_t MainWindow::findObjectIdByName(const QString& objectName) const
{
    return EditorSceneQueryController::findObjectIdByName(*viewport_, objectName);
}

QString MainWindow::generateUniqueScriptName(const QString& prefix) const
{
    return EditorSceneQueryController::generateUniqueScriptName(*viewport_, prefix);
}

QString MainWindow::generateUniqueObjectName(const QString& baseName, std::uint64_t ignoreObjectId) const
{
    return EditorSceneQueryController::generateUniqueObjectName(*viewport_, baseName, ignoreObjectId);
}

void MainWindow::refreshScenePanels()
{
    if (markedHierarchyParentId_ != 0 && !viewport_->containsObject(markedHierarchyParentId_)) {
        markedHierarchyParentId_ = 0;
    }
    populateOutliner();
    clearInspector();
}

void MainWindow::restoreSelectionAfterSceneRefresh(std::uint64_t objectId, bool syncOutliner)
{
    if (objectId != 0 && viewport_->containsObject(objectId)) {
        selectObject(objectId, syncOutliner);
    } else {
        clearInspector();
    }
}

void MainWindow::replaceSceneAndRestoreSelection(const Scene& scene, std::uint64_t objectId, bool syncOutliner)
{
    viewport_->replaceScene(scene);
    refreshScenePanels();
    restoreSelectionAfterSceneRefresh(objectId, syncOutliner);
}

void MainWindow::restoreHistoryState(const EditorHistoryState& state)
{
    restoringHistory_ = true;
    viewport_->replaceScene(state.scene);
    applyAnimationState(EditorAnimationController::setCurrentFrame(animationState_, state.currentFrame), false);
    markedHierarchyParentId_ = state.markedHierarchyParentId;
    refreshScenePanels();
    restoreSelectionAfterSceneRefresh(state.selectedObjectId, true);
    restoringHistory_ = false;
    updateUndoRedoActions();
}

void MainWindow::recordUndoState()
{
    historyController_.recordUndoState(
        historyController_.captureState(viewport_->scene(), selectedObjectId_, markedHierarchyParentId_, animationState_.currentFrame));
    updateUndoRedoActions();
}

void MainWindow::undoLastChange()
{
    EditorHistoryState previousState;
    if (!historyController_.tryTakeUndoState(
            historyController_.captureState(viewport_->scene(), selectedObjectId_, markedHierarchyParentId_, animationState_.currentFrame),
            &previousState)) {
        return;
    }

    restoreHistoryState(previousState);
}

void MainWindow::redoLastChange()
{
    EditorHistoryState nextState;
    if (!historyController_.tryTakeRedoState(
            historyController_.captureState(viewport_->scene(), selectedObjectId_, markedHierarchyParentId_, animationState_.currentFrame),
            &nextState)) {
        return;
    }

    restoreHistoryState(nextState);
}

void MainWindow::updateUndoRedoActions()
{
    if (undoAction_ != nullptr) {
        undoAction_->setEnabled(historyController_.canUndo());
    }
    if (redoAction_ != nullptr) {
        redoAction_->setEnabled(historyController_.canRedo());
    }
}

void MainWindow::populateOutliner()
{
    EditorOutlinerController::populateTree(outlinerTree_, outlinerSceneAccess());
}

void MainWindow::clearInspector()
{
    updatingChannelBox_ = true;
    EditorSelectionController::Context context = selectionContext();
    EditorSelectionController::clearSelection(context, selectedObjectId_, markedHierarchyParentId_);
    updatingChannelBox_ = false;
    refreshAnimationTimelineUi();
}

void MainWindow::updateInspector(std::uint64_t objectId)
{
    updatingChannelBox_ = true;
    EditorSelectionController::Context context = selectionContext();
    EditorSelectionController::selectObject(context, objectId, false, selectedObjectId_, markedHierarchyParentId_);
    updatingChannelBox_ = false;
    refreshAnimationTimelineUi();
}

void MainWindow::handleOutlinerSelectionChanged()
{
    const std::uint64_t objectId = EditorOutlinerController::selectedObjectId(outlinerTree_);
    if (objectId == 0) {
        clearInspector();
        EditorScriptLogController::logSelectionCleared(scriptHistoryTextEdit_);
        return;
    }

    selectObject(objectId, false);
    EditorScriptLogController::logSelection(scriptHistoryTextEdit_, viewport_->findObject(objectId));
    statusBar()->showMessage(QString("Selected: %1").arg(EditorOutlinerController::selectedObjectLabel(outlinerTree_)), 2000);
}

void MainWindow::frameSelectedObject()
{
    if (EditorSelectionController::frameSelectedObject(selectionContext(), selectedObjectId_)) {
        statusBar()->showMessage("Selected object framed", 2000);
    }
}

void MainWindow::resetSceneCamera()
{
    viewport_->resetCamera();
    EditorScriptLogController::appendHistoryLine(scriptHistoryTextEdit_, "viewSet -home;");
    EditorScriptLogController::appendHistoryLine(scriptHistoryTextEdit_, "// Result: camera reset //");
    showStatusMessageIfPresent("Camera reset", 2000);
}

void MainWindow::frameEntireScene()
{
    viewport_->frameScene();
    EditorScriptLogController::appendHistoryLine(scriptHistoryTextEdit_, "viewFit;");
    EditorScriptLogController::appendHistoryLine(scriptHistoryTextEdit_, "// Result: scene framed //");
    showStatusMessageIfPresent("Scene framed", 2000);
}

void MainWindow::setWireframeDisplayEnabled(bool enabled)
{
    viewport_->setWireframeEnabled(enabled);
    EditorScriptLogController::appendComment(scriptHistoryTextEdit_, QString("Wireframe %1").arg(enabled ? "on" : "off"));
    showStatusMessageIfPresent(enabled ? "Wireframe on" : "Wireframe off", 2000);
}

void MainWindow::setAxisVisibilityEnabled(bool enabled)
{
    viewport_->setAxisVisible(enabled);
    EditorScriptLogController::appendComment(scriptHistoryTextEdit_, QString("Axis visibility %1").arg(enabled ? "on" : "off"));
    showStatusMessageIfPresent(enabled ? "Axis visible" : "Axis hidden", 2000);
}

void MainWindow::setBackfaceCullingEnabled(bool enabled)
{
    viewport_->setBackfaceCullingEnabled(enabled);
    EditorScriptLogController::appendComment(scriptHistoryTextEdit_, QString("Backface culling %1").arg(enabled ? "on" : "off"));
    showStatusMessageIfPresent(enabled ? "Backface culling on" : "Backface culling off", 2000);
}

void MainWindow::selectObject(std::uint64_t objectId, bool syncOutliner)
{
    updatingChannelBox_ = true;
    EditorSelectionController::Context context = selectionContext();
    EditorSelectionController::selectObject(context, objectId, syncOutliner, selectedObjectId_, markedHierarchyParentId_);
    updatingChannelBox_ = false;
    refreshAnimationTimelineUi();
}

EditorAnimationTimelineViewModel MainWindow::buildAnimationTimelineViewModel() const
{
    return EditorAnimationController::buildTimelineViewModel(viewport_->scene(), selectedObjectId_, animationState_);
}

void MainWindow::setTransformUiMode(TransformUiMode mode)
{
    if (mode == TransformUiMode::Translate) {
        EditorViewportUiController::applyTransformModeChecks(
            viewportUiActions(),
            EditorViewportUiController::TransformMode::Translate);
        viewport_->setTransformMode(ViewportWidget::TransformMode::Translate);
        applyViewportUiOperationResult(
            EditorViewportUiController::buildTransformModeResult(EditorViewportUiController::TransformMode::Translate));
    } else if (mode == TransformUiMode::Rotate) {
        EditorViewportUiController::applyTransformModeChecks(
            viewportUiActions(),
            EditorViewportUiController::TransformMode::Rotate);
        viewport_->setTransformMode(ViewportWidget::TransformMode::Rotate);
        applyViewportUiOperationResult(
            EditorViewportUiController::buildTransformModeResult(EditorViewportUiController::TransformMode::Rotate));
    } else {
        EditorViewportUiController::applyTransformModeChecks(
            viewportUiActions(),
            EditorViewportUiController::TransformMode::Scale);
        viewport_->setTransformMode(ViewportWidget::TransformMode::Scale);
        applyViewportUiOperationResult(
            EditorViewportUiController::buildTransformModeResult(EditorViewportUiController::TransformMode::Scale));
    }
}

void MainWindow::setViewCameraPreset(ViewCameraUiPreset preset)
{
    if (perspectiveCameraAction_ == nullptr
            || frontCameraAction_ == nullptr
            || backCameraAction_ == nullptr
            || leftCameraAction_ == nullptr
            || rightCameraAction_ == nullptr
            || topCameraAction_ == nullptr
            || bottomCameraAction_ == nullptr) {
        return;
    }

    ViewportWidget::CameraViewPreset viewportPreset = ViewportWidget::CameraViewPreset::Perspective;
    EditorViewportUiController::CameraPreset controllerPreset = EditorViewportUiController::CameraPreset::Perspective;
    switch (preset) {
    case ViewCameraUiPreset::Front:
        controllerPreset = EditorViewportUiController::CameraPreset::Front;
        viewportPreset = ViewportWidget::CameraViewPreset::Front;
        break;
    case ViewCameraUiPreset::Back:
        controllerPreset = EditorViewportUiController::CameraPreset::Back;
        viewportPreset = ViewportWidget::CameraViewPreset::Back;
        break;
    case ViewCameraUiPreset::Left:
        controllerPreset = EditorViewportUiController::CameraPreset::Left;
        viewportPreset = ViewportWidget::CameraViewPreset::Left;
        break;
    case ViewCameraUiPreset::Right:
        controllerPreset = EditorViewportUiController::CameraPreset::Right;
        viewportPreset = ViewportWidget::CameraViewPreset::Right;
        break;
    case ViewCameraUiPreset::Top:
        controllerPreset = EditorViewportUiController::CameraPreset::Top;
        viewportPreset = ViewportWidget::CameraViewPreset::Top;
        break;
    case ViewCameraUiPreset::Bottom:
        controllerPreset = EditorViewportUiController::CameraPreset::Bottom;
        viewportPreset = ViewportWidget::CameraViewPreset::Bottom;
        break;
    case ViewCameraUiPreset::Perspective:
        break;
    }

    EditorViewportUiController::applyCameraPresetChecks(viewportUiActions(), controllerPreset);
    viewport_->setCameraViewPreset(viewportPreset);
    applyViewportUiOperationResult(EditorViewportUiController::buildCameraPresetResult(controllerPreset));
}

void MainWindow::setAxisUiOrientation(AxisUiOrientation orientation)
{
    if (orientation == AxisUiOrientation::World) {
        EditorViewportUiController::applyAxisOrientationChecks(
            viewportUiActions(),
            EditorViewportUiController::AxisOrientation::World);
        viewport_->setAxisOrientation(ViewportWidget::AxisOrientation::World);
        applyViewportUiOperationResult(
            EditorViewportUiController::buildAxisOrientationResult(EditorViewportUiController::AxisOrientation::World));
    } else {
        EditorViewportUiController::applyAxisOrientationChecks(
            viewportUiActions(),
            EditorViewportUiController::AxisOrientation::Local);
        viewport_->setAxisOrientation(ViewportWidget::AxisOrientation::Local);
        applyViewportUiOperationResult(
            EditorViewportUiController::buildAxisOrientationResult(EditorViewportUiController::AxisOrientation::Local));
    }
}

void MainWindow::restoreDefaultWorkspaceLayout()
{
    if (outlinerDock_ == nullptr || inspectorDock_ == nullptr || timeSliderDock_ == nullptr) {
        return;
    }

    outlinerDock_->setFloating(false);
    inspectorDock_->setFloating(false);
    timeSliderDock_->setFloating(false);

    addDockWidget(Qt::LeftDockWidgetArea, outlinerDock_);
    addDockWidget(Qt::RightDockWidgetArea, inspectorDock_);
    addDockWidget(Qt::BottomDockWidgetArea, timeSliderDock_);
    resizeDocks({ outlinerDock_, inspectorDock_ }, { 280, 320 }, Qt::Horizontal);
    resizeDocks({ timeSliderDock_ }, { 150 }, Qt::Vertical);

    outlinerDock_->show();
    inspectorDock_->show();
    timeSliderDock_->show();
    applyViewportUiOperationResult(EditorViewportUiController::buildWorkspaceLayoutResult(), 2000);
}

void MainWindow::showPolygonPrimitivesWindow()
{
    if (polygonPrimitivesDock_ == nullptr) {
        return;
    }

    polygonPrimitivesDock_->show();
    polygonPrimitivesDock_->raise();
    applyViewportUiOperationResult(EditorViewportUiController::buildPolygonPrimitivesWindowResult(), 0);
}

void MainWindow::updateWindowTitle()
{
    const QString sceneName = currentSceneFilePath_.isEmpty()
        ? "untitled"
        : QFileInfo(currentSceneFilePath_).fileName();
    setWindowTitle(QString("%1 - Phoenix Editor Beta").arg(sceneName));
}

void MainWindow::applyDocumentSceneLoad(const Scene& scene, const QString& filePath, bool frameScene)
{
    recordUndoState();
    viewport_->replaceScene(scene);
    setCurrentFrame(viewport_->currentFrame(), false);
    currentSceneFilePath_ = filePath;
    refreshScenePanels();
    if (frameScene) {
        viewport_->frameScene();
    }
    updateWindowTitle();
}

bool MainWindow::showDocumentOperationFailure(
    const QString& dialogTitle,
    const QString& errorMessage,
    const QString& statusMessage,
    int timeoutMs)
{
    if (!errorMessage.isEmpty()) {
        QMessageBox::warning(this, dialogTitle, errorMessage);
    }
    showStatusMessageIfPresent(statusMessage, timeoutMs);
    return false;
}

void MainWindow::applyAnimationState(const EditorAnimationState& state, bool logToScript)
{
    animationState_ = state;

    if (viewport_ != nullptr) {
        viewport_->setAutoKeyEnabled(animationState_.autoKeyEnabled);
        viewport_->setCurrentFrame(animationState_.currentFrame);
    }

    updatingTimeSlider_ = true;
    if (animationTimelinePanel_ != nullptr) {
        animationTimelinePanel_->setViewModel(buildAnimationTimelineViewModel());
    }
    updatingTimeSlider_ = false;

    syncPlaybackTimer();
    refreshAnimationTimelineUi();

    if (selectedObjectId_ != 0 && viewport_ != nullptr && viewport_->containsObject(selectedObjectId_)) {
        updateChannelBox(selectedObjectId_);
        const SceneObject* object = viewport_->findObject(selectedObjectId_);
        const bool canFrame = object != nullptr && object->isVisible() && object->worldBounds().isValid();
        frameSelectedButton_->setEnabled(canFrame);
        frameSelectedAction_->setEnabled(canFrame);
    }

    if (logToScript) {
        EditorScriptLogController::appendHistoryLine(scriptHistoryTextEdit_, QString("currentTime %1;").arg(animationState_.currentFrame));
        EditorScriptLogController::appendHistoryLine(scriptHistoryTextEdit_, QString("// Result: current frame %1 //").arg(animationState_.currentFrame));
    }
}

void MainWindow::syncPlaybackTimer()
{
    if (playbackTimer_ == nullptr) {
        return;
    }

    if (animationState_.playing) {
        playbackTimer_->start();
    } else {
        playbackTimer_->stop();
    }
}

void MainWindow::setCurrentFrame(int frame, bool logToScript)
{
    applyAnimationFlowResult(EditorAnimationFlowController::setCurrentFrame(animationState_, frame, logToScript));
}

void MainWindow::setKeyForSelection(bool logToScript)
{
    applyAnimationFlowResult(
        EditorAnimationFlowController::setKeyForSelection(animationFlowContext(), animationState_, selectedObjectId_, logToScript));
}

void MainWindow::deleteKeyForSelection(bool logToScript)
{
    applyAnimationFlowResult(
        EditorAnimationFlowController::deleteKeyForSelection(animationFlowContext(), animationState_, selectedObjectId_, logToScript));
}

void MainWindow::duplicateCurrentKeyForSelection(bool logToScript)
{
    applyAnimationFlowResult(
        EditorAnimationFlowController::duplicateCurrentKeyForSelection(animationFlowContext(), animationState_, selectedObjectId_, logToScript));
}

void MainWindow::shiftSelectedObjectKeyframes(int frameDelta, bool logToScript)
{
    applyAnimationFlowResult(
        EditorAnimationFlowController::shiftSelectedObjectKeyframes(animationFlowContext(), animationState_, selectedObjectId_, frameDelta, logToScript));
}

void MainWindow::setAutoKeyEnabled(bool enabled, bool logToScript)
{
    applyAnimationFlowResult(EditorAnimationFlowController::setAutoKeyEnabled(animationState_, enabled, logToScript));
}

void MainWindow::setPlaybackRange(int startFrame, int endFrame, bool logToScript)
{
    applyAnimationFlowResult(EditorAnimationFlowController::setPlaybackRange(animationState_, startFrame, endFrame, logToScript));
}

void MainWindow::stepFrame(int delta)
{
    applyAnimationFlowResult(EditorAnimationFlowController::stepFrame(animationState_, delta));
}

void MainWindow::jumpToSelectedObjectKeyframe(bool forward, bool logToScript)
{
    applyAnimationFlowResult(
        EditorAnimationFlowController::jumpToSelectedObjectKeyframe(animationFlowContext(), animationState_, selectedObjectId_, forward, logToScript));
}

void MainWindow::togglePlayback()
{
    applyAnimationFlowResult(EditorAnimationFlowController::togglePlayback(animationState_));
}

void MainWindow::advancePlayback()
{
    applyAnimationFlowResult(EditorAnimationFlowController::advancePlayback(animationState_));
}

PrimitiveMeshFactory::Type MainWindow::primitiveTypeFromItem(const QListWidgetItem* item) const
{
    if (item == nullptr) {
        return PrimitiveMeshFactory::Type::Cube;
    }

    return static_cast<PrimitiveMeshFactory::Type>(item->data(kPrimitiveTypeRole).toInt());
}

void MainWindow::updateChannelBox(std::uint64_t objectId)
{
    const SceneObject* object = viewport_->findObject(objectId);
    if (object == nullptr) {
        return;
    }

    updatingChannelBox_ = true;
    EditorInspectorController::populateSelectionUi(inspectorWidgets(), *object);
    updatingChannelBox_ = false;
    refreshAnimationTimelineUi();
}

void MainWindow::refreshAnimationTimelineUi()
{
    if (animationTimelinePanel_ != nullptr) {
        animationTimelinePanel_->setViewModel(buildAnimationTimelineViewModel());
    }
}

void MainWindow::setChannelBoxEnabled(bool enabled)
{
    EditorInspectorController::setInspectorEnabled(inspectorWidgets(), enabled);
}

EditorInspectorController::InspectorWidgets MainWindow::inspectorWidgets() const
{
    return MainWindowContexts::buildInspectorWidgets(
        inspectorEmptyStateLabel_,
        channelObjectNameLabel_,
        translateXSpinBox_,
        translateYSpinBox_,
        translateZSpinBox_,
        rotateXSpinBox_,
        rotateYSpinBox_,
        rotateZSpinBox_,
        scaleXSpinBox_,
        scaleYSpinBox_,
        scaleZSpinBox_,
        jointOrientXSpinBox_,
        jointOrientYSpinBox_,
        jointOrientZSpinBox_,
        bindPoseStatusLabel_,
        skinBindingStatusLabel_,
        visibilityCheckBox_,
        inspectorDetailsWidget_,
        jointToolsWidget_);
}

EditorInspectorController::InspectorActions MainWindow::inspectorActions() const
{
    return MainWindowContexts::buildInspectorActions(
        markHierarchyParentAction_,
        parentToMarkedParentAction_,
        unparentSelectedAction_,
        bindSkinAction_,
        resetJointOrientationAction_,
        alignJointOrientationAction_,
        captureBindPoseAction_,
        captureBindPoseRecursiveAction_,
        frameSelectedAction_,
        frameSelectedButton_,
        resetJointOrientationButton_,
        alignJointOrientationButton_,
        captureBindPoseButton_,
        captureBindPoseRecursiveButton_);
}

EditorChannelBoxController::Context MainWindow::channelBoxContext() const
{
    return MainWindowContexts::buildChannelBoxContext(viewport_);
}

EditorCreationController::Context MainWindow::creationContext() const
{
    return MainWindowContexts::buildCreationContext(
        [this](const QString& prefix) {
            return generateUniqueScriptName(prefix);
        },
        [this](const QString& baseName, std::uint64_t ignoreObjectId) {
            return generateUniqueObjectName(baseName, ignoreObjectId);
        });
}

EditorOutlinerController::SceneAccess MainWindow::outlinerSceneAccess() const
{
    return MainWindowContexts::buildOutlinerSceneAccess(viewport_);
}

EditorRiggingController::Context MainWindow::riggingContext() const
{
    return MainWindowContexts::buildRiggingContext(viewport_);
}

EditorScriptExecutionController::ExecutionContext MainWindow::scriptExecutionContext()
{
    EditorScriptExecutionController::ExecutionContext context;
    context.createScriptCommandContext = [this]() {
        return createScriptCommandContext();
    };
    context.executeCommand = [this](QString commandLine, const ScriptCommandContext& scriptContext, ScriptCommandExecution* execution) {
        return scriptCommandRegistry_.execute(commandLine, scriptContext, execution);
    };
    return context;
}

EditorSelectionController::Context MainWindow::selectionContext() const
{
    return MainWindowContexts::buildSelectionContext(
        viewport_,
        outlinerTree_,
        scriptHistoryTextEdit_,
        inspectorWidgets(),
        inspectorActions(),
        outlinerSceneAccess());
}

EditorViewportUiController::ActionSet MainWindow::viewportUiActions() const
{
    return MainWindowContexts::buildViewportUiActions(
        translateAction_,
        rotateAction_,
        scaleAction_,
        perspectiveCameraAction_,
        frontCameraAction_,
        backCameraAction_,
        leftCameraAction_,
        rightCameraAction_,
        topCameraAction_,
        bottomCameraAction_,
        worldAxisAction_,
        localAxisAction_);
}

bool MainWindow::showErrorMessageIfPresent(const QString& errorMessage, int timeoutMs)
{
    if (errorMessage.isEmpty()) {
        return false;
    }

    statusBar()->showMessage(errorMessage, timeoutMs);
    return true;
}

void MainWindow::appendScriptResultLogLines(const QString& commentLine, const QString& commandLine, const QString& resultLine)
{
    if (!commentLine.isEmpty()) {
        EditorScriptLogController::appendComment(scriptHistoryTextEdit_, commentLine);
    }
    if (!commandLine.isEmpty()) {
        EditorScriptLogController::appendHistoryLine(scriptHistoryTextEdit_, commandLine);
    }
    if (!resultLine.isEmpty()) {
        EditorScriptLogController::appendHistoryLine(scriptHistoryTextEdit_, resultLine);
    }
}

void MainWindow::showStatusMessageIfPresent(const QString& statusMessage, int timeoutMs)
{
    if (!statusMessage.isEmpty()) {
        statusBar()->showMessage(statusMessage, timeoutMs);
    }
}

void MainWindow::applyRiggingOperationResult(const EditorRiggingController::OperationResult& result, bool logToScript)
{
    if (!result.success) {
        showErrorMessageIfPresent(result.errorMessage, 1500);
        return;
    }

    if (result.markedHierarchyParentId != 0) {
        markedHierarchyParentId_ = result.markedHierarchyParentId;
    }

    if (result.hasScene) {
        recordUndoState();
        replaceSceneAndRestoreSelection(result.scene, result.focusObjectId, true);
    } else if (result.focusObjectId != 0) {
        updateInspector(result.focusObjectId);
    }

    if (logToScript) {
        appendScriptResultLogLines(result.commentLine, result.commandLine, result.resultLine);
    }

    showStatusMessageIfPresent(result.statusMessage, 2000);
}

void MainWindow::applyFileFlowResult(const EditorFileFlowController::OperationResult& result, bool logToScript, int timeoutMs)
{
    if (!result.success) {
        return;
    }

    if (!result.targetPath.isEmpty()) {
        currentSceneFilePath_ = result.targetPath;
    }

    if (logToScript) {
        appendScriptResultLogLines(result.commentLine, result.commandLine, result.resultLine);
    }

    showStatusMessageIfPresent(result.statusMessage, timeoutMs);
}

EditorAnimationFlowController::Context MainWindow::animationFlowContext() const
{
    return MainWindowContexts::buildAnimationFlowContext(viewport_);
}

void MainWindow::applyAnimationFlowResult(const EditorAnimationFlowController::OperationResult& result)
{
    if (!result.success) {
        showErrorMessageIfPresent(result.errorMessage, 1500);
        return;
    }

    if (result.sceneChanged) {
        recordUndoState();
        replaceSceneAndRestoreSelection(result.scene, result.focusObjectId, true);
    }

    applyAnimationState(result.state, false);

    appendScriptResultLogLines(result.commentLine, result.commandLine, result.resultLine);
    showStatusMessageIfPresent(result.statusMessage, 1500);
}

void MainWindow::applyCreationResult(const EditorCreationController::OperationResult& result)
{
    if (!result.success) {
        showErrorMessageIfPresent(result.errorMessage, 1500);
        return;
    }

    if (result.objectId != 0) {
        selectObject(result.objectId, true);
    }

    appendScriptResultLogLines(QString(), result.commandLine, result.resultLine);
    if (result.commandLine.isEmpty() && result.resultLine.isEmpty() && !result.objectName.isEmpty()) {
        EditorScriptLogController::logPrimitiveCreation(scriptHistoryTextEdit_, result.primitiveType, result.objectName);
    }

    showStatusMessageIfPresent(result.statusMessage, 2000);
}

void MainWindow::applyChannelBoxOperationResult(const EditorChannelBoxController::OperationResult& result)
{
    if (!result.success) {
        showErrorMessageIfPresent(result.errorMessage, 1200);
        return;
    }

    updateChannelBox(result.objectId);
    const SceneObject* object = viewport_->findObject(result.objectId);
    if (result.visibilityChanged) {
        visibilityCheckBox_->setText(result.visible ? "on" : "off");
        const bool canFrame = object != nullptr && object->isVisible() && object->worldBounds().isValid();
        frameSelectedButton_->setEnabled(canFrame);
        frameSelectedAction_->setEnabled(canFrame);
        EditorScriptLogController::logVisibilityChange(scriptHistoryTextEdit_, object, result.visible);
    } else if (!result.commandLine.isEmpty() || !result.resultLine.isEmpty()) {
        appendScriptResultLogLines(QString(), result.commandLine, result.resultLine);
    } else if (object != nullptr) {
        EditorScriptLogController::logTransformChange(scriptHistoryTextEdit_, object, result.transform);
    }

    showStatusMessageIfPresent(result.statusMessage, 1200);
}

void MainWindow::applyViewportUiOperationResult(const EditorViewportUiController::OperationResult& result, int timeoutMs)
{
    if (!result.success) {
        return;
    }

    appendScriptResultLogLines(result.commentLine, result.commandLine, result.resultLine);
    showStatusMessageIfPresent(result.statusMessage, timeoutMs);
}

void MainWindow::applyChannelBoxToSelection()
{
    if (updatingChannelBox_ || selectedObjectId_ == 0) {
        return;
    }

    EditorChannelBoxController::TransformInput input;
    input.translation = QVector3D(
        static_cast<float>(translateXSpinBox_->value()),
        static_cast<float>(translateYSpinBox_->value()),
        static_cast<float>(translateZSpinBox_->value()));
    input.rotationEulerDegrees = QVector3D(
        static_cast<float>(rotateXSpinBox_->value()),
        static_cast<float>(rotateYSpinBox_->value()),
        static_cast<float>(rotateZSpinBox_->value()));
    input.scale = QVector3D(
        static_cast<float>(scaleXSpinBox_->value()),
        static_cast<float>(scaleYSpinBox_->value()),
        static_cast<float>(scaleZSpinBox_->value()));

    applyChannelBoxOperationResult(
        EditorChannelBoxController::applyTransform(channelBoxContext(), selectedObjectId_, input));
}

void MainWindow::applyJointOrientationToSelection()
{
    if (updatingChannelBox_ || selectedObjectId_ == 0) {
        return;
    }

    const QVector3D eulerDegrees(
        static_cast<float>(jointOrientXSpinBox_->value()),
        static_cast<float>(jointOrientYSpinBox_->value()),
        static_cast<float>(jointOrientZSpinBox_->value()));

    applyChannelBoxOperationResult(
        EditorChannelBoxController::applyJointOrientation(channelBoxContext(), selectedObjectId_, eulerDegrees));
}

void MainWindow::applyVisibilityToSelection(bool visible)
{
    if (updatingChannelBox_ || selectedObjectId_ == 0) {
        return;
    }

    applyChannelBoxOperationResult(
        EditorChannelBoxController::applyVisibility(channelBoxContext(), selectedObjectId_, visible));
}
