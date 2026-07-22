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
#include <QTextCursor>
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

#include "EditorDocumentController.h"
#include "KeyframeTimelineWidget.h"
#include "EditorSceneMutationController.h"
#include "ViewportWorkspaceWidget.h"
#include "io/PhoenixSceneDocument.h"
#include "logging/LogCategories.h"
#include "scene/PrimitiveMeshFactory.h"
#include "scene/Scene.h"
#include "scene/SceneObject.h"

namespace
{
constexpr auto kPrimitiveTypeRole = Qt::UserRole + 101;

QString formatVector3(const QVector3D& value)
{
    return QString("(%1, %2, %3)")
        .arg(value.x(), 0, 'f', 2)
        .arg(value.y(), 0, 'f', 2)
        .arg(value.z(), 0, 'f', 2);
}

QString formatBounds(const Bounds3D& bounds)
{
    if (!bounds.isValid()) {
        return "Invalid";
    }

    return QString("min %1 | max %2")
        .arg(formatVector3(bounds.min()))
        .arg(formatVector3(bounds.max()));
}

QString formatTransform(const Transform& transform)
{
    return QString("T %1 | R (%2, %3, %4, %5) | S %6")
        .arg(formatVector3(transform.translation))
        .arg(transform.rotation.scalar(), 0, 'f', 2)
        .arg(transform.rotation.x(), 0, 'f', 2)
        .arg(transform.rotation.y(), 0, 'f', 2)
        .arg(transform.rotation.z(), 0, 'f', 2)
        .arg(formatVector3(transform.scale));
}

QString formatBindPoseStatus(const SceneObject& object)
{
    if (!object.isJoint()) {
        return "Bind pose: n/a";
    }

    return object.hasBindPose()
        ? QString("Bind pose captured: %1").arg(formatTransform(object.bindPoseLocalTransform()))
        : "Bind pose: not captured";
}

QString formatSkinBindingStatus(const SceneObject& object)
{
    if (object.meshHandles().isEmpty()) {
        return "Skin binding: n/a";
    }

    return object.hasSkinBinding()
        ? QString("Skin binding: %1 joints, %2 weighted vertices")
              .arg(object.skinJointIds().size())
              .arg(object.skinWeights().size())
        : "Skin binding: not bound";
}

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

QString objectDisplayName(const SceneObject& object)
{
    return object.name().isEmpty() ? QString("Object_%1").arg(object.id()) : object.name();
}

QString mayaCommandName(PrimitiveMeshFactory::Type type)
{
    switch (type) {
    case PrimitiveMeshFactory::Type::Sphere: return "polySphere";
    case PrimitiveMeshFactory::Type::Cube: return "polyCube";
    case PrimitiveMeshFactory::Type::Cylinder: return "polyCylinder";
    case PrimitiveMeshFactory::Type::Cone: return "polyCone";
    case PrimitiveMeshFactory::Type::Torus: return "polyTorus";
    case PrimitiveMeshFactory::Type::Plane: return "polyPlane";
    case PrimitiveMeshFactory::Type::Disc: return "polyDisc";
    case PrimitiveMeshFactory::Type::Pyramid: return "polyPyramid";
    case PrimitiveMeshFactory::Type::Prism: return "polyPrism";
    }

    return "polyPrimitive";
}

QString primitivePrefix(PrimitiveMeshFactory::Type type)
{
    switch (type) {
    case PrimitiveMeshFactory::Type::Sphere: return "pSphere";
    case PrimitiveMeshFactory::Type::Cube: return "pCube";
    case PrimitiveMeshFactory::Type::Cylinder: return "pCylinder";
    case PrimitiveMeshFactory::Type::Cone: return "pCone";
    case PrimitiveMeshFactory::Type::Torus: return "pTorus";
    case PrimitiveMeshFactory::Type::Plane: return "pPlane";
    case PrimitiveMeshFactory::Type::Disc: return "pDisc";
    case PrimitiveMeshFactory::Type::Pyramid: return "pPyramid";
    case PrimitiveMeshFactory::Type::Prism: return "pPrism";
    }

    return "pPrimitive";
}

QString defaultJointPrefix()
{
    return "joint";
}

QPushButton* createTransportButton(const QString& text, QWidget* parent)
{
    QPushButton* button = new QPushButton(text, parent);
    button->setFixedWidth(28);
    return button;
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
            logSelectionToScriptEditor(0);
            statusBar()->showMessage("Selection cleared", 2000);
            return;
        }

        if (const SceneObject* object = viewport_->findObject(objectId)) {
            logSelectionToScriptEditor(objectId);
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
    QMenu* editMenu = menuBar()->addMenu("&Edit");
    undoAction_ = editMenu->addAction("Undo");
    undoAction_->setObjectName("undoAction");
    undoAction_->setShortcut(QKeySequence::Undo);
    QObject::connect(undoAction_, &QAction::triggered, this, &MainWindow::undoLastChange);

    redoAction_ = editMenu->addAction("Redo");
    redoAction_->setObjectName("redoAction");
    redoAction_->setShortcut(QKeySequence::Redo);
    QObject::connect(redoAction_, &QAction::triggered, this, &MainWindow::redoLastChange);

    editMenu->addSeparator();

    QMenu* fileMenu = menuBar()->addMenu("&File");
    newSceneAction_ = fileMenu->addAction("New Scene");
    newSceneAction_->setShortcut(QKeySequence::New);
    QObject::connect(newSceneAction_, &QAction::triggered, this, &MainWindow::newScene);

    openSceneAction_ = fileMenu->addAction("Open Scene...");
    openSceneAction_->setShortcut(QKeySequence::Open);
    QObject::connect(openSceneAction_, &QAction::triggered, this, &MainWindow::openScene);

    saveSceneAction_ = fileMenu->addAction("Save Scene");
    saveSceneAction_->setShortcut(QKeySequence::Save);
    QObject::connect(saveSceneAction_, &QAction::triggered, this, &MainWindow::saveScene);

    saveSceneAsAction_ = fileMenu->addAction("Save Scene As...");
    saveSceneAsAction_->setShortcut(QKeySequence::SaveAs);
    QObject::connect(saveSceneAsAction_, &QAction::triggered, this, &MainWindow::saveSceneAs);

    incrementAndSaveAction_ = fileMenu->addAction("Increment and Save");
    incrementAndSaveAction_->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_S));
    QObject::connect(incrementAndSaveAction_, &QAction::triggered, this, &MainWindow::incrementAndSave);

    archiveSceneAction_ = fileMenu->addAction("Archive Scene");
    QObject::connect(archiveSceneAction_, &QAction::triggered, this, &MainWindow::archiveScene);

    savePreferencesAction_ = fileMenu->addAction("Save Preferences");
    QObject::connect(savePreferencesAction_, &QAction::triggered, this, &MainWindow::savePreferences);

    optimizeSceneSizeAction_ = fileMenu->addAction("Optimize Scene Size");
    QObject::connect(optimizeSceneSizeAction_, &QAction::triggered, this, [this]() {
        viewport_->optimizeSceneStorage();
        refreshScenePanels();
        appendScriptComment("Scene storage optimized");
        statusBar()->showMessage("Scene storage optimized", 2000);
    });

    fileMenu->addSection("Import/Export");
    importFbxAction_ = fileMenu->addAction("Import...");
    importFbxAction_->setObjectName("importFbxAction");
    QObject::connect(importFbxAction_, &QAction::triggered, this, &MainWindow::importFbx);

    exportAllAction_ = fileMenu->addAction("Export All...");
    QObject::connect(exportAllAction_, &QAction::triggered, this, &MainWindow::exportAll);

    exportSelectionAction_ = fileMenu->addAction("Export Selection...");
    QObject::connect(exportSelectionAction_, &QAction::triggered, this, &MainWindow::exportSelection);

    QMenu* createMenu = menuBar()->addMenu("&Create");
    polygonPrimitivesAction_ = createMenu->addAction("Polygon Primitives");
    polygonPrimitivesAction_->setObjectName("polygonPrimitivesAction");
    QObject::connect(polygonPrimitivesAction_, &QAction::triggered, this, &MainWindow::showPolygonPrimitivesWindow);
    createJointAction_ = createMenu->addAction("Joint");
    createJointAction_->setObjectName("createJointAction");
    QObject::connect(createJointAction_, &QAction::triggered, this, &MainWindow::createJoint);

    QMenu* rigMenu = menuBar()->addMenu("&Rig");
    markHierarchyParentAction_ = rigMenu->addAction("Mark Selected As Parent");
    markHierarchyParentAction_->setObjectName("markHierarchyParentAction");
    markHierarchyParentAction_->setEnabled(false);
    QObject::connect(markHierarchyParentAction_, &QAction::triggered, this, &MainWindow::markSelectionAsHierarchyParent);

    parentToMarkedParentAction_ = rigMenu->addAction("Parent Selected To Marked Parent");
    parentToMarkedParentAction_->setObjectName("parentToMarkedParentAction");
    parentToMarkedParentAction_->setEnabled(false);
    parentToMarkedParentAction_->setShortcut(QKeySequence(Qt::Key_P));
    QObject::connect(parentToMarkedParentAction_, &QAction::triggered, this, &MainWindow::parentSelectionToMarkedParent);

    unparentSelectedAction_ = rigMenu->addAction("Unparent Selected");
    unparentSelectedAction_->setObjectName("unparentSelectedAction");
    unparentSelectedAction_->setEnabled(false);
    unparentSelectedAction_->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_P));
    QObject::connect(unparentSelectedAction_, &QAction::triggered, this, &MainWindow::unparentSelection);

    bindSkinAction_ = rigMenu->addAction("Bind Selected Mesh To Marked Joint");
    bindSkinAction_->setObjectName("bindSkinAction");
    bindSkinAction_->setEnabled(false);
    QObject::connect(bindSkinAction_, &QAction::triggered, this, &MainWindow::bindSelectedMeshToMarkedJoint);

    rigMenu->addSeparator();
    resetJointOrientationAction_ = rigMenu->addAction("Reset Joint Orientation");
    resetJointOrientationAction_->setObjectName("resetJointOrientationAction");
    resetJointOrientationAction_->setEnabled(false);
    QObject::connect(resetJointOrientationAction_, &QAction::triggered, this, &MainWindow::resetSelectedJointOrientation);

    alignJointOrientationAction_ = rigMenu->addAction("Align Joint Orientation To Child");
    alignJointOrientationAction_->setObjectName("alignJointOrientationAction");
    alignJointOrientationAction_->setEnabled(false);
    QObject::connect(alignJointOrientationAction_, &QAction::triggered, this, &MainWindow::alignSelectedJointOrientationToChild);

    captureBindPoseAction_ = rigMenu->addAction("Capture Bind Pose");
    captureBindPoseAction_->setObjectName("captureBindPoseAction");
    captureBindPoseAction_->setEnabled(false);
    QObject::connect(captureBindPoseAction_, &QAction::triggered, this, &MainWindow::captureSelectedBindPose);

    captureBindPoseRecursiveAction_ = rigMenu->addAction("Capture Bind Pose Recursive");
    captureBindPoseRecursiveAction_->setObjectName("captureBindPoseRecursiveAction");
    captureBindPoseRecursiveAction_->setEnabled(false);
    QObject::connect(captureBindPoseRecursiveAction_, &QAction::triggered, this, &MainWindow::captureSelectedBindPoseRecursive);

    QMenu* windowsMenu = menuBar()->addMenu("&Windows");
    scriptEditorAction_ = windowsMenu->addAction("Script Editor");
    scriptEditorAction_->setObjectName("scriptEditorAction");
    QObject::connect(scriptEditorAction_, &QAction::triggered, this, &MainWindow::showScriptEditorWindow);

    QMenu* animationMenu = menuBar()->addMenu("&Animation");
    duplicateKeyAction_ = animationMenu->addAction("Duplicate Current Key");
    duplicateKeyAction_->setObjectName("duplicateKeyAction");
    duplicateKeyAction_->setEnabled(false);
    duplicateKeyAction_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    QObject::connect(duplicateKeyAction_, &QAction::triggered, this, [this]() { duplicateCurrentKeyForSelection(true); });

    shiftKeysLeftAction_ = animationMenu->addAction("Shift Keys Left");
    shiftKeysLeftAction_->setObjectName("shiftKeysLeftAction");
    shiftKeysLeftAction_->setEnabled(false);
    QObject::connect(shiftKeysLeftAction_, &QAction::triggered, this, [this]() { shiftSelectedObjectKeyframes(-1, true); });

    shiftKeysRightAction_ = animationMenu->addAction("Shift Keys Right");
    shiftKeysRightAction_->setObjectName("shiftKeysRightAction");
    shiftKeysRightAction_->setEnabled(false);
    QObject::connect(shiftKeysRightAction_, &QAction::triggered, this, [this]() { shiftSelectedObjectKeyframes(1, true); });

    animationMenu->addSeparator();
    previousKeyAction_ = animationMenu->addAction("Previous Key");
    previousKeyAction_->setObjectName("previousKeyAction");
    previousKeyAction_->setEnabled(false);
    previousKeyAction_->setShortcut(QKeySequence(Qt::Key_Comma));
    QObject::connect(previousKeyAction_, &QAction::triggered, this, [this]() { jumpToSelectedObjectKeyframe(false, true); });

    nextKeyAction_ = animationMenu->addAction("Next Key");
    nextKeyAction_->setObjectName("nextKeyAction");
    nextKeyAction_->setEnabled(false);
    nextKeyAction_->setShortcut(QKeySequence(Qt::Key_Period));
    QObject::connect(nextKeyAction_, &QAction::triggered, this, [this]() { jumpToSelectedObjectKeyframe(true, true); });

    QMenu* viewMenu = menuBar()->addMenu("&View");

    resetCameraAction_ = viewMenu->addAction("Reset Camera");
    QObject::connect(resetCameraAction_, &QAction::triggered, this, [this]() {
        viewport_->resetCamera();
        appendScriptHistoryLine("viewSet -home;");
        appendScriptHistoryLine("// Result: camera reset //");
        statusBar()->showMessage("Camera reset", 2000);
    });

    frameSceneAction_ = viewMenu->addAction("Frame Scene");
    QObject::connect(frameSceneAction_, &QAction::triggered, this, [this]() {
        viewport_->frameScene();
        appendScriptHistoryLine("viewFit;");
        appendScriptHistoryLine("// Result: scene framed //");
        statusBar()->showMessage("Scene framed", 2000);
    });

    frameSelectedAction_ = viewMenu->addAction("Frame Selected");
    frameSelectedAction_->setEnabled(false);
    QObject::connect(frameSelectedAction_, &QAction::triggered, this, &MainWindow::frameSelectedObject);

    viewMenu->addSeparator();
    QMenu* camerasMenu = viewMenu->addMenu("Cameras");
    perspectiveCameraAction_ = camerasMenu->addAction("Perspective");
    perspectiveCameraAction_->setObjectName("perspectiveCameraAction");
    perspectiveCameraAction_->setCheckable(true);
    frontCameraAction_ = camerasMenu->addAction("Front");
    frontCameraAction_->setObjectName("frontCameraAction");
    frontCameraAction_->setCheckable(true);
    backCameraAction_ = camerasMenu->addAction("Back");
    backCameraAction_->setObjectName("backCameraAction");
    backCameraAction_->setCheckable(true);
    leftCameraAction_ = camerasMenu->addAction("Left");
    leftCameraAction_->setObjectName("leftCameraAction");
    leftCameraAction_->setCheckable(true);
    rightCameraAction_ = camerasMenu->addAction("Right");
    rightCameraAction_->setObjectName("rightCameraAction");
    rightCameraAction_->setCheckable(true);
    topCameraAction_ = camerasMenu->addAction("Top");
    topCameraAction_->setObjectName("topCameraAction");
    topCameraAction_->setCheckable(true);
    bottomCameraAction_ = camerasMenu->addAction("Bottom");
    bottomCameraAction_->setObjectName("bottomCameraAction");
    bottomCameraAction_->setCheckable(true);

    QObject::connect(perspectiveCameraAction_, &QAction::triggered, this, [this]() { setViewCameraPreset(ViewCameraUiPreset::Perspective); });
    QObject::connect(frontCameraAction_, &QAction::triggered, this, [this]() { setViewCameraPreset(ViewCameraUiPreset::Front); });
    QObject::connect(backCameraAction_, &QAction::triggered, this, [this]() { setViewCameraPreset(ViewCameraUiPreset::Back); });
    QObject::connect(leftCameraAction_, &QAction::triggered, this, [this]() { setViewCameraPreset(ViewCameraUiPreset::Left); });
    QObject::connect(rightCameraAction_, &QAction::triggered, this, [this]() { setViewCameraPreset(ViewCameraUiPreset::Right); });
    QObject::connect(topCameraAction_, &QAction::triggered, this, [this]() { setViewCameraPreset(ViewCameraUiPreset::Top); });
    QObject::connect(bottomCameraAction_, &QAction::triggered, this, [this]() { setViewCameraPreset(ViewCameraUiPreset::Bottom); });
    setViewCameraPreset(ViewCameraUiPreset::Perspective);

    wireframeAction_ = viewMenu->addAction("Wireframe");
    wireframeAction_->setCheckable(true);
    QObject::connect(wireframeAction_, &QAction::toggled, this, [this](bool enabled) {
        viewport_->setWireframeEnabled(enabled);
        appendScriptComment(QString("Wireframe %1").arg(enabled ? "on" : "off"));
        statusBar()->showMessage(enabled ? "Wireframe on" : "Wireframe off", 2000);
    });

    showAxisAction_ = viewMenu->addAction("Show Axis");
    showAxisAction_->setCheckable(true);
    showAxisAction_->setChecked(true);
    QObject::connect(showAxisAction_, &QAction::toggled, this, [this](bool enabled) {
        viewport_->setAxisVisible(enabled);
        appendScriptComment(QString("Axis visibility %1").arg(enabled ? "on" : "off"));
        statusBar()->showMessage(enabled ? "Axis visible" : "Axis hidden", 2000);
    });

    backfaceCullingAction_ = viewMenu->addAction("Backface Culling");
    backfaceCullingAction_->setCheckable(true);
    QObject::connect(backfaceCullingAction_, &QAction::toggled, this, [this](bool enabled) {
        viewport_->setBackfaceCullingEnabled(enabled);
        appendScriptComment(QString("Backface culling %1").arg(enabled ? "on" : "off"));
        statusBar()->showMessage(enabled ? "Backface culling on" : "Backface culling off", 2000);
    });

    viewMenu->addSeparator();
    restoreWorkspaceLayoutAction_ = viewMenu->addAction("Restore Default Layout");
    restoreWorkspaceLayoutAction_->setObjectName("restoreWorkspaceLayoutAction");
    QObject::connect(restoreWorkspaceLayoutAction_, &QAction::triggered, this, &MainWindow::restoreDefaultWorkspaceLayout);

    QMenu* transformMenu = menuBar()->addMenu("&Transform");
    translateAction_ = transformMenu->addAction("Translate");
    translateAction_->setObjectName("translateAction");
    translateAction_->setCheckable(true);
    translateAction_->setShortcut(QKeySequence(Qt::Key_W));
    rotateAction_ = transformMenu->addAction("Rotate");
    rotateAction_->setObjectName("rotateAction");
    rotateAction_->setCheckable(true);
    rotateAction_->setShortcut(QKeySequence(Qt::Key_E));
    scaleAction_ = transformMenu->addAction("Scale");
    scaleAction_->setObjectName("scaleAction");
    scaleAction_->setCheckable(true);
    scaleAction_->setShortcut(QKeySequence(Qt::Key_R));

    QObject::connect(translateAction_, &QAction::triggered, this, [this]() { setTransformUiMode(TransformUiMode::Translate); });
    QObject::connect(rotateAction_, &QAction::triggered, this, [this]() { setTransformUiMode(TransformUiMode::Rotate); });
    QObject::connect(scaleAction_, &QAction::triggered, this, [this]() { setTransformUiMode(TransformUiMode::Scale); });
    setTransformUiMode(TransformUiMode::Translate);

    QMenu* axisMenu = transformMenu->addMenu("Axis Orientation");
    worldAxisAction_ = axisMenu->addAction("World");
    worldAxisAction_->setObjectName("worldAxisAction");
    worldAxisAction_->setCheckable(true);
    localAxisAction_ = axisMenu->addAction("Local");
    localAxisAction_->setObjectName("localAxisAction");
    localAxisAction_->setCheckable(true);

    QObject::connect(worldAxisAction_, &QAction::triggered, this, [this]() { setAxisUiOrientation(AxisUiOrientation::World); });
    QObject::connect(localAxisAction_, &QAction::triggered, this, [this]() { setAxisUiOrientation(AxisUiOrientation::Local); });
    setAxisUiOrientation(AxisUiOrientation::World);
}

void MainWindow::createToolbar()
{
    toolbar_ = addToolBar("Viewport");
    toolbar_->setMovable(false);
    toolbar_->addAction(importFbxAction_);
    toolbar_->addAction(polygonPrimitivesAction_);
    toolbar_->addAction(createJointAction_);
    toolbar_->addAction(markHierarchyParentAction_);
    toolbar_->addAction(parentToMarkedParentAction_);
    toolbar_->addAction(unparentSelectedAction_);
    toolbar_->addAction(alignJointOrientationAction_);
    toolbar_->addAction(captureBindPoseAction_);
    toolbar_->addSeparator();
    toolbar_->addAction(resetCameraAction_);
    toolbar_->addAction(frameSceneAction_);
    toolbar_->addAction(frameSelectedAction_);
    toolbar_->addSeparator();
    toolbar_->addAction(translateAction_);
    toolbar_->addAction(rotateAction_);
    toolbar_->addAction(scaleAction_);
    toolbar_->addSeparator();
    toolbar_->addAction(worldAxisAction_);
    toolbar_->addAction(localAxisAction_);
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

    outlinerDock_ = new QDockWidget("Outliner", this);
    outlinerDock_->setObjectName("OutlinerDock");
    outlinerDock_->setAllowedAreas(Qt::AllDockWidgetAreas);
    outlinerDock_->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    outlinerDock_->setWidget(createOutlinerPanel());
    addDockWidget(Qt::LeftDockWidgetArea, outlinerDock_);

    inspectorDock_ = new QDockWidget("Channel Box", this);
    inspectorDock_->setObjectName("InspectorDock");
    inspectorDock_->setAllowedAreas(Qt::AllDockWidgetAreas);
    inspectorDock_->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    inspectorDock_->setWidget(createInspectorPanel());
    addDockWidget(Qt::RightDockWidgetArea, inspectorDock_);

    resizeDocks({ outlinerDock_, inspectorDock_ }, { 280, 320 }, Qt::Horizontal);

    polygonPrimitivesDock_ = new QDockWidget("Polygon Primitives", this);
    polygonPrimitivesDock_->setObjectName("PolygonPrimitivesDock");
    polygonPrimitivesDock_->setAllowedAreas(Qt::AllDockWidgetAreas);
    polygonPrimitivesDock_->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);

    QWidget* primitivesPanel = new QWidget(this);
    QVBoxLayout* primitivesLayout = new QVBoxLayout(primitivesPanel);
    primitivesLayout->setContentsMargins(8, 8, 8, 8);

    polygonPrimitivesList_ = new QListWidget(primitivesPanel);
    polygonPrimitivesList_->setObjectName("polygonPrimitivesList");
    const struct PrimitiveEntry {
        const char* label;
        bool implemented;
        PrimitiveMeshFactory::Type type;
    } entries[] = {
        { "Sphere", true, PrimitiveMeshFactory::Type::Sphere },
        { "Cube", true, PrimitiveMeshFactory::Type::Cube },
        { "Cylinder", true, PrimitiveMeshFactory::Type::Cylinder },
        { "Cone", true, PrimitiveMeshFactory::Type::Cone },
        { "Torus", true, PrimitiveMeshFactory::Type::Torus },
        { "Plane", true, PrimitiveMeshFactory::Type::Plane },
        { "Disc", true, PrimitiveMeshFactory::Type::Disc },
        { "Platonic Solid", false, PrimitiveMeshFactory::Type::Cube },
        { "Pyramid", true, PrimitiveMeshFactory::Type::Pyramid },
        { "Prism", true, PrimitiveMeshFactory::Type::Prism },
        { "Pipe", false, PrimitiveMeshFactory::Type::Cylinder },
        { "Helix", false, PrimitiveMeshFactory::Type::Cylinder },
        { "Gear", false, PrimitiveMeshFactory::Type::Cylinder },
        { "Soccer Ball", false, PrimitiveMeshFactory::Type::Sphere },
        { "Super Ellipse", false, PrimitiveMeshFactory::Type::Sphere },
        { "Spherical Harmonics", false, PrimitiveMeshFactory::Type::Sphere },
        { "Ultra Shape", false, PrimitiveMeshFactory::Type::Sphere }
    };

    for (const PrimitiveEntry& entry : entries) {
        QListWidgetItem* item = new QListWidgetItem(QString::fromUtf8(entry.label), polygonPrimitivesList_);
        item->setData(kPrimitiveTypeRole, static_cast<int>(entry.type));
        if (!entry.implemented) {
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
            item->setText(QString("%1 (coming soon)").arg(entry.label));
        }
    }
    QObject::connect(polygonPrimitivesList_, &QListWidget::itemClicked, this, [this](QListWidgetItem*) {
        createPrimitiveFromPalette();
    });
    QObject::connect(polygonPrimitivesList_, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem*) {
        createPrimitiveFromPalette();
    });

    QPushButton* createPrimitiveButton = new QPushButton("Create Selected Primitive", primitivesPanel);
    createPrimitiveButton->setObjectName("createPrimitiveButton");
    QObject::connect(createPrimitiveButton, &QPushButton::clicked, this, &MainWindow::createPrimitiveFromPalette);

    QCheckBox* interactiveCreationCheckBox = new QCheckBox("Interactive Creation", primitivesPanel);
    interactiveCreationCheckBox->setObjectName("interactiveCreationCheckBox");
    interactiveCreationCheckBox->setChecked(interactivePrimitiveCreationEnabled_);
    QObject::connect(interactiveCreationCheckBox, &QCheckBox::toggled, this, [this](bool enabled) {
        interactivePrimitiveCreationEnabled_ = enabled;
    });

    QCheckBox* exitOnCompletionCheckBox = new QCheckBox("Exit On Completion", primitivesPanel);
    exitOnCompletionCheckBox->setObjectName("exitOnCompletionCheckBox");
    exitOnCompletionCheckBox->setChecked(exitPrimitiveToolOnCompletionEnabled_);
    QObject::connect(exitOnCompletionCheckBox, &QCheckBox::toggled, this, [this](bool enabled) {
        exitPrimitiveToolOnCompletionEnabled_ = enabled;
    });

    primitivesLayout->addWidget(polygonPrimitivesList_);
    primitivesLayout->addWidget(createPrimitiveButton);
    primitivesLayout->addWidget(interactiveCreationCheckBox);
    primitivesLayout->addWidget(exitOnCompletionCheckBox);
    polygonPrimitivesDock_->setWidget(primitivesPanel);
    addDockWidget(Qt::RightDockWidgetArea, polygonPrimitivesDock_);
    polygonPrimitivesDock_->setFloating(true);
    polygonPrimitivesDock_->hide();

    scriptEditorDock_ = new QDockWidget("Script Editor", this);
    scriptEditorDock_->setObjectName("ScriptEditorDock");
    scriptEditorDock_->setAllowedAreas(Qt::AllDockWidgetAreas);
    scriptEditorDock_->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);
    scriptEditorDock_->setWidget(createScriptEditorPanel());
    addDockWidget(Qt::BottomDockWidgetArea, scriptEditorDock_);
    scriptEditorDock_->setFloating(true);
    scriptEditorDock_->hide();

    timeSliderDock_ = new QDockWidget("Time Slider", this);
    timeSliderDock_->setObjectName("TimeSliderDock");
    timeSliderDock_->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    timeSliderDock_->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    timeSliderDock_->setWidget(createTimeSliderPanel());
    addDockWidget(Qt::BottomDockWidgetArea, timeSliderDock_);
    resizeDocks({ timeSliderDock_ }, { 150 }, Qt::Vertical);
}

void MainWindow::newScene()
{
    viewport_->clearScene();
    setCurrentFrame(viewport_->currentFrame(), false);
    currentSceneFilePath_.clear();
    refreshScenePanels();
    viewport_->resetCamera();
    updateWindowTitle();
    appendScriptHistoryLine("file -f -new;");
    appendScriptHistoryLine("// Result: new scene //");
    statusBar()->showMessage("New scene created", 2000);
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
    QWidget* panel = new QWidget(this);
    QVBoxLayout* rootLayout = new QVBoxLayout(panel);
    rootLayout->setContentsMargins(10, 8, 10, 8);
    rootLayout->setSpacing(6);

    QHBoxLayout* topRow = new QHBoxLayout();
    topRow->setSpacing(10);

    playbackStartSpinBox_ = new QSpinBox(panel);
    playbackStartSpinBox_->setObjectName("playbackStartSpinBox");
    playbackStartSpinBox_->setRange(-10000, 100000);
    playbackStartSpinBox_->setValue(playbackStartFrame_);

    playbackEndSpinBox_ = new QSpinBox(panel);
    playbackEndSpinBox_->setObjectName("playbackEndSpinBox");
    playbackEndSpinBox_->setRange(-10000, 100000);
    playbackEndSpinBox_->setValue(playbackEndFrame_);

    currentFrameSpinBox_ = new QSpinBox(panel);
    currentFrameSpinBox_->setObjectName("currentFrameSpinBox");
    currentFrameSpinBox_->setRange(playbackStartFrame_, playbackEndFrame_);
    currentFrameSpinBox_->setValue(currentFrame_);

    topRow->addWidget(new QLabel("Start", panel));
    topRow->addWidget(playbackStartSpinBox_);
    topRow->addWidget(new QLabel("End", panel));
    topRow->addWidget(playbackEndSpinBox_);
    topRow->addStretch();
    autoKeyButton_ = new QPushButton("Auto Key", panel);
    autoKeyButton_->setObjectName("autoKeyButton");
    autoKeyButton_->setCheckable(true);
    autoKeyButton_->setChecked(autoKeyEnabled_);
    topRow->addWidget(autoKeyButton_);
    setKeyButton_ = new QPushButton("Key Selected", panel);
    setKeyButton_->setObjectName("setKeyButton");
    setKeyButton_->setEnabled(false);
    topRow->addWidget(setKeyButton_);
    deleteKeyButton_ = new QPushButton("Delete Key", panel);
    deleteKeyButton_->setObjectName("deleteKeyButton");
    deleteKeyButton_->setEnabled(false);
    topRow->addWidget(deleteKeyButton_);
    duplicateKeyButton_ = new QPushButton("Duplicate Key", panel);
    duplicateKeyButton_->setObjectName("duplicateKeyButton");
    duplicateKeyButton_->setEnabled(false);
    topRow->addWidget(duplicateKeyButton_);
    shiftKeysLeftButton_ = new QPushButton("Shift -1", panel);
    shiftKeysLeftButton_->setObjectName("shiftKeysLeftButton");
    shiftKeysLeftButton_->setEnabled(false);
    topRow->addWidget(shiftKeysLeftButton_);
    shiftKeysRightButton_ = new QPushButton("Shift +1", panel);
    shiftKeysRightButton_->setObjectName("shiftKeysRightButton");
    shiftKeysRightButton_->setEnabled(false);
    topRow->addWidget(shiftKeysRightButton_);
    topRow->addWidget(new QLabel("Current", panel));
    topRow->addWidget(currentFrameSpinBox_);

    QWidget* ticksHost = new QWidget(panel);
    QVBoxLayout* ticksLayout = new QVBoxLayout(ticksHost);
    ticksLayout->setContentsMargins(0, 0, 0, 0);
    ticksLayout->setSpacing(2);

    QHBoxLayout* labelsRow = new QHBoxLayout();
    labelsRow->setContentsMargins(4, 0, 4, 0);
    labelsRow->setSpacing(0);
    for (int frame = 0; frame <= 24; ++frame) {
        QLabel* label = new QLabel(QString::number(frame), ticksHost);
        label->setAlignment(frame == 24 ? Qt::AlignRight : Qt::AlignLeft);
        labelsRow->addWidget(label, 1);
    }

    timeSlider_ = new QSlider(Qt::Horizontal, ticksHost);
    timeSlider_->setObjectName("timeSlider");
    timeSlider_->setRange(playbackStartFrame_, playbackEndFrame_);
    timeSlider_->setValue(currentFrame_);
    timeSlider_->setTickPosition(QSlider::TicksBelow);
    timeSlider_->setTickInterval(1);
    timeSlider_->setPageStep(1);

    keyframeTimelineWidget_ = new KeyframeTimelineWidget(ticksHost);
    keyframeTimelineWidget_->setObjectName("keyframeTimelineWidget");
    keyframeTimelineWidget_->setFrameRange(playbackStartFrame_, playbackEndFrame_);
    keyframeTimelineWidget_->setCurrentFrame(currentFrame_);

    timelineStatusLabel_ = new QLabel("No selection", panel);
    timelineStatusLabel_->setObjectName("timelineStatusLabel");
    timelineStatusLabel_->setStyleSheet("color: #bdbdbd;");

    ticksLayout->addLayout(labelsRow);
    ticksLayout->addWidget(keyframeTimelineWidget_);
    ticksLayout->addWidget(timeSlider_);

    QHBoxLayout* controlsRow = new QHBoxLayout();
    controlsRow->setSpacing(4);
    controlsRow->addStretch();

    QPushButton* jumpStartButton = createTransportButton("|<", panel);
    jumpStartButton->setObjectName("jumpStartButton");
    previousKeyButton_ = createTransportButton("<<", panel);
    previousKeyButton_->setObjectName("previousKeyButton");
    QPushButton* stepBackButton = createTransportButton("<", panel);
    stepBackButton->setObjectName("stepBackButton");
    playPauseButton_ = createTransportButton(">", panel);
    playPauseButton_->setObjectName("playPauseButton");
    QPushButton* stepForwardButton = createTransportButton(">", panel);
    stepForwardButton->setObjectName("stepForwardButton");
    nextKeyButton_ = createTransportButton(">>", panel);
    nextKeyButton_->setObjectName("nextKeyButton");
    QPushButton* jumpEndButton = createTransportButton(">|", panel);
    jumpEndButton->setObjectName("jumpEndButton");

    controlsRow->addWidget(jumpStartButton);
    controlsRow->addWidget(previousKeyButton_);
    controlsRow->addWidget(stepBackButton);
    controlsRow->addWidget(playPauseButton_);
    controlsRow->addWidget(stepForwardButton);
    controlsRow->addWidget(nextKeyButton_);
    controlsRow->addWidget(jumpEndButton);

    QObject::connect(playbackStartSpinBox_, qOverload<int>(&QSpinBox::valueChanged), this, [this](int) {
        if (!updatingTimeSlider_) {
            setPlaybackRange(playbackStartSpinBox_->value(), playbackEndSpinBox_->value());
        }
    });
    QObject::connect(playbackEndSpinBox_, qOverload<int>(&QSpinBox::valueChanged), this, [this](int) {
        if (!updatingTimeSlider_) {
            setPlaybackRange(playbackStartSpinBox_->value(), playbackEndSpinBox_->value());
        }
    });
    QObject::connect(currentFrameSpinBox_, qOverload<int>(&QSpinBox::valueChanged), this, [this](int frame) {
        if (!updatingTimeSlider_) {
            setCurrentFrame(frame);
        }
    });
    QObject::connect(timeSlider_, &QSlider::valueChanged, this, [this](int frame) {
        if (!updatingTimeSlider_) {
            setCurrentFrame(frame);
        }
    });
    QObject::connect(jumpStartButton, &QPushButton::clicked, this, [this]() { setCurrentFrame(playbackStartFrame_); });
    QObject::connect(stepBackButton, &QPushButton::clicked, this, [this]() { stepFrame(-1); });
    QObject::connect(playPauseButton_, &QPushButton::clicked, this, &MainWindow::togglePlayback);
    QObject::connect(stepForwardButton, &QPushButton::clicked, this, [this]() { stepFrame(1); });
    QObject::connect(previousKeyButton_, &QPushButton::clicked, this, [this]() { jumpToSelectedObjectKeyframe(false, true); });
    QObject::connect(nextKeyButton_, &QPushButton::clicked, this, [this]() { jumpToSelectedObjectKeyframe(true, true); });
    QObject::connect(jumpEndButton, &QPushButton::clicked, this, [this]() { setCurrentFrame(playbackEndFrame_); });
    QObject::connect(setKeyButton_, &QPushButton::clicked, this, [this]() { setKeyForSelection(true); });
    QObject::connect(deleteKeyButton_, &QPushButton::clicked, this, [this]() { deleteKeyForSelection(true); });
    QObject::connect(duplicateKeyButton_, &QPushButton::clicked, this, [this]() { duplicateCurrentKeyForSelection(true); });
    QObject::connect(shiftKeysLeftButton_, &QPushButton::clicked, this, [this]() { shiftSelectedObjectKeyframes(-1, true); });
    QObject::connect(shiftKeysRightButton_, &QPushButton::clicked, this, [this]() { shiftSelectedObjectKeyframes(1, true); });
    QObject::connect(autoKeyButton_, &QPushButton::toggled, this, [this](bool enabled) { setAutoKeyEnabled(enabled, true); });

    rootLayout->addLayout(topRow);
    rootLayout->addWidget(ticksHost);
    rootLayout->addWidget(timelineStatusLabel_);
    rootLayout->addLayout(controlsRow);
    refreshAnimationTimelineUi();
    return panel;
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
        QMessageBox::warning(this, "Open Scene", result.errorMessage);
        statusBar()->showMessage("Open scene failed", 3000);
        return false;
    }

    recordUndoState();
    viewport_->replaceScene(result.scene);
    setCurrentFrame(viewport_->currentFrame(), false);
    currentSceneFilePath_ = filePath;
    refreshScenePanels();
    viewport_->frameScene();
    updateWindowTitle();
    if (logToScript) {
        appendScriptHistoryLine(QString("file -o \"%1\";").arg(QDir::toNativeSeparators(filePath)));
        appendScriptHistoryLine(QString("// Result: opened %1 //").arg(QFileInfo(filePath).fileName()));
    }
    statusBar()->showMessage(QString("Opened %1").arg(QFileInfo(filePath).fileName()), 3000);
    return true;
}

bool MainWindow::importFbxFromPath(const QString& filePath, bool logToScript)
{
    if (!viewport_->importFbx(filePath)) {
        const QString errorMessage = viewport_->lastImportMessage().isEmpty()
            ? "Unknown FBX import error."
            : viewport_->lastImportMessage();
        statusBar()->showMessage(QString("Import failed: %1").arg(QFileInfo(filePath).fileName()), 5000);
        QMessageBox::warning(this, "Import FBX", errorMessage);
        return false;
    }

    setCurrentFrame(currentFrame_, false);
    refreshScenePanels();
    if (logToScript) {
        appendScriptHistoryLine(QString("file -import \"%1\";").arg(QDir::toNativeSeparators(filePath)));
        appendScriptHistoryLine(QString("// Result: imported %1 //").arg(QFileInfo(filePath).fileName()));
    }
    statusBar()->showMessage(QString("Imported %1").arg(QFileInfo(filePath).fileName()), 4000);
    return true;
}

bool MainWindow::saveSceneToPath(const QString& filePath, bool logToScript)
{
    const QString targetPath = filePath.isEmpty() ? currentSceneFilePath_ : filePath;
    if (targetPath.isEmpty()) {
        statusBar()->showMessage("Save scene failed", 3000);
        return false;
    }

    const EditorDocumentController::OperationResult result = EditorDocumentController::saveScene(viewport_->scene(), targetPath);
    if (!result.success) {
        QMessageBox::warning(this, "Save Scene", result.errorMessage);
        statusBar()->showMessage("Save scene failed", 3000);
        return false;
    }

    currentSceneFilePath_ = targetPath;
    updateWindowTitle();
    if (logToScript) {
        appendScriptHistoryLine(QString("file -save \"%1\";").arg(QDir::toNativeSeparators(currentSceneFilePath_)));
        appendScriptHistoryLine(QString("// Result: saved %1 //").arg(QFileInfo(currentSceneFilePath_).fileName()));
    }
    statusBar()->showMessage(QString("Saved %1").arg(QFileInfo(currentSceneFilePath_).fileName()), 3000);
    return true;
}

bool MainWindow::incrementAndSave()
{
    QFileInfo info(currentSceneFilePath_);
    QString targetPath = currentSceneFilePath_;

    if (targetPath.isEmpty()) {
        targetPath = QDir::currentPath() + "/scene_001.phoenixscene";
    } else {
        const QString baseName = info.completeBaseName();
        QRegularExpression suffixPattern("^(.*?)(?:_(\\d+))?$");
        const QRegularExpressionMatch match = suffixPattern.match(baseName);
        const QString stem = match.hasMatch() ? match.captured(1) : baseName;
        const int version = match.hasMatch() && !match.captured(2).isEmpty() ? match.captured(2).toInt() + 1 : 1;
        const QString nextName = QString("%1_%2.%3")
                                     .arg(stem)
                                     .arg(version, 3, 10, QChar('0'))
                                     .arg(info.suffix().isEmpty() ? "phoenixscene" : info.suffix());
        targetPath = info.dir().filePath(nextName);
    }

    currentSceneFilePath_ = targetPath;
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

    const QString archiveName = QString("%1_%2.%3")
                                    .arg(info.completeBaseName())
                                    .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"))
                                    .arg(info.suffix());
    const QString archivePath = QDir(archiveDirPath).filePath(archiveName);

    QFile::remove(archivePath);
    if (!QFile::copy(currentSceneFilePath_, archivePath)) {
        QMessageBox::warning(this, "Archive Scene", "Failed to archive current scene file.");
        statusBar()->showMessage("Archive scene failed", 3000);
        return false;
    }

    statusBar()->showMessage(QString("Archived to %1").arg(QFileInfo(archivePath).fileName()), 3000);
    appendScriptComment(QString("Archived scene to %1").arg(QDir::toNativeSeparators(archivePath)));
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
        QMessageBox::warning(this, "Export All", result.errorMessage);
        statusBar()->showMessage("Export failed", 3000);
        return false;
    }

    statusBar()->showMessage(QString("Exported %1").arg(QFileInfo(filePath).fileName()), 3000);
    appendScriptHistoryLine(QString("file -exportAll \"%1\";").arg(QDir::toNativeSeparators(filePath)));
    appendScriptHistoryLine(QString("// Result: exported %1 //").arg(QFileInfo(filePath).fileName()));
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
        QMessageBox::warning(this, "Export Selection", result.errorMessage);
        statusBar()->showMessage("Export selection failed", 3000);
        return false;
    }

    statusBar()->showMessage(QString("Exported selection to %1").arg(QFileInfo(filePath).fileName()), 3000);
    appendScriptHistoryLine(QString("file -exportSelected \"%1\";").arg(QDir::toNativeSeparators(filePath)));
    appendScriptHistoryLine(QString("// Result: exported selection to %1 //").arg(QFileInfo(filePath).fileName()));
    return true;
}

void MainWindow::savePreferences()
{
    QSettings settings("ProjectPhoenix", "PhoenixEditor");
    settings.setValue("mainWindow/geometry", saveGeometry());
    settings.setValue("mainWindow/state", saveState());
    settings.setValue("animation/autoKeyEnabled", autoKeyEnabled_);
    appendScriptComment("Saved Phoenix Editor preferences");
    statusBar()->showMessage("Preferences saved", 2000);
}

void MainWindow::loadPreferences()
{
    QSettings settings("ProjectPhoenix", "PhoenixEditor");
    const QByteArray geometry = settings.value("mainWindow/geometry").toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }

    const QByteArray state = settings.value("mainWindow/state").toByteArray();
    if (!state.isEmpty()) {
        restoreState(state);
    }

    autoKeyEnabled_ = settings.value("animation/autoKeyEnabled", false).toBool();
    if (viewport_ != nullptr) {
        viewport_->setAutoKeyEnabled(autoKeyEnabled_);
    }
}

void MainWindow::createPrimitiveFromPalette()
{
    if (polygonPrimitivesList_ == nullptr || polygonPrimitivesList_->currentItem() == nullptr) {
        statusBar()->showMessage("Choose a primitive first", 1500);
        return;
    }

    const PrimitiveMeshFactory::Type type = primitiveTypeFromItem(polygonPrimitivesList_->currentItem());
    if (!PrimitiveMeshFactory::isImplemented(type)) {
        statusBar()->showMessage("This primitive is not implemented yet", 2000);
        return;
    }

    const QString objectName = generateUniqueScriptName(primitivePrefix(type));
    const SceneObject::Id objectId = viewport_->createPrimitive(type, objectName);
    if (objectId == 0) {
        statusBar()->showMessage("Primitive creation failed", 2000);
        return;
    }

    refreshScenePanels();
    selectObject(objectId, true);
    logPrimitiveCreationToScriptEditor(type, objectName);

    if (interactivePrimitiveCreationEnabled_) {
        setTransformUiMode(TransformUiMode::Translate);
        viewport_->frameObject(objectId);
    }

    if (exitPrimitiveToolOnCompletionEnabled_ && polygonPrimitivesDock_ != nullptr) {
        polygonPrimitivesDock_->hide();
    }

    statusBar()->showMessage(QString("Created %1").arg(PrimitiveMeshFactory::displayName(type)), 2000);
}

void MainWindow::createJoint()
{
    const QString jointName = generateUniqueObjectName(defaultJointPrefix());
    const SceneObject::Id parentId = selectedObjectId_ != 0 ? selectedObjectId_ : 0;
    const SceneObject::Id objectId = viewport_->createJoint(jointName, parentId);
    if (objectId == 0) {
        statusBar()->showMessage("Joint creation failed", 2000);
        return;
    }

    refreshScenePanels();
    selectObject(objectId, true);
    appendScriptHistoryLine(QString("joint -name \"%1\";").arg(jointName));
    appendScriptHistoryLine(QString("// Result: %1 //").arg(jointName));
    statusBar()->showMessage(QString("Created %1").arg(jointName), 2000);
}

void MainWindow::markSelectionAsHierarchyParent()
{
    if (selectedObjectId_ == 0) {
        statusBar()->showMessage("Select an object to mark as parent", 1500);
        return;
    }

    const SceneObject* object = viewport_->findObject(selectedObjectId_);
    if (object == nullptr) {
        statusBar()->showMessage("Selected object is no longer available", 1500);
        return;
    }

    markedHierarchyParentId_ = selectedObjectId_;
    const QString objectName = objectDisplayName(*object);
    appendScriptComment(QString("Marked hierarchy parent: %1").arg(objectName));
    statusBar()->showMessage(QString("Marked parent: %1").arg(objectName), 2000);
    updateInspector(selectedObjectId_);
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
    if (selectedObjectId_ == 0 || markedHierarchyParentId_ == 0) {
        statusBar()->showMessage("Select a mesh and mark a joint first", 1500);
        return;
    }

    const SceneObject* meshObject = viewport_->findObject(selectedObjectId_);
    const SceneObject* jointObject = viewport_->findObject(markedHierarchyParentId_);
    if (meshObject == nullptr || jointObject == nullptr) {
        statusBar()->showMessage("Bind target is no longer available", 1500);
        return;
    }

    if (meshObject->meshHandles().isEmpty()) {
        statusBar()->showMessage("Selected object has no mesh to bind", 1500);
        return;
    }

    if (!jointObject->isJoint()) {
        statusBar()->showMessage("Marked object is not a joint", 1500);
        return;
    }

    Scene updatedScene = viewport_->sceneSnapshot();
    if (!updatedScene.bindObjectToSkeleton(selectedObjectId_, markedHierarchyParentId_)) {
        statusBar()->showMessage("Bind skin failed", 1500);
        return;
    }

    const QString meshName = objectDisplayName(*meshObject);
    const QString jointName = objectDisplayName(*jointObject);
    recordUndoState();
    viewport_->replaceScene(updatedScene);
    refreshScenePanels();
    selectObject(selectedObjectId_, true);
    appendScriptHistoryLine(QString("bindSkin %1 %2;").arg(meshName, jointName));
    appendScriptHistoryLine(QString("// Result: bound %1 to %2 //").arg(meshName, jointName));
    statusBar()->showMessage(QString("Bound %1 to %2").arg(meshName, jointName), 2000);
}

void MainWindow::resetSelectedJointOrientation()
{
    if (selectedObjectId_ == 0) {
        statusBar()->showMessage("Select a joint to reset orientation", 1500);
        return;
    }

    const SceneObject* object = viewport_->findObject(selectedObjectId_);
    if (object == nullptr || !object->isJoint()) {
        statusBar()->showMessage("Selected object is not a joint", 1500);
        return;
    }

    if (!viewport_->resetJointOrientation(selectedObjectId_)) {
        statusBar()->showMessage("Reset joint orientation failed", 1500);
        return;
    }

    updateInspector(selectedObjectId_);
    appendScriptHistoryLine(QString("jointOrient %1 -reset;").arg(objectDisplayName(*object)));
    appendScriptHistoryLine(QString("// Result: reset joint orientation on %1 //").arg(objectDisplayName(*object)));
    statusBar()->showMessage("Joint orientation reset", 1500);
}

void MainWindow::alignSelectedJointOrientationToChild()
{
    if (selectedObjectId_ == 0) {
        statusBar()->showMessage("Select a joint to align orientation", 1500);
        return;
    }

    const SceneObject* object = viewport_->findObject(selectedObjectId_);
    if (object == nullptr || !object->isJoint()) {
        statusBar()->showMessage("Selected object is not a joint", 1500);
        return;
    }

    if (!viewport_->alignJointOrientationToChild(selectedObjectId_)) {
        statusBar()->showMessage("Align orientation needs a child joint offset", 1500);
        return;
    }

    updateInspector(selectedObjectId_);
    appendScriptHistoryLine(QString("jointOrient %1 -alignToChild;").arg(objectDisplayName(*object)));
    appendScriptHistoryLine(QString("// Result: aligned joint orientation on %1 //").arg(objectDisplayName(*object)));
    statusBar()->showMessage("Joint orientation aligned to child", 1500);
}

void MainWindow::captureSelectedBindPose()
{
    if (selectedObjectId_ == 0) {
        statusBar()->showMessage("Select a joint to capture bind pose", 1500);
        return;
    }

    const SceneObject* object = viewport_->findObject(selectedObjectId_);
    if (object == nullptr || !object->isJoint()) {
        statusBar()->showMessage("Selected object is not a joint", 1500);
        return;
    }

    if (!viewport_->captureBindPose(selectedObjectId_, false)) {
        statusBar()->showMessage("Capture bind pose failed", 1500);
        return;
    }

    updateInspector(selectedObjectId_);
    appendScriptHistoryLine(QString("bindPose -capture %1;").arg(objectDisplayName(*object)));
    appendScriptHistoryLine(QString("// Result: captured bind pose on %1 //").arg(objectDisplayName(*object)));
    statusBar()->showMessage("Bind pose captured", 1500);
}

void MainWindow::captureSelectedBindPoseRecursive()
{
    if (selectedObjectId_ == 0) {
        statusBar()->showMessage("Select a joint to capture bind pose", 1500);
        return;
    }

    const SceneObject* object = viewport_->findObject(selectedObjectId_);
    if (object == nullptr || !object->isJoint()) {
        statusBar()->showMessage("Selected object is not a joint", 1500);
        return;
    }

    if (!viewport_->captureBindPose(selectedObjectId_, true)) {
        statusBar()->showMessage("Recursive capture bind pose failed", 1500);
        return;
    }

    updateInspector(selectedObjectId_);
    appendScriptHistoryLine(QString("bindPose -capture -recursive %1;").arg(objectDisplayName(*object)));
    appendScriptHistoryLine(QString("// Result: captured bind pose on %1 recursively //").arg(objectDisplayName(*object)));
    statusBar()->showMessage("Bind pose captured recursively", 1500);
}

bool MainWindow::reparentObjectInUi(std::uint64_t childId, std::uint64_t newParentId, bool logToScript)
{
    const SceneObject* childObject = viewport_->findObject(childId);
    const SceneObject* parentObject = newParentId == 0 ? nullptr : viewport_->findObject(newParentId);
    if (childObject == nullptr || (newParentId != 0 && parentObject == nullptr)) {
        statusBar()->showMessage("Hierarchy target is no longer available", 1500);
        return false;
    }

    if (childId == newParentId) {
        statusBar()->showMessage("Cannot parent an object to itself", 1500);
        return false;
    }

    const QString childName = objectDisplayName(*childObject);
    const QString parentName = parentObject == nullptr ? QString() : objectDisplayName(*parentObject);

    Scene updatedScene = viewport_->sceneSnapshot();
    if (!updatedScene.reparentObject(childId, newParentId)) {
        statusBar()->showMessage(newParentId == 0 ? "Unparent operation failed" : "Parent operation failed", 1500);
        return false;
    }

    recordUndoState();
    viewport_->replaceScene(updatedScene);
    refreshScenePanels();
    selectObject(childId, true);

    if (logToScript) {
        if (newParentId == 0) {
            appendScriptHistoryLine(QString("unparent %1;").arg(childName));
            appendScriptHistoryLine(QString("// Result: unparented %1 //").arg(childName));
        } else {
            appendScriptHistoryLine(QString("parent %1 %2;").arg(childName, parentName));
            appendScriptHistoryLine(QString("// Result: parented %1 under %2 //").arg(childName, parentName));
        }
    }

    statusBar()->showMessage(
        newParentId == 0
            ? QString("Unparented %1").arg(childName)
            : QString("Parented %1 under %2").arg(childName, parentName),
        2000);
    return true;
}

void MainWindow::showScriptEditorWindow()
{
    if (scriptEditorDock_ == nullptr) {
        return;
    }

    scriptEditorDock_->show();
    scriptEditorDock_->raise();
    appendScriptComment("Script Editor opened");
}

void MainWindow::executeScriptEditorAll()
{
    if (scriptInputTextEdit_ == nullptr) {
        return;
    }

    const QStringList lines = scriptInputTextEdit_->toPlainText().split('\n');
    for (QString line : lines) {
        QString resultLine;
        if (executeScriptCommand(line, &resultLine)) {
            if (!resultLine.isEmpty()) {
                appendScriptHistoryLine(resultLine);
            }
        }
    }
}

void MainWindow::executeScriptEditorSelection()
{
    if (scriptInputTextEdit_ == nullptr) {
        return;
    }

    QString selectedText = scriptInputTextEdit_->textCursor().selectedText();
    selectedText.replace(QChar(0x2029), '\n');
    const QStringList lines = selectedText.split('\n');
    for (QString line : lines) {
        QString resultLine;
        if (executeScriptCommand(line, &resultLine)) {
            if (!resultLine.isEmpty()) {
                appendScriptHistoryLine(resultLine);
            }
        }
    }
}

void MainWindow::clearScriptHistory()
{
    if (scriptHistoryTextEdit_ != nullptr) {
        scriptHistoryTextEdit_->clear();
        appendScriptComment("Script history cleared");
    }
}

void MainWindow::appendScriptHistoryLine(const QString& line)
{
    if (scriptHistoryTextEdit_ == nullptr || line.trimmed().isEmpty()) {
        return;
    }

    scriptHistoryTextEdit_->appendPlainText(line);
    QTextCursor cursor = scriptHistoryTextEdit_->textCursor();
    cursor.movePosition(QTextCursor::End);
    scriptHistoryTextEdit_->setTextCursor(cursor);
}

void MainWindow::appendScriptComment(const QString& line)
{
    if (line.trimmed().isEmpty()) {
        return;
    }

    appendScriptHistoryLine(QString("// %1 //").arg(line));
}

bool MainWindow::executeScriptCommand(QString commandLine, QString* resultLine)
{
    commandLine = commandLine.trimmed();
    if (commandLine.isEmpty()) {
        return false;
    }

    if (commandLine.startsWith("//")) {
        appendScriptHistoryLine(commandLine);
        return false;
    }

    appendScriptHistoryLine(commandLine);
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
    context.clearSelection = [this]() {
        clearInspector();
    };
    context.selectObjectByName = [this](const QString& objectName) {
        const std::uint64_t objectId = findObjectIdByName(objectName);
        if (objectId == 0) {
            return false;
        }

        selectObject(objectId, true);
        return true;
    };
    context.createPrimitive = [this](PrimitiveMeshFactory::Type type) {
        const QString objectName = generateUniqueScriptName(primitivePrefix(type));
        const SceneObject::Id objectId = viewport_->createPrimitive(type, objectName);
        if (objectId == 0) {
            return QString();
        }

        refreshScenePanels();
        selectObject(objectId, true);
        return objectName;
    };
    context.createJoint = [this](const QString& requestedName) {
        const QString objectName = generateUniqueObjectName(requestedName.trimmed().isEmpty() ? defaultJointPrefix() : requestedName.trimmed());
        const SceneObject::Id parentId = selectedObjectId_ != 0 ? selectedObjectId_ : 0;
        const SceneObject::Id objectId = viewport_->createJoint(objectName, parentId);
        if (objectId == 0) {
            return QString();
        }

        refreshScenePanels();
        selectObject(objectId, true);
        return objectName;
    };
    context.renameObject = [this](const QString& sourceName, const QString& newName) {
        const std::uint64_t objectId = findObjectIdByName(sourceName);
        if (objectId == 0 || newName.trimmed().isEmpty()) {
            return QString();
        }

        const QString uniqueName = generateUniqueObjectName(newName.trimmed(), objectId);
        const EditorSceneMutationController::MutationResult mutation =
            EditorSceneMutationController::renameObject(viewport_->sceneSnapshot(), objectId, uniqueName);
        if (!mutation.success) {
            return QString();
        }

        recordUndoState();
        viewport_->replaceScene(mutation.scene);
        refreshScenePanels();
        selectObject(objectId, true);
        return uniqueName;
    };
    context.duplicateObject = [this](const QString& sourceName) {
        const std::uint64_t objectId = findObjectIdByName(sourceName);
        if (objectId == 0) {
            return QString();
        }

        const SceneObject* sourceObject = viewport_->findObject(objectId);
        if (sourceObject == nullptr) {
            return QString();
        }

        const QString sourceObjectName = sourceObject->name();
        const QString duplicateName = generateUniqueObjectName(QString("%1Copy").arg(sourceObjectName));
        const EditorSceneMutationController::MutationResult mutation =
            EditorSceneMutationController::duplicateObject(viewport_->sceneSnapshot(), objectId, duplicateName);
        if (!mutation.success) {
            return QString();
        }

        recordUndoState();
        viewport_->replaceScene(mutation.scene);
        refreshScenePanels();
        selectObject(mutation.affectedObjectId, true);
        return duplicateName;
    };
    context.groupObject = [this](const QString& sourceName) {
        const std::uint64_t objectId = findObjectIdByName(sourceName);
        if (objectId == 0) {
            return QString();
        }

        const QString groupName = generateUniqueObjectName("group");
        const EditorSceneMutationController::MutationResult mutation =
            EditorSceneMutationController::groupObject(viewport_->sceneSnapshot(), objectId, groupName);
        if (!mutation.success) {
            return QString();
        }

        recordUndoState();
        viewport_->replaceScene(mutation.scene);
        refreshScenePanels();
        selectObject(mutation.affectedObjectId, true);
        return groupName;
    };
    context.deleteObject = [this](const QString& sourceName) {
        const std::uint64_t objectId = findObjectIdByName(sourceName);
        if (objectId == 0) {
            return false;
        }

        const EditorSceneMutationController::MutationResult mutation =
            EditorSceneMutationController::deleteObject(viewport_->sceneSnapshot(), objectId);
        if (!mutation.success) {
            return false;
        }

        recordUndoState();
        viewport_->replaceScene(mutation.scene);
        refreshScenePanels();
        clearInspector();
        return true;
    };
    context.parentObject = [this](const QString& childName, const QString& parentName) {
        const std::uint64_t childId = findObjectIdByName(childName);
        const std::uint64_t parentId = findObjectIdByName(parentName);
        if (childId == 0 || parentId == 0) {
            return false;
        }

        const EditorSceneMutationController::MutationResult mutation =
            EditorSceneMutationController::reparentObject(viewport_->sceneSnapshot(), childId, parentId);
        if (!mutation.success) {
            return false;
        }

        recordUndoState();
        viewport_->replaceScene(mutation.scene);
        refreshScenePanels();
        selectObject(childId, true);
        return true;
    };
    context.unparentObject = [this](const QString& childName) {
        const std::uint64_t childId = findObjectIdByName(childName);
        if (childId == 0) {
            return false;
        }

        const EditorSceneMutationController::MutationResult mutation =
            EditorSceneMutationController::reparentObject(viewport_->sceneSnapshot(), childId, 0);
        if (!mutation.success) {
            return false;
        }

        recordUndoState();
        viewport_->replaceScene(mutation.scene);
        refreshScenePanels();
        selectObject(childId, true);
        return true;
    };
    context.bindSkin = [this](const QString& meshName, const QString& jointName) {
        const std::uint64_t meshId = findObjectIdByName(meshName);
        const std::uint64_t jointId = findObjectIdByName(jointName);
        if (meshId == 0 || jointId == 0) {
            return false;
        }

        const EditorSceneMutationController::MutationResult mutation =
            EditorSceneMutationController::bindSkin(viewport_->sceneSnapshot(), meshId, jointId);
        if (!mutation.success) {
            return false;
        }

        recordUndoState();
        viewport_->replaceScene(mutation.scene);
        refreshScenePanels();
        selectObject(meshId, true);
        return true;
    };
    context.setJointOrientation = [this](const QString& objectName, const QVector3D& eulerDegrees) {
        const std::uint64_t objectId = findObjectIdByName(objectName);
        if (objectId == 0) {
            return false;
        }

        if (!viewport_->setJointOrientation(objectId, QQuaternion::fromEulerAngles(eulerDegrees))) {
            return false;
        }

        refreshScenePanels();
        selectObject(objectId, true);
        return true;
    };
    context.resetJointOrientation = [this](const QString& objectName) {
        const std::uint64_t objectId = findObjectIdByName(objectName);
        if (objectId == 0) {
            return false;
        }

        if (!viewport_->resetJointOrientation(objectId)) {
            return false;
        }

        refreshScenePanels();
        selectObject(objectId, true);
        return true;
    };
    context.alignJointOrientationToChild = [this](const QString& objectName) {
        const std::uint64_t objectId = findObjectIdByName(objectName);
        if (objectId == 0) {
            return false;
        }

        if (!viewport_->alignJointOrientationToChild(objectId)) {
            return false;
        }

        refreshScenePanels();
        selectObject(objectId, true);
        return true;
    };
    context.captureBindPose = [this](const QString& objectName, bool recursive) {
        const std::uint64_t objectId = findObjectIdByName(objectName);
        if (objectId == 0) {
            return false;
        }

        if (!viewport_->captureBindPose(objectId, recursive)) {
            return false;
        }

        refreshScenePanels();
        selectObject(objectId, true);
        return true;
    };
    context.setAttribute = [this](const QString& objectName, const QString& attributeName, const QList<double>& values) {
        const std::uint64_t objectId = findObjectIdByName(objectName);
        if (objectId == 0) {
            return false;
        }

        const SceneObject* object = viewport_->findObject(objectId);
        if (object == nullptr) {
            return false;
        }

        if (attributeName == "visibility") {
            if (values.size() != 1) {
                return false;
            }

            const bool visible = values.first() != 0.0;
            if (!viewport_->setObjectVisibility(objectId, visible)) {
                return false;
            }

            if (objectId == selectedObjectId_) {
                updateChannelBox(objectId);
                const SceneObject* updatedObject = viewport_->findObject(objectId);
                const bool canFrame = updatedObject != nullptr && updatedObject->isVisible() && updatedObject->worldBounds().isValid();
                frameSelectedButton_->setEnabled(canFrame);
                frameSelectedAction_->setEnabled(canFrame);
            }

            return true;
        }

        if (values.size() != 3) {
            return false;
        }

        if (attributeName == "jointOrient") {
        if (!viewport_->setJointOrientation(objectId, QQuaternion::fromEulerAngles(
                    static_cast<float>(values.at(0)),
                    static_cast<float>(values.at(1)),
                    static_cast<float>(values.at(2))))) {
                return false;
            }

            if (objectId == selectedObjectId_) {
                updateChannelBox(objectId);
            }
            return true;
        }

        Transform transform = object->localTransform();
        const QVector3D vectorValue(
            static_cast<float>(values.at(0)),
            static_cast<float>(values.at(1)),
            static_cast<float>(values.at(2)));

        if (attributeName == "translate") {
            transform.translation = vectorValue;
        } else if (attributeName == "rotate") {
            transform.rotation = QQuaternion::fromEulerAngles(vectorValue);
        } else if (attributeName == "scale") {
            transform.scale = vectorValue;
        } else {
            return false;
        }

        if (!viewport_->setObjectLocalTransform(objectId, transform)) {
            return false;
        }

        if (objectId == selectedObjectId_) {
            updateChannelBox(objectId);
        }
        return true;
    };
    context.newScene = [this]() {
        viewport_->clearScene();
        currentSceneFilePath_.clear();
        refreshScenePanels();
        viewport_->resetCamera();
        updateWindowTitle();
    };
    context.openSceneFile = [this](const QString& filePath) {
        return openSceneFromPath(filePath, false);
    };
    context.importSceneFile = [this](const QString& filePath) {
        return importFbxFromPath(filePath, false);
    };
    context.saveSceneFile = [this](const QString& filePath) {
        return saveSceneToPath(filePath, false);
    };
    context.frameView = [this](const QString& targetName) {
        if (targetName.trimmed().isEmpty()) {
            viewport_->frameScene();
            return true;
        }

        const std::uint64_t objectId = findObjectIdByName(targetName);
        if (objectId == 0) {
            return false;
        }

        viewport_->frameObject(objectId);
        return true;
    };
    context.resetCamera = [this]() {
        viewport_->resetCamera();
    };
    context.activateTool = [this](const QString& toolName) {
        if (toolName == "MoveSuperContext") {
            translateAction_->setChecked(true);
            rotateAction_->setChecked(false);
            scaleAction_->setChecked(false);
            viewport_->setTransformMode(ViewportWidget::TransformMode::Translate);
            return true;
        }

        if (toolName == "RotateSuperContext") {
            translateAction_->setChecked(false);
            rotateAction_->setChecked(true);
            scaleAction_->setChecked(false);
            viewport_->setTransformMode(ViewportWidget::TransformMode::Rotate);
            return true;
        }

        if (toolName == "ScaleSuperContext") {
            translateAction_->setChecked(false);
            rotateAction_->setChecked(false);
            scaleAction_->setChecked(true);
            viewport_->setTransformMode(ViewportWidget::TransformMode::Scale);
            return true;
        }

        return false;
    };
    context.setCurrentFrame = [this](int frame) {
        setCurrentFrame(frame, false);
    };
    context.setKeyframe = [this](const QString& objectName, int frame) {
        const std::uint64_t objectId = findObjectIdByName(objectName);
        if (objectId == 0) {
            return false;
        }

        selectObject(objectId, true);
        if (frame >= 0) {
            setCurrentFrame(frame, false);
        }

        return viewport_->setObjectKeyframe(objectId, currentFrame_);
    };
    context.deleteKeyframe = [this](const QString& objectName, int frame) {
        const std::uint64_t objectId = findObjectIdByName(objectName);
        if (objectId == 0) {
            return false;
        }

        selectObject(objectId, true);
        if (frame >= 0) {
            setCurrentFrame(frame, false);
        }

        return viewport_->removeObjectKeyframe(objectId, currentFrame_);
    };
    context.copyKeyframe = [this](const QString& objectName, int sourceFrame, int targetFrame) {
        const std::uint64_t objectId = findObjectIdByName(objectName);
        if (objectId == 0) {
            return false;
        }

        Scene updatedScene = viewport_->sceneSnapshot();
        if (!updatedScene.duplicateObjectKeyframe(objectId, sourceFrame, targetFrame)) {
            return false;
        }

        recordUndoState();
        viewport_->replaceScene(updatedScene);
        refreshScenePanels();
        selectObject(objectId, true);
        setCurrentFrame(targetFrame, false);
        return true;
    };
    context.shiftKeyframes = [this](const QString& objectName, int frameDelta) {
        const std::uint64_t objectId = findObjectIdByName(objectName);
        if (objectId == 0) {
            return false;
        }

        Scene updatedScene = viewport_->sceneSnapshot();
        if (!updatedScene.offsetObjectKeyframes(objectId, frameDelta)) {
            return false;
        }

        recordUndoState();
        viewport_->replaceScene(updatedScene);
        refreshScenePanels();
        selectObject(objectId, true);
        setCurrentFrame(currentFrame_ + frameDelta, false);
        return true;
    };
    context.setAutoKey = [this](bool enabled) {
        setAutoKeyEnabled(enabled, false);
    };
    context.setPlaybackRange = [this](int startFrame, int endFrame) {
        setPlaybackRange(startFrame, endFrame, false);
    };
    context.setPlaybackState = [this](bool playing) {
        if (playbackTimer_ == nullptr || playPauseButton_ == nullptr) {
            return;
        }

        if (playing) {
            playbackTimer_->start();
            playPauseButton_->setText("||");
        } else {
            playbackTimer_->stop();
            playPauseButton_->setText(">");
        }
    };
    return context;
}

std::uint64_t MainWindow::findObjectIdByName(const QString& objectName) const
{
    for (std::uint64_t objectId : viewport_->allObjectIds()) {
        const SceneObject* object = viewport_->findObject(objectId);
        if (object != nullptr && objectDisplayName(*object) == objectName) {
            return objectId;
        }
    }

    return 0;
}

QString MainWindow::generateUniqueScriptName(const QString& prefix) const
{
    int suffix = 1;
    while (findObjectIdByName(QString("%1%2").arg(prefix).arg(suffix)) != 0) {
        ++suffix;
    }
    return QString("%1%2").arg(prefix).arg(suffix);
}

QString MainWindow::generateUniqueObjectName(const QString& baseName, std::uint64_t ignoreObjectId) const
{
    const auto nameInUse = [this, ignoreObjectId](const QString& candidate) {
        for (std::uint64_t objectId : viewport_->allObjectIds()) {
            if (objectId == ignoreObjectId) {
                continue;
            }

            const SceneObject* object = viewport_->findObject(objectId);
            if (object != nullptr && objectDisplayName(*object) == candidate) {
                return true;
            }
        }

        return false;
    };

    const QString trimmedBaseName = baseName.trimmed().isEmpty() ? QString("object") : baseName.trimmed();
    if (!nameInUse(trimmedBaseName)) {
        return trimmedBaseName;
    }

    int suffix = 1;
    while (nameInUse(QString("%1%2").arg(trimmedBaseName).arg(suffix))) {
        ++suffix;
    }

    return QString("%1%2").arg(trimmedBaseName).arg(suffix);
}

void MainWindow::logPrimitiveCreationToScriptEditor(PrimitiveMeshFactory::Type type, const QString& objectName)
{
    appendScriptHistoryLine(QString("select -cl ;"));
    appendScriptHistoryLine(QString("%1 -ch 1;").arg(mayaCommandName(type)));
    appendScriptHistoryLine(QString("// Result: %1 %1Shape //").arg(objectName));
}

void MainWindow::logSelectionToScriptEditor(std::uint64_t objectId)
{
    if (objectId == 0) {
        appendScriptHistoryLine("select -cl;");
        appendScriptHistoryLine("// Result: selection cleared //");
        return;
    }

    const SceneObject* object = viewport_->findObject(objectId);
    if (object == nullptr) {
        return;
    }

    appendScriptHistoryLine(QString("select -r %1;").arg(objectDisplayName(*object)));
    appendScriptHistoryLine(QString("// Result: %1 //").arg(objectDisplayName(*object)));
}

void MainWindow::logChannelBoxChangeToScriptEditor(const Transform& transform)
{
    const SceneObject* object = viewport_->findObject(selectedObjectId_);
    if (object == nullptr) {
        return;
    }

    const QString objectName = objectDisplayName(*object);
    const QVector3D eulerDegrees = transform.rotation.toEulerAngles();
    appendScriptHistoryLine(QString("setAttr \"%1.translate\" %2 %3 %4;")
                                .arg(objectName)
                                .arg(transform.translation.x(), 0, 'f', 3)
                                .arg(transform.translation.y(), 0, 'f', 3)
                                .arg(transform.translation.z(), 0, 'f', 3));
    appendScriptHistoryLine(QString("setAttr \"%1.rotate\" %2 %3 %4;")
                                .arg(objectName)
                                .arg(eulerDegrees.x(), 0, 'f', 3)
                                .arg(eulerDegrees.y(), 0, 'f', 3)
                                .arg(eulerDegrees.z(), 0, 'f', 3));
    appendScriptHistoryLine(QString("setAttr \"%1.scale\" %2 %3 %4;")
                                .arg(objectName)
                                .arg(transform.scale.x(), 0, 'f', 3)
                                .arg(transform.scale.y(), 0, 'f', 3)
                                .arg(transform.scale.z(), 0, 'f', 3));
    appendScriptHistoryLine(QString("// Result: updated %1 transform //").arg(objectName));
}

void MainWindow::logVisibilityChangeToScriptEditor(bool visible)
{
    const SceneObject* object = viewport_->findObject(selectedObjectId_);
    if (object == nullptr) {
        return;
    }

    const QString objectName = objectDisplayName(*object);
    appendScriptHistoryLine(QString("setAttr \"%1.visibility\" %2;").arg(objectName).arg(visible ? 1 : 0));
    appendScriptHistoryLine(QString("// Result: %1 visibility %2 //").arg(objectName, visible ? "on" : "off"));
}

void MainWindow::refreshScenePanels()
{
    if (markedHierarchyParentId_ != 0 && !viewport_->containsObject(markedHierarchyParentId_)) {
        markedHierarchyParentId_ = 0;
    }
    populateOutliner();
    clearInspector();
}

void MainWindow::restoreHistoryState(const EditorHistoryState& state)
{
    restoringHistory_ = true;
    viewport_->replaceScene(state.scene);
    setCurrentFrame(state.currentFrame, false);
    markedHierarchyParentId_ = state.markedHierarchyParentId;
    refreshScenePanels();
    if (state.selectedObjectId != 0 && viewport_->containsObject(state.selectedObjectId)) {
        selectObject(state.selectedObjectId, true);
    } else {
        clearInspector();
    }
    restoringHistory_ = false;
    updateUndoRedoActions();
}

void MainWindow::recordUndoState()
{
    historyController_.recordUndoState(
        historyController_.captureState(viewport_->scene(), selectedObjectId_, markedHierarchyParentId_, currentFrame_));
    updateUndoRedoActions();
}

void MainWindow::undoLastChange()
{
    EditorHistoryState previousState;
    if (!historyController_.tryTakeUndoState(
            historyController_.captureState(viewport_->scene(), selectedObjectId_, markedHierarchyParentId_, currentFrame_),
            &previousState)) {
        return;
    }

    restoreHistoryState(previousState);
}

void MainWindow::redoLastChange()
{
    EditorHistoryState nextState;
    if (!historyController_.tryTakeRedoState(
            historyController_.captureState(viewport_->scene(), selectedObjectId_, markedHierarchyParentId_, currentFrame_),
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
    outlinerTree_->clear();

    const QVector<SceneObject::Id> rootIds = viewport_->rootObjectIds();
    for (SceneObject::Id rootId : rootIds) {
        populateOutlinerItem(nullptr, rootId);
    }

    outlinerTree_->expandToDepth(1);

    if (outlinerTree_->topLevelItemCount() == 0) {
        QTreeWidgetItem* emptyItem = new QTreeWidgetItem();
        emptyItem->setText(0, "No scene loaded");
        outlinerTree_->addTopLevelItem(emptyItem);
    }
}

void MainWindow::populateOutlinerItem(QTreeWidgetItem* parentItem, std::uint64_t objectId)
{
    const SceneObject* object = viewport_->findObject(objectId);
    if (object == nullptr) {
        return;
    }

    if (shouldPromoteOutlinerNode(objectId)) {
        for (SceneObject::Id childId : object->childIds()) {
            populateOutlinerItem(parentItem, childId);
        }
        return;
    }

    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, objectDisplayName(*object));
    item->setData(0, Qt::UserRole, QVariant::fromValue<qulonglong>(objectId));

    if (parentItem == nullptr) {
        outlinerTree_->addTopLevelItem(item);
    } else {
        parentItem->addChild(item);
    }

    for (SceneObject::Id childId : object->childIds()) {
        populateOutlinerItem(item, childId);
    }
}

bool MainWindow::shouldPromoteOutlinerNode(std::uint64_t objectId) const
{
    const SceneObject* object = viewport_->findObject(objectId);
    if (object == nullptr) {
        return false;
    }

    return object->name() == "RootNode"
        && object->meshHandles().isEmpty()
        && object->childIds().size() == 1;
}

void MainWindow::clearInspector()
{
    selectedObjectId_ = 0;
    viewport_->setSelectedObject(0);
    syncOutlinerSelection(0);
    const bool hasScene = !viewport_->isSceneEmpty();
    if (hasScene) {
        inspectorEmptyStateLabel_->setText("No selection.");
    } else {
        inspectorEmptyStateLabel_->setText("No selection. Import an FBX to inspect the scene.");
    }

    updatingChannelBox_ = true;
    channelObjectNameLabel_->setText("-");
    translateXSpinBox_->setValue(0.0);
    translateYSpinBox_->setValue(0.0);
    translateZSpinBox_->setValue(0.0);
    rotateXSpinBox_->setValue(0.0);
    rotateYSpinBox_->setValue(0.0);
    rotateZSpinBox_->setValue(0.0);
    scaleXSpinBox_->setValue(1.0);
    scaleYSpinBox_->setValue(1.0);
    scaleZSpinBox_->setValue(1.0);
    jointOrientXSpinBox_->setValue(0.0);
    jointOrientYSpinBox_->setValue(0.0);
    jointOrientZSpinBox_->setValue(0.0);
    bindPoseStatusLabel_->setText("Bind pose: n/a");
    skinBindingStatusLabel_->setText("Skin binding: n/a");
    visibilityCheckBox_->setChecked(true);
    visibilityCheckBox_->setText("on");
    updatingChannelBox_ = false;
    setChannelBoxEnabled(false);
    if (setKeyButton_ != nullptr) {
        setKeyButton_->setEnabled(false);
    }
    if (deleteKeyButton_ != nullptr) {
        deleteKeyButton_->setEnabled(false);
    }
    if (markHierarchyParentAction_ != nullptr) {
        markHierarchyParentAction_->setEnabled(false);
    }
    if (parentToMarkedParentAction_ != nullptr) {
        parentToMarkedParentAction_->setEnabled(false);
    }
    if (unparentSelectedAction_ != nullptr) {
        unparentSelectedAction_->setEnabled(false);
    }
    if (bindSkinAction_ != nullptr) {
        bindSkinAction_->setEnabled(false);
    }
    if (resetJointOrientationAction_ != nullptr) {
        resetJointOrientationAction_->setEnabled(false);
    }
    if (alignJointOrientationAction_ != nullptr) {
        alignJointOrientationAction_->setEnabled(false);
    }
    if (captureBindPoseAction_ != nullptr) {
        captureBindPoseAction_->setEnabled(false);
    }
    if (captureBindPoseRecursiveAction_ != nullptr) {
        captureBindPoseRecursiveAction_->setEnabled(false);
    }
    refreshAnimationTimelineUi();
    frameSelectedButton_->setEnabled(false);
    frameSelectedAction_->setEnabled(false);
}

void MainWindow::updateInspector(std::uint64_t objectId)
{
    const SceneObject* object = viewport_->findObject(objectId);
    if (object == nullptr) {
        clearInspector();
        return;
    }

    selectedObjectId_ = objectId;
    viewport_->setSelectedObject(objectId);
    inspectorEmptyStateLabel_->setText("Selected object channel box.");
    updateChannelBox(objectId);

    const bool canFrame = object->isVisible() && object->worldBounds().isValid();
    if (setKeyButton_ != nullptr) {
        setKeyButton_->setEnabled(true);
    }
    if (deleteKeyButton_ != nullptr) {
        deleteKeyButton_->setEnabled(object->hasTransformKeyframe(currentFrame_));
    }
    if (markHierarchyParentAction_ != nullptr) {
        markHierarchyParentAction_->setEnabled(true);
    }
    if (parentToMarkedParentAction_ != nullptr) {
        parentToMarkedParentAction_->setEnabled(markedHierarchyParentId_ != 0 && markedHierarchyParentId_ != objectId);
    }
    if (unparentSelectedAction_ != nullptr) {
        unparentSelectedAction_->setEnabled(object->parentId() != 0);
    }
    if (bindSkinAction_ != nullptr) {
        const SceneObject* markedObject = markedHierarchyParentId_ == 0 ? nullptr : viewport_->findObject(markedHierarchyParentId_);
        bindSkinAction_->setEnabled(!object->meshHandles().isEmpty() && markedObject != nullptr && markedObject->isJoint() && markedHierarchyParentId_ != objectId);
    }
    if (resetJointOrientationAction_ != nullptr) {
        resetJointOrientationAction_->setEnabled(object->isJoint());
    }
    if (alignJointOrientationAction_ != nullptr) {
        alignJointOrientationAction_->setEnabled(object->isJoint() && !object->childIds().isEmpty());
    }
    if (captureBindPoseAction_ != nullptr) {
        captureBindPoseAction_->setEnabled(object->isJoint());
    }
    if (captureBindPoseRecursiveAction_ != nullptr) {
        captureBindPoseRecursiveAction_->setEnabled(object->isJoint());
    }
    refreshAnimationTimelineUi();
    frameSelectedButton_->setEnabled(canFrame);
    frameSelectedAction_->setEnabled(canFrame);
}

void MainWindow::handleOutlinerSelectionChanged()
{
    const QList<QTreeWidgetItem*> selectedItems = outlinerTree_->selectedItems();
    if (selectedItems.isEmpty()) {
        clearInspector();
        logSelectionToScriptEditor(0);
        return;
    }

    const QVariant objectIdData = selectedItems.first()->data(0, Qt::UserRole);
    if (!objectIdData.isValid()) {
        clearInspector();
        return;
    }

    const std::uint64_t objectId = objectIdData.toULongLong();
    selectObject(objectId, false);
    logSelectionToScriptEditor(objectId);
    statusBar()->showMessage(QString("Selected: %1").arg(selectedItems.first()->text(0)), 2000);
}

void MainWindow::frameSelectedObject()
{
    if (selectedObjectId_ == 0) {
        return;
    }

    viewport_->frameObject(selectedObjectId_);
    const SceneObject* object = viewport_->findObject(selectedObjectId_);
    if (object != nullptr) {
        appendScriptHistoryLine(QString("viewFit %1;").arg(objectDisplayName(*object)));
        appendScriptHistoryLine(QString("// Result: framed %1 //").arg(objectDisplayName(*object)));
    }
    statusBar()->showMessage("Selected object framed", 2000);
}

void MainWindow::selectObject(std::uint64_t objectId, bool syncOutliner)
{
    if (objectId == 0) {
        clearInspector();
        return;
    }

    if (syncOutliner) {
        syncOutlinerSelection(objectId);
    }

    updateInspector(objectId);
}

void MainWindow::syncOutlinerSelection(std::uint64_t objectId)
{
    QSignalBlocker blocker(outlinerTree_);

    if (objectId == 0) {
        outlinerTree_->clearSelection();
        return;
    }

    for (QTreeWidgetItemIterator it(outlinerTree_); *it != nullptr; ++it) {
        QTreeWidgetItem* item = *it;
        if (item->data(0, Qt::UserRole).toULongLong() == objectId) {
            outlinerTree_->setCurrentItem(item);
            item->setSelected(true);
            return;
        }
    }

    outlinerTree_->clearSelection();
}

void MainWindow::setTransformUiMode(TransformUiMode mode)
{
    translateAction_->setChecked(mode == TransformUiMode::Translate);
    rotateAction_->setChecked(mode == TransformUiMode::Rotate);
    scaleAction_->setChecked(mode == TransformUiMode::Scale);

    if (mode == TransformUiMode::Translate) {
        viewport_->setTransformMode(ViewportWidget::TransformMode::Translate);
        appendScriptHistoryLine("setToolTo MoveSuperContext;");
        appendScriptHistoryLine("// Result: move tool //");
        statusBar()->showMessage("Transform mode: Translate", 1500);
    } else if (mode == TransformUiMode::Rotate) {
        viewport_->setTransformMode(ViewportWidget::TransformMode::Rotate);
        appendScriptHistoryLine("setToolTo RotateSuperContext;");
        appendScriptHistoryLine("// Result: rotate tool //");
        statusBar()->showMessage("Transform mode: Rotate", 1500);
    } else {
        viewport_->setTransformMode(ViewportWidget::TransformMode::Scale);
        appendScriptHistoryLine("setToolTo ScaleSuperContext;");
        appendScriptHistoryLine("// Result: scale tool //");
        statusBar()->showMessage("Transform mode: Scale", 1500);
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

    perspectiveCameraAction_->setChecked(preset == ViewCameraUiPreset::Perspective);
    frontCameraAction_->setChecked(preset == ViewCameraUiPreset::Front);
    backCameraAction_->setChecked(preset == ViewCameraUiPreset::Back);
    leftCameraAction_->setChecked(preset == ViewCameraUiPreset::Left);
    rightCameraAction_->setChecked(preset == ViewCameraUiPreset::Right);
    topCameraAction_->setChecked(preset == ViewCameraUiPreset::Top);
    bottomCameraAction_->setChecked(preset == ViewCameraUiPreset::Bottom);

    QString label = "Perspective";
    ViewportWidget::CameraViewPreset viewportPreset = ViewportWidget::CameraViewPreset::Perspective;
    switch (preset) {
    case ViewCameraUiPreset::Front:
        label = "Front";
        viewportPreset = ViewportWidget::CameraViewPreset::Front;
        break;
    case ViewCameraUiPreset::Back:
        label = "Back";
        viewportPreset = ViewportWidget::CameraViewPreset::Back;
        break;
    case ViewCameraUiPreset::Left:
        label = "Left";
        viewportPreset = ViewportWidget::CameraViewPreset::Left;
        break;
    case ViewCameraUiPreset::Right:
        label = "Right";
        viewportPreset = ViewportWidget::CameraViewPreset::Right;
        break;
    case ViewCameraUiPreset::Top:
        label = "Top";
        viewportPreset = ViewportWidget::CameraViewPreset::Top;
        break;
    case ViewCameraUiPreset::Bottom:
        label = "Bottom";
        viewportPreset = ViewportWidget::CameraViewPreset::Bottom;
        break;
    case ViewCameraUiPreset::Perspective:
        break;
    }

    viewport_->setCameraViewPreset(viewportPreset);
    appendScriptComment(QString("Camera preset: %1").arg(label));
    statusBar()->showMessage(QString("Camera view: %1").arg(label), 1500);
}

void MainWindow::setAxisUiOrientation(AxisUiOrientation orientation)
{
    worldAxisAction_->setChecked(orientation == AxisUiOrientation::World);
    localAxisAction_->setChecked(orientation == AxisUiOrientation::Local);

    if (orientation == AxisUiOrientation::World) {
        viewport_->setAxisOrientation(ViewportWidget::AxisOrientation::World);
        appendScriptComment("Axis orientation set to World");
        statusBar()->showMessage("Axis orientation: World", 1500);
    } else {
        viewport_->setAxisOrientation(ViewportWidget::AxisOrientation::Local);
        appendScriptComment("Axis orientation set to Local");
        statusBar()->showMessage("Axis orientation: Local", 1500);
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
    appendScriptComment("Workspace layout restored");
    statusBar()->showMessage("Workspace layout restored", 2000);
}

void MainWindow::showPolygonPrimitivesWindow()
{
    if (polygonPrimitivesDock_ == nullptr) {
        return;
    }

    polygonPrimitivesDock_->show();
    polygonPrimitivesDock_->raise();
    appendScriptComment("Polygon Primitives window opened");
}

void MainWindow::updateWindowTitle()
{
    const QString sceneName = currentSceneFilePath_.isEmpty()
        ? "untitled"
        : QFileInfo(currentSceneFilePath_).fileName();
    setWindowTitle(QString("%1 - Phoenix Editor Beta").arg(sceneName));
}

void MainWindow::setCurrentFrame(int frame, bool logToScript)
{
    const int clampedFrame = qBound(playbackStartFrame_, frame, playbackEndFrame_);
    currentFrame_ = clampedFrame;
    viewport_->setCurrentFrame(clampedFrame);

    updatingTimeSlider_ = true;
    if (timeSlider_ != nullptr) {
        timeSlider_->setValue(clampedFrame);
    }
    if (currentFrameSpinBox_ != nullptr) {
        currentFrameSpinBox_->setValue(clampedFrame);
    }
    updatingTimeSlider_ = false;

    refreshAnimationTimelineUi();

    if (selectedObjectId_ != 0 && viewport_->containsObject(selectedObjectId_)) {
        updateChannelBox(selectedObjectId_);
        const SceneObject* object = viewport_->findObject(selectedObjectId_);
        const bool canFrame = object != nullptr && object->isVisible() && object->worldBounds().isValid();
        frameSelectedButton_->setEnabled(canFrame);
        frameSelectedAction_->setEnabled(canFrame);
    }

    if (logToScript) {
        appendScriptHistoryLine(QString("currentTime %1;").arg(currentFrame_));
        appendScriptHistoryLine(QString("// Result: current frame %1 //").arg(currentFrame_));
    }

    statusBar()->showMessage(QString("Current frame: %1").arg(currentFrame_), 800);
}

void MainWindow::setKeyForSelection(bool logToScript)
{
    if (selectedObjectId_ == 0) {
        statusBar()->showMessage("Select an object to key", 1500);
        return;
    }

    const SceneObject* object = viewport_->findObject(selectedObjectId_);
    if (object == nullptr) {
        statusBar()->showMessage("Selected object is no longer available", 1500);
        return;
    }

    if (!viewport_->setObjectKeyframe(selectedObjectId_, currentFrame_)) {
        qCWarning(logAnimation) << "set key failed:" << "objectId=" << selectedObjectId_ << "frame=" << currentFrame_;
        statusBar()->showMessage("Set key failed", 1500);
        return;
    }

    qCInfo(logAnimation) << "set key:"
            << "object=" << objectDisplayName(*object)
            << "objectId=" << selectedObjectId_
            << "frame=" << currentFrame_;

    refreshAnimationTimelineUi();
    updateChannelBox(selectedObjectId_);

    if (logToScript) {
        appendScriptHistoryLine(QString("setKeyframe %1 -t %2;").arg(objectDisplayName(*object)).arg(currentFrame_));
        appendScriptHistoryLine(QString("// Result: key set on %1 at frame %2 //").arg(objectDisplayName(*object)).arg(currentFrame_));
    }

    statusBar()->showMessage(QString("Key set at frame %1").arg(currentFrame_), 1500);
}

void MainWindow::deleteKeyForSelection(bool logToScript)
{
    if (selectedObjectId_ == 0) {
        statusBar()->showMessage("Select an object to delete a key", 1500);
        return;
    }

    const SceneObject* object = viewport_->findObject(selectedObjectId_);
    if (object == nullptr) {
        statusBar()->showMessage("Selected object is no longer available", 1500);
        return;
    }

    if (!object->hasTransformKeyframe(currentFrame_)) {
        statusBar()->showMessage(QString("No key at frame %1").arg(currentFrame_), 1500);
        return;
    }

    const QString objectName = objectDisplayName(*object);
    if (!viewport_->removeObjectKeyframe(selectedObjectId_, currentFrame_)) {
        qCWarning(logAnimation) << "delete key failed:" << "objectId=" << selectedObjectId_ << "frame=" << currentFrame_;
        statusBar()->showMessage("Delete key failed", 1500);
        return;
    }

    qCInfo(logAnimation) << "delete key:"
            << "object=" << objectName
            << "objectId=" << selectedObjectId_
            << "frame=" << currentFrame_;

    refreshAnimationTimelineUi();
    updateChannelBox(selectedObjectId_);

    if (logToScript) {
        appendScriptHistoryLine(QString("cutKey %1 -t %2;").arg(objectName).arg(currentFrame_));
        appendScriptHistoryLine(QString("// Result: deleted key on %1 at frame %2 //").arg(objectName).arg(currentFrame_));
    }

    statusBar()->showMessage(QString("Deleted key at frame %1").arg(currentFrame_), 1500);
}

void MainWindow::duplicateCurrentKeyForSelection(bool logToScript)
{
    if (selectedObjectId_ == 0) {
        statusBar()->showMessage("Select an object to duplicate a key", 1500);
        return;
    }

    const SceneObject* object = viewport_->findObject(selectedObjectId_);
    if (object == nullptr) {
        statusBar()->showMessage("Selected object is no longer available", 1500);
        return;
    }

    if (!object->hasTransformKeyframe(currentFrame_)) {
        statusBar()->showMessage(QString("No key at frame %1 to duplicate").arg(currentFrame_), 1500);
        return;
    }

    const std::uint64_t objectId = selectedObjectId_;
    const QString objectName = objectDisplayName(*object);
    const int sourceFrame = currentFrame_;
    const int targetFrame = currentFrame_ + 1;
    Scene updatedScene = viewport_->sceneSnapshot();
    if (!updatedScene.duplicateObjectKeyframe(objectId, sourceFrame, targetFrame)) {
        qCWarning(logAnimation) << "duplicate key failed:"
                << "objectId=" << objectId
                << "sourceFrame=" << sourceFrame
                << "targetFrame=" << targetFrame;
        statusBar()->showMessage("Duplicate key failed", 1500);
        return;
    }

    recordUndoState();
    viewport_->replaceScene(updatedScene);
    refreshScenePanels();
    selectObject(objectId, true);
    setCurrentFrame(targetFrame, false);

    qCInfo(logAnimation) << "duplicate key:"
            << "object=" << objectName
            << "objectId=" << objectId
            << "sourceFrame=" << sourceFrame
            << "targetFrame=" << targetFrame;

    if (logToScript) {
        appendScriptHistoryLine(QString("copyKey %1 -t %2 -to %3;").arg(objectName).arg(sourceFrame).arg(targetFrame));
        appendScriptHistoryLine(QString("// Result: copied key on %1 from frame %2 to %3 //").arg(objectName).arg(sourceFrame).arg(targetFrame));
    }

    statusBar()->showMessage(QString("Duplicated key to frame %1").arg(targetFrame), 1500);
}

void MainWindow::shiftSelectedObjectKeyframes(int frameDelta, bool logToScript)
{
    if (selectedObjectId_ == 0) {
        statusBar()->showMessage("Select an object to shift keys", 1500);
        return;
    }

    const SceneObject* object = viewport_->findObject(selectedObjectId_);
    if (object == nullptr) {
        statusBar()->showMessage("Selected object is no longer available", 1500);
        return;
    }

    if (object->transformKeyframes().isEmpty()) {
        statusBar()->showMessage("Selected object has no keys to shift", 1500);
        return;
    }

    const std::uint64_t objectId = selectedObjectId_;
    const QString objectName = objectDisplayName(*object);
    Scene updatedScene = viewport_->sceneSnapshot();
    if (!updatedScene.offsetObjectKeyframes(objectId, frameDelta)) {
        qCWarning(logAnimation) << "shift keys failed:" << "objectId=" << objectId << "delta=" << frameDelta;
        statusBar()->showMessage("Shift keys failed", 1500);
        return;
    }

    recordUndoState();
    viewport_->replaceScene(updatedScene);
    refreshScenePanels();
    selectObject(objectId, true);
    setCurrentFrame(currentFrame_ + frameDelta, false);

    qCInfo(logAnimation) << "shift keys:"
            << "object=" << objectName
            << "objectId=" << objectId
            << "delta=" << frameDelta
            << "newCurrentFrame=" << (currentFrame_ + frameDelta);

    if (logToScript) {
        appendScriptHistoryLine(QString("shiftKey %1 -by %2;").arg(objectName).arg(frameDelta));
        appendScriptHistoryLine(QString("// Result: shifted keys on %1 by %2 //").arg(objectName).arg(frameDelta));
    }

    statusBar()->showMessage(QString("Shifted keys by %1").arg(frameDelta), 1500);
}

void MainWindow::setAutoKeyEnabled(bool enabled, bool logToScript)
{
    autoKeyEnabled_ = enabled;
    viewport_->setAutoKeyEnabled(enabled);
    refreshAnimationTimelineUi();

    if (logToScript) {
        appendScriptHistoryLine(QString("autoKeyframe -state %1;").arg(enabled ? "on" : "off"));
        appendScriptHistoryLine(QString("// Result: auto key %1 //").arg(enabled ? "on" : "off"));
    }

    statusBar()->showMessage(enabled ? "Auto Key enabled" : "Auto Key disabled", 1500);
}

void MainWindow::setPlaybackRange(int startFrame, int endFrame, bool logToScript)
{
    if (startFrame > endFrame) {
        std::swap(startFrame, endFrame);
    }

    playbackStartFrame_ = startFrame;
    playbackEndFrame_ = endFrame;

    updatingTimeSlider_ = true;
    if (playbackStartSpinBox_ != nullptr) {
        playbackStartSpinBox_->setValue(playbackStartFrame_);
    }
    if (playbackEndSpinBox_ != nullptr) {
        playbackEndSpinBox_->setValue(playbackEndFrame_);
    }
    if (timeSlider_ != nullptr) {
        timeSlider_->setRange(playbackStartFrame_, playbackEndFrame_);
    }
    if (currentFrameSpinBox_ != nullptr) {
        currentFrameSpinBox_->setRange(playbackStartFrame_, playbackEndFrame_);
    }
    updatingTimeSlider_ = false;

    if (logToScript) {
        appendScriptHistoryLine(QString("playbackOptions -min %1 -max %2;").arg(playbackStartFrame_).arg(playbackEndFrame_));
        appendScriptHistoryLine(QString("// Result: playback range %1 to %2 //").arg(playbackStartFrame_).arg(playbackEndFrame_));
    }

    setCurrentFrame(currentFrame_, logToScript);
}

void MainWindow::stepFrame(int delta)
{
    setCurrentFrame(currentFrame_ + delta);
}

void MainWindow::jumpToSelectedObjectKeyframe(bool forward, bool logToScript)
{
    if (selectedObjectId_ == 0) {
        statusBar()->showMessage("Select an object to jump keys", 1500);
        return;
    }

    const SceneObject* object = viewport_->findObject(selectedObjectId_);
    if (object == nullptr) {
        statusBar()->showMessage("Selected object is no longer available", 1500);
        return;
    }

    const int targetFrame = forward
        ? viewport_->nextObjectKeyframe(selectedObjectId_, currentFrame_)
        : viewport_->previousObjectKeyframe(selectedObjectId_, currentFrame_);
    if (targetFrame == currentFrame_) {
        qCInfo(logAnimation) << "jump key skipped:"
                << "object=" << objectDisplayName(*object)
                << "objectId=" << selectedObjectId_
                << "direction=" << (forward ? "next" : "previous")
                << "frame=" << currentFrame_;
        statusBar()->showMessage(forward ? "No next key" : "No previous key", 1200);
        return;
    }

    setCurrentFrame(targetFrame, false);

    qCInfo(logAnimation) << "jump key:"
            << "object=" << objectDisplayName(*object)
            << "objectId=" << selectedObjectId_
            << "direction=" << (forward ? "next" : "previous")
            << "targetFrame=" << targetFrame;

    if (logToScript) {
        appendScriptComment(QString("%1 key on %2 -> frame %3").arg(forward ? "next" : "previous", objectDisplayName(*object)).arg(targetFrame));
    }

    statusBar()->showMessage(QString("Jumped to frame %1").arg(targetFrame), 1200);
}

void MainWindow::togglePlayback()
{
    if (playbackTimer_ == nullptr || playPauseButton_ == nullptr) {
        return;
    }

    if (playbackTimer_->isActive()) {
        playbackTimer_->stop();
        playPauseButton_->setText(">");
        appendScriptHistoryLine("play -state off;");
        appendScriptHistoryLine("// Result: playback stopped //");
        statusBar()->showMessage("Playback stopped", 1000);
    } else {
        playbackTimer_->start();
        playPauseButton_->setText("||");
        appendScriptHistoryLine("play -state on;");
        appendScriptHistoryLine("// Result: playback started //");
        statusBar()->showMessage("Playback started", 1000);
    }
}

void MainWindow::advancePlayback()
{
    const int nextFrame = currentFrame_ >= playbackEndFrame_ ? playbackStartFrame_ : currentFrame_ + 1;
    setCurrentFrame(nextFrame, false);
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

    const Transform& transform = object->localTransform();
    const QVector3D eulerDegrees = transform.rotation.toEulerAngles();
    const QVector3D jointOrientEuler = object->jointOrientation().toEulerAngles();
    const bool isJoint = object->isJoint();

    updatingChannelBox_ = true;
    channelObjectNameLabel_->setText(objectDisplayName(*object));
    translateXSpinBox_->setValue(transform.translation.x());
    translateYSpinBox_->setValue(transform.translation.y());
    translateZSpinBox_->setValue(transform.translation.z());
    rotateXSpinBox_->setValue(eulerDegrees.x());
    rotateYSpinBox_->setValue(eulerDegrees.y());
    rotateZSpinBox_->setValue(eulerDegrees.z());
    scaleXSpinBox_->setValue(transform.scale.x());
    scaleYSpinBox_->setValue(transform.scale.y());
    scaleZSpinBox_->setValue(transform.scale.z());
    jointOrientXSpinBox_->setValue(isJoint ? jointOrientEuler.x() : 0.0);
    jointOrientYSpinBox_->setValue(isJoint ? jointOrientEuler.y() : 0.0);
    jointOrientZSpinBox_->setValue(isJoint ? jointOrientEuler.z() : 0.0);
    bindPoseStatusLabel_->setText(formatBindPoseStatus(*object));
    skinBindingStatusLabel_->setText(formatSkinBindingStatus(*object));
    visibilityCheckBox_->setChecked(object->isVisible());
    visibilityCheckBox_->setText(object->isVisible() ? "on" : "off");
    updatingChannelBox_ = false;
    jointToolsWidget_->setEnabled(isJoint);
    resetJointOrientationButton_->setEnabled(isJoint);
    alignJointOrientationButton_->setEnabled(isJoint && !object->childIds().isEmpty());
    captureBindPoseButton_->setEnabled(isJoint);
    captureBindPoseRecursiveButton_->setEnabled(isJoint);
    setChannelBoxEnabled(true);
    refreshAnimationTimelineUi();
}

void MainWindow::refreshAnimationTimelineUi()
{
    QVector<int> keyframes;
    bool hasSelection = false;
    bool currentFrameKeyed = false;
    bool hasAnyKeys = false;
    QString objectName = "No selection";

    if (selectedObjectId_ != 0) {
        const SceneObject* object = viewport_->findObject(selectedObjectId_);
        if (object != nullptr) {
            hasSelection = true;
            objectName = objectDisplayName(*object);
            const TransformKeyframeTrack& track = object->transformKeyframes();
            hasAnyKeys = !track.isEmpty();
            keyframes.reserve(track.size());
            for (const TransformKeyframe& keyframe : track) {
                keyframes.append(keyframe.frame);
                if (keyframe.frame == currentFrame_) {
                    currentFrameKeyed = true;
                }
            }
        }
    }

    if (keyframeTimelineWidget_ != nullptr) {
        keyframeTimelineWidget_->setFrameRange(playbackStartFrame_, playbackEndFrame_);
        keyframeTimelineWidget_->setCurrentFrame(currentFrame_);
        keyframeTimelineWidget_->setKeyframes(keyframes);
        keyframeTimelineWidget_->setCurrentFrameKeyed(currentFrameKeyed);
    }

    if (setKeyButton_ != nullptr) {
        setKeyButton_->setText(currentFrameKeyed ? "Key Selected" : "Key Selected");
        setKeyButton_->setStyleSheet(currentFrameKeyed
                ? "QPushButton { background-color: #b86d1f; color: white; font-weight: 600; }"
                : "QPushButton { background-color: #4a4a4a; color: white; }");
        if (!hasSelection) {
            setKeyButton_->setText("Key Selected");
            setKeyButton_->setStyleSheet(QString());
        }
    }

    if (deleteKeyButton_ != nullptr) {
        deleteKeyButton_->setEnabled(hasSelection && currentFrameKeyed);
        deleteKeyButton_->setStyleSheet(currentFrameKeyed
                ? "QPushButton { background-color: #565656; color: white; }"
                : QString());
    }

    if (duplicateKeyButton_ != nullptr) {
        duplicateKeyButton_->setEnabled(hasSelection && currentFrameKeyed);
    }

    if (shiftKeysLeftButton_ != nullptr) {
        shiftKeysLeftButton_->setEnabled(hasSelection && hasAnyKeys);
    }

    if (shiftKeysRightButton_ != nullptr) {
        shiftKeysRightButton_->setEnabled(hasSelection && hasAnyKeys);
    }

    if (previousKeyButton_ != nullptr) {
        previousKeyButton_->setEnabled(hasSelection && hasAnyKeys);
    }

    if (nextKeyButton_ != nullptr) {
        nextKeyButton_->setEnabled(hasSelection && hasAnyKeys);
    }

    if (duplicateKeyAction_ != nullptr) {
        duplicateKeyAction_->setEnabled(hasSelection && currentFrameKeyed);
    }

    if (shiftKeysLeftAction_ != nullptr) {
        shiftKeysLeftAction_->setEnabled(hasSelection && hasAnyKeys);
    }

    if (shiftKeysRightAction_ != nullptr) {
        shiftKeysRightAction_->setEnabled(hasSelection && hasAnyKeys);
    }

    if (previousKeyAction_ != nullptr) {
        previousKeyAction_->setEnabled(hasSelection && hasAnyKeys);
    }

    if (nextKeyAction_ != nullptr) {
        nextKeyAction_->setEnabled(hasSelection && hasAnyKeys);
    }

    if (autoKeyButton_ != nullptr) {
        if (autoKeyButton_->isChecked() != autoKeyEnabled_) {
            autoKeyButton_->setChecked(autoKeyEnabled_);
        }
        autoKeyButton_->setStyleSheet(autoKeyEnabled_
                ? "QPushButton { background-color: #8f2424; color: white; font-weight: 600; }"
                : QString());
    }

    if (timelineStatusLabel_ != nullptr) {
        if (!hasSelection) {
            timelineStatusLabel_->setText(autoKeyEnabled_ ? "No selection | Auto Key on" : "No selection");
            timelineStatusLabel_->setStyleSheet("color: #bdbdbd;");
        } else if (currentFrameKeyed) {
            timelineStatusLabel_->setText(QString("%1 | %2 keys | frame %3 keyed%4")
                    .arg(objectName)
                    .arg(keyframes.size())
                    .arg(currentFrame_)
                    .arg(autoKeyEnabled_ ? " | Auto Key on" : ""));
            timelineStatusLabel_->setStyleSheet("color: #ffb040; font-weight: 600;");
        } else if (!keyframes.isEmpty()) {
            timelineStatusLabel_->setText(QString("%1 | %2 keys | frame %3 has no key%4")
                    .arg(objectName)
                    .arg(keyframes.size())
                    .arg(currentFrame_)
                    .arg(autoKeyEnabled_ ? " | Auto Key on" : ""));
            timelineStatusLabel_->setStyleSheet("color: #9fdcff;");
        } else {
            timelineStatusLabel_->setText(QString("%1 | no keys yet%2")
                    .arg(objectName)
                    .arg(autoKeyEnabled_ ? " | Auto Key on" : ""));
            timelineStatusLabel_->setStyleSheet("color: #bdbdbd;");
        }
    }
}

void MainWindow::setChannelBoxEnabled(bool enabled)
{
    inspectorDetailsWidget_->setEnabled(enabled);
}

void MainWindow::applyChannelBoxToSelection()
{
    if (updatingChannelBox_ || selectedObjectId_ == 0) {
        return;
    }

    const SceneObject* object = viewport_->findObject(selectedObjectId_);
    if (object == nullptr) {
        return;
    }

    Transform transform = object->localTransform();
    transform.translation = QVector3D(
        static_cast<float>(translateXSpinBox_->value()),
        static_cast<float>(translateYSpinBox_->value()),
        static_cast<float>(translateZSpinBox_->value()));
    transform.rotation = QQuaternion::fromEulerAngles(
        static_cast<float>(rotateXSpinBox_->value()),
        static_cast<float>(rotateYSpinBox_->value()),
        static_cast<float>(rotateZSpinBox_->value()));
    transform.scale = QVector3D(
        static_cast<float>(scaleXSpinBox_->value()),
        static_cast<float>(scaleYSpinBox_->value()),
        static_cast<float>(scaleZSpinBox_->value()));

    if (!viewport_->setObjectLocalTransform(selectedObjectId_, transform)) {
        return;
    }

    updateChannelBox(selectedObjectId_);
    logChannelBoxChangeToScriptEditor(transform);
    statusBar()->showMessage("Channel Box updated", 1200);
}

void MainWindow::applyJointOrientationToSelection()
{
    if (updatingChannelBox_ || selectedObjectId_ == 0) {
        return;
    }

    const SceneObject* object = viewport_->findObject(selectedObjectId_);
    if (object == nullptr || !object->isJoint()) {
        return;
    }

    const QVector3D eulerDegrees(
        static_cast<float>(jointOrientXSpinBox_->value()),
        static_cast<float>(jointOrientYSpinBox_->value()),
        static_cast<float>(jointOrientZSpinBox_->value()));

    if (!viewport_->setJointOrientation(selectedObjectId_, QQuaternion::fromEulerAngles(eulerDegrees))) {
        statusBar()->showMessage("Joint orientation update failed", 1200);
        return;
    }

    updateChannelBox(selectedObjectId_);
    appendScriptHistoryLine(QString("jointOrient %1 -euler %2 %3 %4;")
            .arg(objectDisplayName(*object))
            .arg(jointOrientXSpinBox_->value(), 0, 'f', 3)
            .arg(jointOrientYSpinBox_->value(), 0, 'f', 3)
            .arg(jointOrientZSpinBox_->value(), 0, 'f', 3));
    appendScriptHistoryLine(QString("// Result: joint orientation updated on %1 //").arg(objectDisplayName(*object)));
    statusBar()->showMessage("Joint orientation updated", 1200);
}

void MainWindow::applyVisibilityToSelection(bool visible)
{
    if (updatingChannelBox_ || selectedObjectId_ == 0) {
        return;
    }

    if (!viewport_->setObjectVisibility(selectedObjectId_, visible)) {
        return;
    }

    visibilityCheckBox_->setText(visible ? "on" : "off");
    const SceneObject* object = viewport_->findObject(selectedObjectId_);
    const bool canFrame = object != nullptr && object->isVisible() && object->worldBounds().isValid();
    frameSelectedButton_->setEnabled(canFrame);
    frameSelectedAction_->setEnabled(canFrame);
    logVisibilityChangeToScriptEditor(visible);
    statusBar()->showMessage(visible ? "Visibility on" : "Visibility off", 1200);
}
