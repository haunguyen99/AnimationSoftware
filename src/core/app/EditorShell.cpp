#include "EditorShell.h"

#include <QAction>
#include <QApplication>
#include <QDockWidget>
#include <QFileDialog>
#include <QFormLayout>
#include <QInputDialog>
#include <QLineEdit>
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
#include <QSignalBlocker>
#include <QSlider>
#include <QSpinBox>
#include <QStatusBar>
#include <QStyle>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>
#include <QTreeWidgetItem>
#include <QHBoxLayout>
#include <QFrame>
#include <QVBoxLayout>

#include <algorithm>

#include "AnimationTimelinePanel.h"
#include "animation/editor/EditorAnimationFlowController.h"
#include "EditorDocumentController.h"
#include "EditorChannelBoxController.h"
#include "EditorFileFlowController.h"
#include "EditorInspectorController.h"
#include "EditorOutlinerController.h"
#include "rigging/editor/EditorRiggingController.h"
#include "EditorScriptExecutionController.h"
#include "EditorSelectionController.h"
#include "EditorViewportUiController.h"
#include "EditorScriptLogController.h"
#include "EditorSceneQueryController.h"
#include "EditorSceneMutationController.h"
#include "EditorViewportCommandController.h"
#include "GraphEditorPanel.h"
#include "RangeSliderPanel.h"
#include "core/app/EditorShellContexts.h"
#include "core/app/EditorMenuBar.h"
#include "core/app/EditorTheme.h"
#include "core/app/EditorToolBar.h"
#include "core/app/WorkspaceManager.h"
#include "ViewportWorkspaceWidget.h"
#include "core/settings/EditorPreferences.h"
#include "io/PhoenixSceneDocument.h"
#include "core/logging/LogCategories.h"
#include "scene/PrimitiveMeshFactory.h"
#include "scene/Scene.h"
#include "scene/SceneObject.h"

namespace
{
constexpr auto kPrimitiveTypeRole = Qt::UserRole + 101;
using EditorSceneQueryController::objectDisplayName;

QLabel* createChannelRowLabel(const QString& text, QWidget* parent)
{
    QLabel* label = new QLabel(text, parent);
    label->setObjectName("channelRowLabel");
    label->setMinimumWidth(78);
    return label;
}

QWidget* createChannelRow(const QString& labelText, QWidget* fieldWidget, QWidget* parent)
{
    QWidget* row = new QWidget(parent);
    QHBoxLayout* layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);
    layout->addWidget(createChannelRowLabel(labelText, row));
    layout->addWidget(fieldWidget, 1);
    return row;
}

QWidget* createChannelSection(const QString& title, QWidget* parent, QVBoxLayout** contentLayout)
{
    QWidget* section = new QWidget(parent);
    section->setObjectName("channelBoxSection");

    QVBoxLayout* outerLayout = new QVBoxLayout(section);
    outerLayout->setContentsMargins(10, 8, 10, 10);
    outerLayout->setSpacing(8);

    QLabel* titleLabel = new QLabel(title, section);
    titleLabel->setObjectName("channelSectionLabel");
    outerLayout->addWidget(titleLabel);

    QVBoxLayout* bodyLayout = new QVBoxLayout();
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(6);
    outerLayout->addLayout(bodyLayout);

    if (contentLayout != nullptr) {
        *contentLayout = bodyLayout;
    }

    return section;
}

QDoubleSpinBox* createChannelSpinBox(QWidget* parent)
{
    QDoubleSpinBox* spinBox = new QDoubleSpinBox(parent);
    spinBox->setDecimals(3);
    spinBox->setRange(-999999.0, 999999.0);
    spinBox->setSingleStep(0.1);
    spinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    spinBox->setAlignment(Qt::AlignRight);
    spinBox->setMinimumHeight(22);
    spinBox->setProperty("variant", "channelBox");
    return spinBox;
}

}

EditorShell::EditorShell()
{
    setWindowTitle("Phoenix Editor Beta");
    resize(1440, 820);
    EditorTheme::apply(this);
    setDockNestingEnabled(true);
    setDockOptions(QMainWindow::AllowNestedDocks
        | QMainWindow::AllowTabbedDocks
        | QMainWindow::GroupedDragging
        | QMainWindow::AnimatedDocks);

    viewport_ = new ViewportWorkspaceWidget(this);
    playbackController_.bind(EditorPlaybackController::Context {
        [this]() {
            return animationState_;
        },
        [this](const EditorAnimationFlowController::OperationResult& result) {
            applyAnimationFlowResult(result);
        },
    });
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

    workspaceManager_ = new WorkspaceManager(this, this);

    editorMenuBar_ = new EditorMenuBar(this, this);
    editorMenuBar_->build(EditorMenuBar::Bindings {
        // File
        .newScene                   = [this]() { newScene(); },
        .openScene                  = [this]() { openScene(); },
        .saveScene                  = [this]() { saveScene(); },
        .saveSceneAs                = [this]() { saveSceneAs(); },
        .incrementAndSave           = [this]() { incrementAndSave(); },
        .archiveScene               = [this]() { archiveScene(); },
        .savePreferences            = [this]() { savePreferences(); },
        .optimizeSceneSize          = [this]() { optimizeSceneStorage(); },
        .importFbx                  = [this]() { importFbx(); },
        .exportAll                  = [this]() { exportAll(); },
        .exportSelection            = [this]() { exportSelection(); },
        // Edit
        .undo                       = [this]() { undoLastChange(); },
        .redo                       = [this]() { redoLastChange(); },
        // Create
        .showPolygonPrimitives      = [this]() { showPolygonPrimitivesWindow(); },
        .createJoint                = [this]() { createJoint(); },
        // Rig
        .markHierarchyParent        = [this]() { markSelectionAsHierarchyParent(); },
        .parentToMarkedParent       = [this]() { parentSelectionToMarkedParent(); },
        .unparentSelected           = [this]() { unparentSelection(); },
        .bindSkin                   = [this]() { bindSelectedMeshToMarkedJoint(); },
        .resetJointOrientation      = [this]() { resetSelectedJointOrientation(); },
        .alignJointOrientation      = [this]() { alignSelectedJointOrientationToChild(); },
        .captureBindPose            = [this]() { captureSelectedBindPose(); },
        .captureBindPoseRecursive   = [this]() { captureSelectedBindPoseRecursive(); },
        // Animation
        .duplicateKey               = [this]() { duplicateCurrentKeyForSelection(true); },
        .shiftKeysLeft              = [this]() { shiftSelectedObjectKeyframes(-1, true); },
        .shiftKeysRight             = [this]() { shiftSelectedObjectKeyframes(1, true); },
        .previousKey                = [this]() { jumpToSelectedObjectKeyframe(false, true); },
        .nextKey                    = [this]() { jumpToSelectedObjectKeyframe(true, true); },
        // View
        .resetCamera                = [this]() { resetSceneCamera(); },
        .frameScene                 = [this]() { frameEntireScene(); },
        .frameSelected              = [this]() { frameSelectedObject(); },
        .setWireframe               = [this](bool on) { setWireframeDisplayEnabled(on); },
        .setAxisVisibility          = [this](bool on) { setAxisVisibilityEnabled(on); },
        .setBackfaceCulling         = [this](bool on) { setBackfaceCullingEnabled(on); },
        .setCameraPerspective       = [this]() { setViewCameraPreset(ViewCameraUiPreset::Perspective); },
        .setCameraFront             = [this]() { setViewCameraPreset(ViewCameraUiPreset::Front); },
        .setCameraBack              = [this]() { setViewCameraPreset(ViewCameraUiPreset::Back); },
        .setCameraLeft              = [this]() { setViewCameraPreset(ViewCameraUiPreset::Left); },
        .setCameraRight             = [this]() { setViewCameraPreset(ViewCameraUiPreset::Right); },
        .setCameraTop               = [this]() { setViewCameraPreset(ViewCameraUiPreset::Top); },
        .setCameraBottom            = [this]() { setViewCameraPreset(ViewCameraUiPreset::Bottom); },
        .restoreDefaultLayout       = [this]() { restoreDefaultWorkspaceLayout(); },
        // Transform
        .setTranslate               = [this]() { setTransformUiMode(TransformUiMode::Translate); },
        .setRotate                  = [this]() { setTransformUiMode(TransformUiMode::Rotate); },
        .setScale                   = [this]() { setTransformUiMode(TransformUiMode::Scale); },
        .setWorldAxis               = [this]() { setAxisUiOrientation(AxisUiOrientation::World); },
        .setLocalAxis               = [this]() { setAxisUiOrientation(AxisUiOrientation::Local); },
        // Windows
        .showScriptEditor           = [this]() { showScriptEditorWindow(); },
        .showGraphEditor            = [this]() { showGraphEditorWindow(); },
        .workspaceManager           = workspaceManager_,
        .saveWorkspaceLayout        = [this](const QString& name) {
            workspaceManager_->saveUserLayout(name);
            statusBar()->showMessage(QString("Layout '%1' saved").arg(name), 2000);
            editorMenuBar_->refreshUserLayoutMenu();
        },
        .deleteWorkspaceLayout      = [this](const QString& name) {
            workspaceManager_->deleteUserLayout(name);
            statusBar()->showMessage(QString("Layout '%1' deleted").arg(name), 2000);
            editorMenuBar_->refreshUserLayoutMenu();
        },
        .userLayoutNames            = [this]() { return workspaceManager_->userLayoutNames(); },
        .restoreUserLayout          = [this](const QString& name) { workspaceManager_->restoreUserLayout(name); },
        .showStatusMessage          = [this](const QString& msg, int ms) { statusBar()->showMessage(msg, ms); },
    });
    {
        auto result = EditorToolBar::build(this, editorMenuBar_->actions());
        toolbar_ = result.toolbar;
        if (result.presetButton != nullptr) {
            connect(workspaceManager_, &WorkspaceManager::activePresetChanged,
                    this, [btn = result.presetButton](const QString& name) {
                        btn->setText(name);
                    });
        }
    }

    createDocks();
    activePanelStatusLabel_ = new QLabel(this);
    activePanelStatusLabel_->setObjectName("activePanelStatusLabel");
    statusBar()->addPermanentWidget(activePanelStatusLabel_);
    loadPreferences();
    updateWindowTitle();
    clearInspector();
    updateUndoRedoActions();
    applyAnimationState(animationState_);
    refreshActivePanelVisuals();
    statusBar()->showMessage("Ready");
}

bool EditorShell::eventFilter(QObject* watched, QEvent* event)
{
    if (watched != nullptr && event != nullptr) {
        const QVariant panelValue = watched->property("editorActivePanel");
        if (panelValue.isValid()) {
            const auto panel = static_cast<ActivePanel>(panelValue.toInt());
            if (event->type() == QEvent::MouseButtonPress
                || event->type() == QEvent::FocusIn
                || event->type() == QEvent::WindowActivate) {
                setActivePanel(panel);
            }
            if (event->type() == QEvent::MouseButtonDblClick) {
                auto* mouseEvent = static_cast<QMouseEvent*>(event);
                if (mouseEvent->button() == Qt::LeftButton) {
                    if (auto* dock = qobject_cast<QDockWidget*>(watched->property("editorDockTarget").value<QObject*>())) {
                        const bool nextFloating = !dock->isFloating();
                        dock->setFloating(nextFloating);
                        if (!nextFloating) {
                            restoreBottomPanelLayout();
                        }
                        dock->raise();
                        return true;
                    }
                }
            }
        }
    }

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

void EditorShell::installPanelActivationTracking(QWidget* panelRoot, ActivePanel panel, QDockWidget* dock)
{
    if (panelRoot == nullptr) {
        return;
    }

    const int panelValue = static_cast<int>(panel);
    panelRoot->setProperty("editorActivePanel", panelValue);
    if (dock != nullptr) {
        panelRoot->setProperty("editorDockTarget", QVariant::fromValue(static_cast<QObject*>(dock)));
    }
    panelRoot->installEventFilter(this);
    for (QWidget* child : panelRoot->findChildren<QWidget*>()) {
        child->setProperty("editorActivePanel", panelValue);
        if (dock != nullptr) {
            child->setProperty("editorDockTarget", QVariant::fromValue(static_cast<QObject*>(dock)));
        }
        child->installEventFilter(this);
    }
}

void EditorShell::setActivePanel(ActivePanel panel)
{
    if (activePanel_ == panel) {
        return;
    }

    activePanel_ = panel;
    refreshActivePanelVisuals();
}

void EditorShell::refreshActivePanelVisuals()
{
    const auto applyDockState = [this](QDockWidget* dock, ActivePanel panel) {
        if (dock == nullptr) {
            return;
        }

        dock->setProperty("activePanel", activePanel_ == panel ? "true" : "false");
        style()->unpolish(dock);
        style()->polish(dock);
        dock->update();
    };
    const auto applyPanelState = [this](QWidget* widget, ActivePanel panel) {
        if (widget == nullptr) {
            return;
        }

        widget->setProperty("activePanel", activePanel_ == panel ? "true" : "false");
        style()->unpolish(widget);
        style()->polish(widget);
        widget->update();
    };

    applyPanelState(viewport_, ActivePanel::Viewport);
    if (viewport_ != nullptr) {
        viewport_->setWorkspaceActive(activePanel_ == ActivePanel::Viewport);
    }
    applyDockState(outlinerDock_, ActivePanel::Outliner);
    applyDockState(inspectorDock_, ActivePanel::ChannelBox);
    applyDockState(timeSliderDock_, ActivePanel::TimeSlider);
    applyDockState(rangeSliderDock_, ActivePanel::RangeSlider);
    applyDockState(commandLineDock_, ActivePanel::CommandLine);
    applyDockState(graphEditorDock_, ActivePanel::GraphEditor);
    applyDockState(polygonPrimitivesDock_, ActivePanel::PrimitivePalette);
    applyDockState(scriptEditorDock_, ActivePanel::ScriptEditor);
    applyDockState(rigPanelDock_, ActivePanel::RigPanel);

    if (activePanelStatusLabel_ != nullptr) {
        activePanelStatusLabel_->setText(QString("Active Panel: %1").arg(activePanelDisplayName(activePanel_)));
    }
}

QString EditorShell::activePanelDisplayName(ActivePanel panel) const
{
    switch (panel) {
    case ActivePanel::Viewport:
        return "Viewport";
    case ActivePanel::Outliner:
        return "Outliner";
    case ActivePanel::ChannelBox:
        return "Channel Box";
    case ActivePanel::TimeSlider:
        return "Time Slider";
    case ActivePanel::RangeSlider:
        return "Range Slider";
    case ActivePanel::CommandLine:
        return "Command Line";
    case ActivePanel::GraphEditor:
        return "Graph Editor";
    case ActivePanel::PrimitivePalette:
        return "Polygon Primitives";
    case ActivePanel::ScriptEditor:
        return "Script Editor";
    case ActivePanel::RigPanel:
        return "Rig Panel";
    case ActivePanel::None:
    default:
        return "None";
    }
}

void EditorShell::createDocks()
{
    viewport_->setObjectName("viewportWidget");
    viewport_->setProperty("panelSurface", "true");
    setCentralWidget(viewport_);
    installPanelActivationTracking(viewport_, ActivePanel::Viewport);
    viewportDock_ = nullptr;

    // ── Register panels with WorkspaceManager ────────────────────────────────
    // WorkspaceManager creates the QDockWidget wrappers; EditorShell creates
    // the content widgets and wires activation tracking after registration.

    auto registerAndTrack = [this](
        const QString& id,
        const QString& displayName,
        QWidget* panel,
        ActivePanel activePanel,
        Qt::DockWidgetArea defaultArea,
        QDockWidget::DockWidgetFeatures features,
        Qt::DockWidgetAreas allowedAreas = Qt::AllDockWidgetAreas,
        bool compactTitleBar = false)
    {
        panel->setProperty("panelSurface", "true");
        workspaceManager_->registerPanel(id, displayName, panel, defaultArea, features, allowedAreas, compactTitleBar);
        QDockWidget* d = workspaceManager_->dock(id);
        installPanelActivationTracking(panel, activePanel, d);
        return d;
    };

    outlinerDock_ = registerAndTrack(
        WorkspaceManager::kOutliner, "Outliner", createOutlinerPanel(),
        ActivePanel::Outliner, Qt::LeftDockWidgetArea,
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    inspectorDock_ = registerAndTrack(
        WorkspaceManager::kChannelBox, "Channel Box", createInspectorPanel(),
        ActivePanel::ChannelBox, Qt::RightDockWidgetArea,
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    polygonPrimitivesDock_ = registerAndTrack(
        WorkspaceManager::kPrimitivePalette, "Polygon Primitives", createPrimitivePalettePanel(),
        ActivePanel::PrimitivePalette, Qt::RightDockWidgetArea,
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);

    scriptEditorDock_ = registerAndTrack(
        WorkspaceManager::kScriptEditor, "Script Editor", createScriptEditorPanel(),
        ActivePanel::ScriptEditor, Qt::BottomDockWidgetArea,
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);

    timeSliderDock_ = registerAndTrack(
        WorkspaceManager::kTimeline, "Time Slider", createTimeSliderPanel(),
        ActivePanel::TimeSlider, Qt::BottomDockWidgetArea,
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable,
        Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea,
        /*compactTitleBar=*/true);

    rangeSliderDock_ = registerAndTrack(
        WorkspaceManager::kRangeSlider, "Range Slider", createRangeSliderPanel(),
        ActivePanel::RangeSlider, Qt::BottomDockWidgetArea,
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable,
        Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea,
        /*compactTitleBar=*/true);

    commandLineDock_ = registerAndTrack(
        WorkspaceManager::kCommandLine, "Command Line", createCommandLinePanel(),
        ActivePanel::CommandLine, Qt::BottomDockWidgetArea,
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable,
        Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea,
        /*compactTitleBar=*/true);

    graphEditorDock_ = registerAndTrack(
        WorkspaceManager::kGraphEditor, "Graph Editor", createGraphEditorPanel(),
        ActivePanel::GraphEditor, Qt::BottomDockWidgetArea,
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);

    // Rig Panel — hidden in Default/Animation presets; raised in Rigging preset.
    rigPanelDock_ = registerAndTrack(
        WorkspaceManager::kRigPanel, "Rig Panel", createRigPanel(),
        ActivePanel::RigPanel, Qt::RightDockWidgetArea,
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);

    workspaceManager_->applyInitialLayout();
}

QWidget* EditorShell::createPrimitivePalettePanel()
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
    QObject::connect(polygonPrimitivesList_, &QListWidget::itemClicked, this, &EditorShell::handlePrimitivePaletteItemActivated);
    QObject::connect(polygonPrimitivesList_, &QListWidget::itemDoubleClicked, this, &EditorShell::handlePrimitivePaletteItemActivated);

    QPushButton* createPrimitiveButton = new QPushButton("Create Selected Primitive", primitivesPanel);
    createPrimitiveButton->setObjectName("createPrimitiveButton");
    QObject::connect(createPrimitiveButton, &QPushButton::clicked, this, &EditorShell::createPrimitiveFromPalette);

    QCheckBox* interactiveCreationCheckBox = new QCheckBox("Interactive Creation", primitivesPanel);
    interactiveCreationCheckBox->setObjectName("interactiveCreationCheckBox");
    interactiveCreationCheckBox->setChecked(interactivePrimitiveCreationEnabled_);
    QObject::connect(interactiveCreationCheckBox, &QCheckBox::toggled, this, &EditorShell::setInteractivePrimitiveCreationEnabled);

    QCheckBox* exitOnCompletionCheckBox = new QCheckBox("Exit On Completion", primitivesPanel);
    exitOnCompletionCheckBox->setObjectName("exitOnCompletionCheckBox");
    exitOnCompletionCheckBox->setChecked(exitPrimitiveToolOnCompletionEnabled_);
    QObject::connect(exitOnCompletionCheckBox, &QCheckBox::toggled, this, &EditorShell::setExitPrimitiveToolOnCompletionEnabled);

    primitivesLayout->addWidget(polygonPrimitivesList_);
    primitivesLayout->addWidget(createPrimitiveButton);
    primitivesLayout->addWidget(interactiveCreationCheckBox);
    primitivesLayout->addWidget(exitOnCompletionCheckBox);
    return primitivesPanel;
}

void EditorShell::newScene()
{
    viewport_->clearScene();
    playbackController_.setCurrentFrame(viewport_->currentFrame(), false);
    currentSceneFilePath_.clear();
    refreshScenePanels();
    viewport_->resetCamera();
    updateWindowTitle();
    applyFileFlowResult(EditorFileFlowController::buildNewSceneResult(), true, 2000);
}

void EditorShell::openScene()
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

QWidget* EditorShell::createOutlinerPanel()
{
    QWidget* panel = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(8, 8, 8, 8);

    outlinerTree_ = new QTreeWidget(panel);
    outlinerTree_->setObjectName("outlinerTree");
    outlinerTree_->setHeaderLabel("Scene");
    outlinerTree_->setSelectionMode(QAbstractItemView::SingleSelection);
    outlinerTree_->viewport()->installEventFilter(this);
    QObject::connect(outlinerTree_, &QTreeWidget::itemSelectionChanged, this, &EditorShell::handleOutlinerSelectionChanged);
    layout->addWidget(outlinerTree_);

    return panel;
}

QWidget* EditorShell::createInspectorPanel()
{
    QWidget* inspectorPanel = new QWidget(this);
    inspectorPanel->setObjectName("channelBoxPanel");
    QVBoxLayout* inspectorLayout = new QVBoxLayout(inspectorPanel);
    inspectorLayout->setContentsMargins(12, 12, 12, 12);
    inspectorLayout->setSpacing(8);

    inspectorEmptyStateLabel_ = new QLabel(inspectorPanel);
    inspectorEmptyStateLabel_->setObjectName("inspectorEmptyStateLabel");
    inspectorEmptyStateLabel_->setWordWrap(true);

    inspectorDetailsWidget_ = new QWidget(inspectorPanel);
    QVBoxLayout* inspectorDetailsLayout = new QVBoxLayout(inspectorDetailsWidget_);
    inspectorDetailsLayout->setContentsMargins(0, 0, 0, 0);
    inspectorDetailsLayout->setSpacing(8);

    channelObjectNameLabel_ = new QLabel(inspectorDetailsWidget_);
    channelObjectNameLabel_->setObjectName("channelObjectNameLabel");
    channelObjectNameLabel_->setWordWrap(true);
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
    visibilityCheckBox_->setProperty("variant", "channelVisibility");

    // Joint tools live in the separate Rig Panel dock (registered in createDocks).
    // The widgets (jointToolsWidget_, jointOrientX/Y/ZSpinBox_, etc.) are created
    // in createRigPanel() but stored as EditorShell members so EditorInspectorController
    // can still drive them via inspectorWidgets().

    QVBoxLayout* transformSectionLayout = nullptr;
    QWidget* transformSection = createChannelSection("Transform", inspectorDetailsWidget_, &transformSectionLayout);
    transformSectionLayout->addWidget(channelObjectNameLabel_);
    transformSectionLayout->addWidget(createChannelRow("Translate X", translateXSpinBox_, transformSection));
    transformSectionLayout->addWidget(createChannelRow("Translate Y", translateYSpinBox_, transformSection));
    transformSectionLayout->addWidget(createChannelRow("Translate Z", translateZSpinBox_, transformSection));
    transformSectionLayout->addWidget(createChannelRow("Rotate X", rotateXSpinBox_, transformSection));
    transformSectionLayout->addWidget(createChannelRow("Rotate Y", rotateYSpinBox_, transformSection));
    transformSectionLayout->addWidget(createChannelRow("Rotate Z", rotateZSpinBox_, transformSection));
    transformSectionLayout->addWidget(createChannelRow("Scale X", scaleXSpinBox_, transformSection));
    transformSectionLayout->addWidget(createChannelRow("Scale Y", scaleYSpinBox_, transformSection));
    transformSectionLayout->addWidget(createChannelRow("Scale Z", scaleZSpinBox_, transformSection));
    transformSectionLayout->addWidget(createChannelRow("Visibility", visibilityCheckBox_, transformSection));

    inspectorDetailsLayout->addWidget(transformSection);

    for (QDoubleSpinBox* spinBox : { translateXSpinBox_, translateYSpinBox_, translateZSpinBox_,
             rotateXSpinBox_, rotateYSpinBox_, rotateZSpinBox_,
             scaleXSpinBox_, scaleYSpinBox_, scaleZSpinBox_ }) {
        QObject::connect(spinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double) {
            applyChannelBoxToSelection();
        });
    }
    QObject::connect(visibilityCheckBox_, &QCheckBox::toggled, this, &EditorShell::applyVisibilityToSelection);

    frameSelectedButton_ = new QPushButton("Frame Selected", inspectorPanel);
    frameSelectedButton_->setObjectName("frameSelectedButton");
    frameSelectedButton_->setEnabled(false);
    QObject::connect(frameSelectedButton_, &QPushButton::clicked, this, &EditorShell::frameSelectedObject);

    inspectorLayout->addWidget(inspectorEmptyStateLabel_);
    inspectorLayout->addWidget(inspectorDetailsWidget_);
    inspectorLayout->addWidget(frameSelectedButton_);
    inspectorLayout->addStretch();

    return inspectorPanel;
}

QWidget* EditorShell::createRigPanel()
{
    QWidget* rigPanel = new QWidget(this);
    rigPanel->setObjectName("channelBoxPanel"); // reuse style — same dark bg

    QVBoxLayout* rigLayout = new QVBoxLayout(rigPanel);
    rigLayout->setContentsMargins(12, 12, 12, 12);
    rigLayout->setSpacing(8);

    // ── Joint tools ──────────────────────────────────────────────────────────
    jointToolsWidget_ = new QWidget(rigPanel);
    jointToolsWidget_->setObjectName("jointToolsWidget");
    QVBoxLayout* jointToolsLayout = new QVBoxLayout(jointToolsWidget_);
    jointToolsLayout->setContentsMargins(0, 0, 0, 0);
    jointToolsLayout->setSpacing(6);

    jointOrientXSpinBox_ = createChannelSpinBox(jointToolsWidget_);
    jointOrientXSpinBox_->setObjectName("jointOrientXSpinBox");
    jointOrientYSpinBox_ = createChannelSpinBox(jointToolsWidget_);
    jointOrientYSpinBox_->setObjectName("jointOrientYSpinBox");
    jointOrientZSpinBox_ = createChannelSpinBox(jointToolsWidget_);
    jointOrientZSpinBox_->setObjectName("jointOrientZSpinBox");
    jointToolsLayout->addWidget(createChannelRow("Joint X", jointOrientXSpinBox_, jointToolsWidget_));
    jointToolsLayout->addWidget(createChannelRow("Joint Y", jointOrientYSpinBox_, jointToolsWidget_));
    jointToolsLayout->addWidget(createChannelRow("Joint Z", jointOrientZSpinBox_, jointToolsWidget_));

    bindPoseStatusLabel_ = new QLabel("Bind pose: n/a", jointToolsWidget_);
    bindPoseStatusLabel_->setObjectName("bindPoseStatusLabel");
    bindPoseStatusLabel_->setProperty("variant", "channelMeta");
    bindPoseStatusLabel_->setWordWrap(true);
    jointToolsLayout->addWidget(bindPoseStatusLabel_);

    skinBindingStatusLabel_ = new QLabel("Skin binding: n/a", jointToolsWidget_);
    skinBindingStatusLabel_->setObjectName("skinBindingStatusLabel");
    skinBindingStatusLabel_->setProperty("variant", "channelMeta");
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

    for (QDoubleSpinBox* spinBox : { jointOrientXSpinBox_, jointOrientYSpinBox_, jointOrientZSpinBox_ }) {
        QObject::connect(spinBox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double) {
            applyJointOrientationToSelection();
        });
    }
    QObject::connect(resetJointOrientationButton_, &QPushButton::clicked,
                     this, &EditorShell::resetSelectedJointOrientation);
    QObject::connect(alignJointOrientationButton_, &QPushButton::clicked,
                     this, &EditorShell::alignSelectedJointOrientationToChild);
    QObject::connect(captureBindPoseButton_, &QPushButton::clicked,
                     this, &EditorShell::captureSelectedBindPose);
    QObject::connect(captureBindPoseRecursiveButton_, &QPushButton::clicked,
                     this, &EditorShell::captureSelectedBindPoseRecursive);

    QVBoxLayout* jointSectionLayout = nullptr;
    QWidget* jointSection = createChannelSection("Joint Tools", rigPanel, &jointSectionLayout);
    jointSectionLayout->addWidget(jointToolsWidget_);

    rigLayout->addWidget(jointSection);
    rigLayout->addStretch();

    return rigPanel;
}

QWidget* EditorShell::createTimeSliderPanel()
{
    QWidget* panel = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    animationTimelinePanel_ = new AnimationTimelinePanel(panel);
    animationTimelinePanel_->bindTimelineActions(
        editorMenuBar_->actions().duplicateKey,
        editorMenuBar_->actions().shiftKeysLeft,
        editorMenuBar_->actions().shiftKeysRight,
        editorMenuBar_->actions().previousKey,
        editorMenuBar_->actions().nextKey);
    animationTimelinePanel_->setPlaybackRangeChangedCallback([this](int startFrame, int endFrame) {
        if (!updatingTimeSlider_) {
            playbackController_.setPlaybackRange(startFrame, endFrame);
        }
    });
    animationTimelinePanel_->setSelectedFrameRangeChangedCallback([this](int startFrame, int endFrame) {
        if (!updatingTimeSlider_) {
            setSelectedFrameRange(startFrame, endFrame);
        }
    });
    animationTimelinePanel_->setCurrentFrameChangedCallback([this](int frame) {
        if (!updatingTimeSlider_) {
            playbackController_.setCurrentFrame(frame);
        }
    });
    animationTimelinePanel_->setJumpStartCallback([this]() { playbackController_.jumpToStart(); });
    animationTimelinePanel_->setStepBackCallback([this]() { playbackController_.stepFrame(-1); });
    animationTimelinePanel_->setTogglePlaybackCallback([this]() { playbackController_.togglePlayback(); });
    animationTimelinePanel_->setStepForwardCallback([this]() { playbackController_.stepFrame(1); });
    animationTimelinePanel_->setPreviousKeyCallback([this]() { jumpToSelectedObjectKeyframe(false, true); });
    animationTimelinePanel_->setNextKeyCallback([this]() { jumpToSelectedObjectKeyframe(true, true); });
    animationTimelinePanel_->setJumpEndCallback([this]() { playbackController_.jumpToEnd(); });
    animationTimelinePanel_->setSetKeyCallback([this]() { setKeyForSelection(true); });
    animationTimelinePanel_->setDeleteKeyCallback([this]() { deleteKeyForSelection(true); });
    animationTimelinePanel_->setDuplicateKeyCallback([this]() { duplicateCurrentKeyForSelection(true); });
    animationTimelinePanel_->setShiftKeysLeftCallback([this]() { shiftSelectedObjectKeyframes(-1, true); });
    animationTimelinePanel_->setShiftKeysRightCallback([this]() { shiftSelectedObjectKeyframes(1, true); });
    animationTimelinePanel_->setAutoKeyChangedCallback([this](bool enabled) { setAutoKeyEnabled(enabled, true); });
    animationTimelinePanel_->setViewModel(buildAnimationTimelineViewModel());
    layout->addWidget(animationTimelinePanel_);

    return panel;
}

QWidget* EditorShell::createRangeSliderPanel()
{
    QWidget* panel = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    rangeSliderPanel_ = new RangeSliderPanel(panel);
    rangeSliderPanel_->setAutoKeyChangedCallback([this](bool enabled) {
        setAutoKeyEnabled(enabled, true);
    });
    rangeSliderPanel_->setFramesPerSecondChangedCallback([this](int framesPerSecond) {
        applyAnimationFlowResult(
            EditorAnimationFlowController::setFramesPerSecond(
                animationFlowContext(),
                animationState_,
                framesPerSecond,
                true));
    });
    rangeSliderPanel_->setVisibleRangeChangedCallback([this](int startFrame, int endFrame) {
        if (!updatingTimeSlider_) {
            playbackController_.setVisibleFrameRange(startFrame, endFrame, false);
        }
    });
    rangeSliderPanel_->setPlaybackRangeChangedCallback([this](int startFrame, int endFrame) {
        if (!updatingTimeSlider_) {
            playbackController_.setPlaybackRange(startFrame, endFrame);
        }
    });
    rangeSliderPanel_->setViewModel(buildAnimationTimelineViewModel());
    layout->addWidget(rangeSliderPanel_);

    return panel;
}

QWidget* EditorShell::createCommandLinePanel()
{
    QWidget* panel = new QWidget(this);
    panel->setObjectName("commandLinePanel");
    panel->setMinimumHeight(28);

    QHBoxLayout* layout = new QHBoxLayout(panel);
    layout->setContentsMargins(10, 4, 10, 4);
    layout->setSpacing(10);

    QLabel* prefixLabel = new QLabel("Command Line", panel);
    prefixLabel->setObjectName("commandLinePrefixLabel");
    layout->addWidget(prefixLabel);

    commandLineStatusLabel_ = new QLabel("Ready", panel);
    commandLineStatusLabel_->setObjectName("commandLineStatusLabel");
    commandLineStatusLabel_->setProperty("statusTone", "muted");
    commandLineStatusLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(commandLineStatusLabel_, 1);

    commandLineBaseText_ = "Ready";
    commandLineBaseTone_ = "muted";

    connect(statusBar(), &QStatusBar::messageChanged, this, [this](const QString&) {
        updateCommandLineDisplay();
    });

    return panel;
}

QWidget* EditorShell::createGraphEditorPanel()
{
    graphEditorPanel_ = new GraphEditorPanel(this);
    graphEditorPanel_->setObjectName("graphEditorPanel");
    graphEditorPanel_->setViewModel(buildGraphEditorViewModel());
    graphEditorPanel_->setCurrentFrameChangedCallback([this](int frame) {
        if (!updatingTimeSlider_) {
            playbackController_.setCurrentFrame(frame);
        }
    });
    graphEditorPanel_->setKeyEditedCallback([this](const GraphEditorKeyEdit& edit) {
        handleGraphEditorKeyEdited(edit);
    });
    graphEditorPanel_->setTangentEditedCallback([this](const TangentEdit& edit) {
        handleGraphEditorTangentEdited(edit);
    });
    graphEditorPanel_->setTangentModeChangedCallback([this](const TangentModeChange& change) {
        handleGraphEditorTangentModeChanged(change);
    });
    return graphEditorPanel_;
}

QWidget* EditorShell::createScriptEditorPanel()
{
    QWidget* panel = new QWidget(this);
    QVBoxLayout* rootLayout = new QVBoxLayout(panel);
    rootLayout->setContentsMargins(6, 6, 6, 6);
    rootLayout->setSpacing(6);

    QMenuBar* menuBar = new QMenuBar(panel);
    QMenu* fileMenu = menuBar->addMenu("File");
    fileMenu->addAction("Clear History", this, &EditorShell::clearScriptHistory);
    QMenu* editMenu = menuBar->addMenu("Edit");
    editMenu->addAction("Execute All", this, &EditorShell::executeScriptEditorAll);
    editMenu->addAction("Execute Selection", this, &EditorShell::executeScriptEditorSelection);
    menuBar->addMenu("History");
    menuBar->addMenu("Command");
    menuBar->addMenu("Tabs");
    menuBar->addMenu("Help");

    QToolBar* toolBar = new QToolBar(panel);
    toolBar->setMovable(false);
    QAction* executeAllAction = toolBar->addAction("Execute All");
    executeAllAction->setObjectName("scriptExecuteAllAction");
    QObject::connect(executeAllAction, &QAction::triggered, this, &EditorShell::executeScriptEditorAll);
    QAction* executeSelectionAction = toolBar->addAction("Execute Selection");
    executeSelectionAction->setObjectName("scriptExecuteSelectionAction");
    QObject::connect(executeSelectionAction, &QAction::triggered, this, &EditorShell::executeScriptEditorSelection);
    QAction* clearHistoryAction = toolBar->addAction("Clear History");
    clearHistoryAction->setObjectName("scriptClearHistoryAction");
    QObject::connect(clearHistoryAction, &QAction::triggered, this, &EditorShell::clearScriptHistory);

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

void EditorShell::importFbx()
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

bool EditorShell::saveScene()
{
    if (currentSceneFilePath_.isEmpty()) {
        return saveSceneAs();
    }

    return saveSceneToPath(currentSceneFilePath_, true);
}

bool EditorShell::saveSceneAs()
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

bool EditorShell::openSceneFromPath(const QString& filePath, bool logToScript)
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

bool EditorShell::importFbxFromPath(const QString& filePath, bool logToScript)
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

    playbackController_.setCurrentFrame(animationState_.currentFrame, false);
    refreshScenePanels();
    applyFileFlowResult(EditorFileFlowController::buildImportSceneResult(filePath), logToScript, 4000);
    return true;
}

bool EditorShell::saveSceneToPath(const QString& filePath, bool logToScript)
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

bool EditorShell::incrementAndSave()
{
    currentSceneFilePath_ = EditorFileFlowController::buildIncrementSavePath(currentSceneFilePath_, QDir::currentPath());
    return saveScene();
}

bool EditorShell::archiveScene()
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

bool EditorShell::exportAll()
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

bool EditorShell::exportSelection()
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

void EditorShell::optimizeSceneStorage()
{
    viewport_->optimizeSceneStorage();
    refreshScenePanels();
    EditorScriptLogController::appendComment(scriptHistoryTextEdit_, "Scene storage optimized");
    showStatusMessageIfPresent("Scene storage optimized", 2000);
}

void EditorShell::savePreferences()
{
    EditorPreferences::write(EditorPreferencesState {
        saveGeometry(),
        saveState(),
        animationState_.autoKeyEnabled,
    });
    if (workspaceManager_ != nullptr) {
        workspaceManager_->saveSessionLayout();
    }
    EditorScriptLogController::appendComment(scriptHistoryTextEdit_, "Saved Phoenix Editor preferences");
    showStatusMessageIfPresent("Preferences saved", 2000);
}

void EditorShell::loadPreferences()
{
    const EditorPreferencesState preferences = EditorPreferences::read();
    if (!preferences.geometry.isEmpty()) {
        restoreGeometry(preferences.geometry);
    }

    if (!preferences.windowState.isEmpty()) {
        restoreState(preferences.windowState);
    }

    if (workspaceManager_ != nullptr) {
        workspaceManager_->restoreSessionLayout();
    }

    animationState_ = EditorAnimationController::setAutoKeyEnabled(
        animationState_,
        preferences.autoKeyEnabled);
    if (viewport_ != nullptr) {
        viewport_->setAutoKeyEnabled(animationState_.autoKeyEnabled);
    }
}

void EditorShell::handlePrimitivePaletteItemActivated(QListWidgetItem* item)
{
    if (item == nullptr) {
        return;
    }

    createPrimitiveFromPalette();
}

void EditorShell::setInteractivePrimitiveCreationEnabled(bool enabled)
{
    interactivePrimitiveCreationEnabled_ = enabled;
}

void EditorShell::setExitPrimitiveToolOnCompletionEnabled(bool enabled)
{
    exitPrimitiveToolOnCompletionEnabled_ = enabled;
}

void EditorShell::createPrimitiveFromPalette()
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

void EditorShell::createJoint()
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

void EditorShell::markSelectionAsHierarchyParent()
{
    applyRiggingOperationResult(EditorRiggingController::markHierarchyParent(riggingContext(), selectedObjectId_));
}

void EditorShell::parentSelectionToMarkedParent()
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

void EditorShell::unparentSelection()
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

void EditorShell::bindSelectedMeshToMarkedJoint()
{
    applyRiggingOperationResult(
        EditorRiggingController::bindSelectedMeshToMarkedJoint(riggingContext(), selectedObjectId_, markedHierarchyParentId_));
}

void EditorShell::resetSelectedJointOrientation()
{
    applyRiggingOperationResult(EditorRiggingController::resetSelectedJointOrientation(riggingContext(), selectedObjectId_));
}

void EditorShell::alignSelectedJointOrientationToChild()
{
    applyRiggingOperationResult(EditorRiggingController::alignSelectedJointOrientationToChild(riggingContext(), selectedObjectId_));
}

void EditorShell::captureSelectedBindPose()
{
    applyRiggingOperationResult(EditorRiggingController::captureSelectedBindPose(riggingContext(), selectedObjectId_, false));
}

void EditorShell::captureSelectedBindPoseRecursive()
{
    applyRiggingOperationResult(EditorRiggingController::captureSelectedBindPose(riggingContext(), selectedObjectId_, true));
}

bool EditorShell::reparentObjectInUi(std::uint64_t childId, std::uint64_t newParentId, bool logToScript)
{
    const EditorRiggingController::OperationResult result =
        EditorRiggingController::reparentObject(riggingContext(), childId, newParentId);
    applyRiggingOperationResult(result, logToScript);
    return result.success;
}

void EditorShell::showScriptEditorWindow()
{
    if (scriptEditorDock_ == nullptr) {
        return;
    }

    scriptEditorDock_->show();
    scriptEditorDock_->raise();
    EditorScriptLogController::appendComment(scriptHistoryTextEdit_, "Script Editor opened");
}

void EditorShell::showGraphEditorWindow()
{
    if (graphEditorDock_ == nullptr) {
        return;
    }

    graphEditorDock_->show();
    graphEditorDock_->raise();
}

void EditorShell::executeScriptEditorAll()
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

void EditorShell::executeScriptEditorSelection()
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

void EditorShell::clearScriptHistory()
{
    EditorScriptLogController::clearHistory(scriptHistoryTextEdit_);
}

bool EditorShell::executeScriptCommand(QString commandLine, QString* resultLine)
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

ScriptCommandContext EditorShell::createScriptCommandContext()
{
    ScriptCommandContext context;
    EditorDocumentController::ScriptBindings documentBindings;
    EditorViewportCommandController::ScriptBindings viewportBindings;
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
        const auto& a = editorMenuBar_->actions();
        a.translate->setChecked(translateChecked);
        a.rotate->setChecked(rotateChecked);
        a.scale->setChecked(scaleChecked);
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
    EditorAnimationEngineFacade::bindScriptCommands(context, createAnimationScriptBindings());
    return context;
}

EditorAnimationEngineFacade::ScriptBindings EditorShell::createAnimationScriptBindings()
{
    EditorAnimationEngineFacade::ScriptBindings bindings;
    bindings.context = animationEngineContext();
    bindings.findObjectIdByName = [this](const QString& objectName) {
        return findObjectIdByName(objectName);
    };
    bindings.selectObjectById = [this](std::uint64_t objectId) {
        selectObject(objectId, true);
    };
    return bindings;
}

EditorCreationController::ScriptBindings EditorShell::createCreationScriptBindings()
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

EditorSceneMutationController::ScriptBindings EditorShell::createSceneScriptBindings()
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
        EditorSceneRuntimeController::ApplySceneRequest request;
        request.scene = scene;
        request.recordUndo = true;
        request.selection.objectId = objectId;
        request.selection.clearSelection = clearSelectionAfter;
        EditorSceneRuntimeController::applyScene(sceneRuntimeContext(), request);
    };
    bindings.applyLiveMutation = [this](std::uint64_t objectId, bool clearSelectionAfter) {
        refreshScenePanels();
        EditorSceneRuntimeController::SelectionRefreshRequest request;
        request.objectId = objectId;
        request.clearSelection = clearSelectionAfter;
        EditorSceneRuntimeController::refreshSelection(sceneRuntimeContext(), request);
    };
    return bindings;
}

std::uint64_t EditorShell::findObjectIdByName(const QString& objectName) const
{
    return EditorSceneQueryController::findObjectIdByName(*viewport_, objectName);
}

QString EditorShell::generateUniqueScriptName(const QString& prefix) const
{
    return EditorSceneQueryController::generateUniqueScriptName(*viewport_, prefix);
}

QString EditorShell::generateUniqueObjectName(const QString& baseName, std::uint64_t ignoreObjectId) const
{
    return EditorSceneQueryController::generateUniqueObjectName(*viewport_, baseName, ignoreObjectId);
}

void EditorShell::refreshScenePanels()
{
    if (markedHierarchyParentId_ != 0 && !viewport_->containsObject(markedHierarchyParentId_)) {
        markedHierarchyParentId_ = 0;
    }
    populateOutliner();
    clearInspector();
}

void EditorShell::restoreHistoryState(const EditorHistoryState& state)
{
    restoringHistory_ = true;
    markedHierarchyParentId_ = state.markedHierarchyParentId;
    EditorSceneRuntimeController::ApplySceneRequest request;
    request.scene = state.scene;
    request.applyCurrentFrame = true;
    request.currentFrame = state.currentFrame;
    request.selection.objectId = state.selectedObjectId;
    EditorSceneRuntimeController::applyScene(sceneRuntimeContext(), request);
    restoringHistory_ = false;
    updateUndoRedoActions();
}

void EditorShell::recordUndoState()
{
    historyController_.recordUndoState(
        historyController_.captureState(viewport_->scene(), selectedObjectId_, markedHierarchyParentId_, animationState_.currentFrame));
    updateUndoRedoActions();
}

void EditorShell::undoLastChange()
{
    EditorHistoryState previousState;
    if (!historyController_.tryTakeUndoState(
            historyController_.captureState(viewport_->scene(), selectedObjectId_, markedHierarchyParentId_, animationState_.currentFrame),
            &previousState)) {
        return;
    }

    restoreHistoryState(previousState);
}

void EditorShell::redoLastChange()
{
    EditorHistoryState nextState;
    if (!historyController_.tryTakeRedoState(
            historyController_.captureState(viewport_->scene(), selectedObjectId_, markedHierarchyParentId_, animationState_.currentFrame),
            &nextState)) {
        return;
    }

    restoreHistoryState(nextState);
}

void EditorShell::updateUndoRedoActions()
{
    if (editorMenuBar_ == nullptr) {
        return;
    }
    const auto& a = editorMenuBar_->actions();
    if (a.undo != nullptr) {
        a.undo->setEnabled(historyController_.canUndo());
    }
    if (a.redo != nullptr) {
        a.redo->setEnabled(historyController_.canRedo());
    }
}

// ---------------------------------------------------------------------------
// Graph Editor handlers (Phase 2 / 3 / 4)
// ---------------------------------------------------------------------------

namespace
{
// Find index in track where kf.frame == frame; returns -1 if not found.
int findKeyframeIndex(const TransformKeyframeTrack& track, int frame)
{
    for (int i = 0; i < track.size(); ++i) {
        if (track[i].frame == frame) {
            return i;
        }
    }
    return -1;
}

// Patch one channel of a Transform in-place by curveId / newValue.
void patchTransformChannel(Transform& t, const QString& curveId, double newValue)
{
    if (curveId == "tx") {
        t.translation.setX(static_cast<float>(newValue));
    } else if (curveId == "ty") {
        t.translation.setY(static_cast<float>(newValue));
    } else if (curveId == "tz") {
        t.translation.setZ(static_cast<float>(newValue));
    } else if (curveId == "rx") {
        QVector3D e = t.rotation.toEulerAngles();
        e.setX(static_cast<float>(newValue));
        t.rotation = QQuaternion::fromEulerAngles(e);
    } else if (curveId == "ry") {
        QVector3D e = t.rotation.toEulerAngles();
        e.setY(static_cast<float>(newValue));
        t.rotation = QQuaternion::fromEulerAngles(e);
    } else if (curveId == "rz") {
        QVector3D e = t.rotation.toEulerAngles();
        e.setZ(static_cast<float>(newValue));
        t.rotation = QQuaternion::fromEulerAngles(e);
    } else if (curveId == "sx") {
        t.scale.setX(static_cast<float>(newValue));
    } else if (curveId == "sy") {
        t.scale.setY(static_cast<float>(newValue));
    } else if (curveId == "sz") {
        t.scale.setZ(static_cast<float>(newValue));
    }
}
} // namespace

void EditorShell::handleGraphEditorKeyEdited(const GraphEditorKeyEdit& edit)
{
    if (selectedObjectId_ == 0) {
        return;
    }

    Scene scene = viewport_->sceneSnapshot();
    SceneObject* object = scene.findObject(selectedObjectId_);
    if (object == nullptr) {
        return;
    }

    TransformKeyframeTrack track = object->transformKeyframes();
    const int idx = findKeyframeIndex(track, edit.oldFrame);
    if (idx < 0) {
        return;
    }

    // Patch channel value and move to new frame
    patchTransformChannel(track[idx].transform, edit.curveId, edit.newValue);
    track[idx].frame = edit.newFrame;

    // Re-sort if frame changed
    if (edit.oldFrame != edit.newFrame) {
        std::sort(track.begin(), track.end(), [](const TransformKeyframe& a, const TransformKeyframe& b) {
            return a.frame < b.frame;
        });
    }

    object->setTransformKeyframes(track);

    EditorSceneRuntimeController::ApplySceneRequest request;
    request.scene = scene;
    request.recordUndo = true;
    request.applyCurrentFrame = true;
    request.currentFrame = edit.newFrame;
    request.selection.objectId = selectedObjectId_;
    EditorSceneRuntimeController::applyScene(sceneRuntimeContext(), request);
}

void EditorShell::handleGraphEditorTangentEdited(const TangentEdit& edit)
{
    if (selectedObjectId_ == 0) {
        return;
    }

    Scene scene = viewport_->sceneSnapshot();
    SceneObject* object = scene.findObject(selectedObjectId_);
    if (object == nullptr) {
        return;
    }

    TransformKeyframeTrack track = object->transformKeyframes();
    const int idx = findKeyframeIndex(track, edit.frame);
    if (idx < 0) {
        return;
    }

    if (edit.isInHandle) {
        track[idx].tangent.inAngle = edit.newAngle;
        if (track[idx].tangent.mode == TangentMode::Auto) {
            track[idx].tangent.mode = TangentMode::Broken;
        }
    } else {
        track[idx].tangent.outAngle = edit.newAngle;
        if (track[idx].tangent.mode == TangentMode::Auto) {
            track[idx].tangent.mode = TangentMode::Broken;
        }
    }

    object->setTransformKeyframes(track);

    EditorSceneRuntimeController::ApplySceneRequest request;
    request.scene = scene;
    request.recordUndo = true;
    request.selection.objectId = selectedObjectId_;
    EditorSceneRuntimeController::applyScene(sceneRuntimeContext(), request);
}

void EditorShell::handleGraphEditorTangentModeChanged(const TangentModeChange& change)
{
    if (selectedObjectId_ == 0) {
        return;
    }

    Scene scene = viewport_->sceneSnapshot();
    SceneObject* object = scene.findObject(selectedObjectId_);
    if (object == nullptr) {
        return;
    }

    TransformKeyframeTrack track = object->transformKeyframes();
    const int idx = findKeyframeIndex(track, change.frame);
    if (idx < 0) {
        return;
    }

    track[idx].tangent.mode = change.newMode;
    if (change.newMode == TangentMode::Flat) {
        track[idx].tangent.inAngle = 0.f;
        track[idx].tangent.outAngle = 0.f;
    }

    object->setTransformKeyframes(track);

    EditorSceneRuntimeController::ApplySceneRequest request;
    request.scene = scene;
    request.recordUndo = true;
    request.selection.objectId = selectedObjectId_;
    EditorSceneRuntimeController::applyScene(sceneRuntimeContext(), request);
}

// ---------------------------------------------------------------------------

void EditorShell::populateOutliner()
{
    EditorOutlinerController::populateTree(outlinerTree_, outlinerSceneAccess());
}

void EditorShell::clearInspector()
{
    updatingChannelBox_ = true;
    EditorSelectionController::Context context = selectionContext();
    EditorSelectionController::clearSelection(context, selectedObjectId_, markedHierarchyParentId_);
    updatingChannelBox_ = false;
    refreshAnimationTimelineUi();
}

void EditorShell::updateInspector(std::uint64_t objectId)
{
    updatingChannelBox_ = true;
    EditorSelectionController::Context context = selectionContext();
    EditorSelectionController::selectObject(context, objectId, false, selectedObjectId_, markedHierarchyParentId_);
    updatingChannelBox_ = false;
    refreshAnimationTimelineUi();
}

void EditorShell::handleOutlinerSelectionChanged()
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

void EditorShell::frameSelectedObject()
{
    if (EditorSelectionController::frameSelectedObject(selectionContext(), selectedObjectId_)) {
        statusBar()->showMessage("Selected object framed", 2000);
    }
}

void EditorShell::resetSceneCamera()
{
    viewport_->resetCamera();
    EditorScriptLogController::appendHistoryLine(scriptHistoryTextEdit_, "viewSet -home;");
    EditorScriptLogController::appendHistoryLine(scriptHistoryTextEdit_, "// Result: camera reset //");
    showStatusMessageIfPresent("Camera reset", 2000);
}

void EditorShell::frameEntireScene()
{
    viewport_->frameScene();
    EditorScriptLogController::appendHistoryLine(scriptHistoryTextEdit_, "viewFit;");
    EditorScriptLogController::appendHistoryLine(scriptHistoryTextEdit_, "// Result: scene framed //");
    showStatusMessageIfPresent("Scene framed", 2000);
}

void EditorShell::setWireframeDisplayEnabled(bool enabled)
{
    viewport_->setWireframeEnabled(enabled);
    EditorScriptLogController::appendComment(scriptHistoryTextEdit_, QString("Wireframe %1").arg(enabled ? "on" : "off"));
    showStatusMessageIfPresent(enabled ? "Wireframe on" : "Wireframe off", 2000);
}

void EditorShell::setAxisVisibilityEnabled(bool enabled)
{
    viewport_->setAxisVisible(enabled);
    EditorScriptLogController::appendComment(scriptHistoryTextEdit_, QString("Axis visibility %1").arg(enabled ? "on" : "off"));
    showStatusMessageIfPresent(enabled ? "Axis visible" : "Axis hidden", 2000);
}

void EditorShell::setBackfaceCullingEnabled(bool enabled)
{
    viewport_->setBackfaceCullingEnabled(enabled);
    EditorScriptLogController::appendComment(scriptHistoryTextEdit_, QString("Backface culling %1").arg(enabled ? "on" : "off"));
    showStatusMessageIfPresent(enabled ? "Backface culling on" : "Backface culling off", 2000);
}

void EditorShell::selectObject(std::uint64_t objectId, bool syncOutliner)
{
    updatingChannelBox_ = true;
    EditorSelectionController::Context context = selectionContext();
    EditorSelectionController::selectObject(context, objectId, syncOutliner, selectedObjectId_, markedHierarchyParentId_);
    updatingChannelBox_ = false;
    refreshAnimationTimelineUi();
}

EditorAnimationTimelineViewModel EditorShell::buildAnimationTimelineViewModel() const
{
    return EditorAnimationTimelineViewBuilder::build(viewport_->scene(), selectedObjectId_, animationState_);
}

GraphEditorViewModel EditorShell::buildGraphEditorViewModel() const
{
    GraphEditorViewModel viewModel;
    viewModel.visibleStartFrame = animationState_.playbackStartFrame;
    viewModel.visibleEndFrame = animationState_.playbackEndFrame;
    viewModel.currentFrame = animationState_.currentFrame;

    const SceneObject* object = viewport_ != nullptr ? viewport_->findObject(selectedObjectId_) : nullptr;
    if (object == nullptr) {
        return viewModel;
    }

    viewModel.objectName = QString("Graph Editor  |  %1").arg(object->name());
    if (!object->hasAnimation()) {
        viewModel.summaryText = "Selected object has no animation curves yet.";
        return viewModel;
    }

    viewModel.summaryText = QString("%1 keys across transform channels. Click canvas to scrub frame.")
        .arg(object->transformKeyframes().size());

    const auto buildPoints = [object](auto valueFn) {
        QVector<GraphEditorCurvePoint> points;
        const TransformKeyframeTrack& track = object->transformKeyframes();
        points.reserve(track.size());
        for (const TransformKeyframe& keyframe : track) {
            GraphEditorCurvePoint pt;
            pt.frame = keyframe.frame;
            pt.value = valueFn(keyframe);
            pt.inAngle = keyframe.tangent.inAngle;
            pt.outAngle = keyframe.tangent.outAngle;
            pt.tangentMode = keyframe.tangent.mode;
            points.append(pt);
        }
        return points;
    };

    const auto appendCurve =
        [&viewModel, &buildPoints](const QString& id, const QString& label, const QColor& color, auto valueFn) {
            GraphEditorCurve curve;
            curve.id = id;
            curve.label = label;
            curve.color = color;
            curve.points = buildPoints(valueFn);
            viewModel.curves.append(curve);
        };

    appendCurve("tx", "Translate X", QColor("#f67272"), [](const TransformKeyframe& key) { return key.transform.translation.x(); });
    appendCurve("ty", "Translate Y", QColor("#75db8f"), [](const TransformKeyframe& key) { return key.transform.translation.y(); });
    appendCurve("tz", "Translate Z", QColor("#64a8ff"), [](const TransformKeyframe& key) { return key.transform.translation.z(); });
    appendCurve("rx", "Rotate X", QColor("#ffae57"), [](const TransformKeyframe& key) { return key.transform.rotation.toEulerAngles().x(); });
    appendCurve("ry", "Rotate Y", QColor("#d7c16b"), [](const TransformKeyframe& key) { return key.transform.rotation.toEulerAngles().y(); });
    appendCurve("rz", "Rotate Z", QColor("#d889ff"), [](const TransformKeyframe& key) { return key.transform.rotation.toEulerAngles().z(); });
    appendCurve("sx", "Scale X", QColor("#78d4cf"), [](const TransformKeyframe& key) { return key.transform.scale.x(); });
    appendCurve("sy", "Scale Y", QColor("#9fe870"), [](const TransformKeyframe& key) { return key.transform.scale.y(); });
    appendCurve("sz", "Scale Z", QColor("#6ed6ff"), [](const TransformKeyframe& key) { return key.transform.scale.z(); });

    return viewModel;
}

void EditorShell::setTransformUiMode(TransformUiMode mode)
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

void EditorShell::setViewCameraPreset(ViewCameraUiPreset preset)
{
    if (editorMenuBar_ == nullptr) {
        return;
    }
    const auto& a = editorMenuBar_->actions();
    if (a.perspectiveCamera == nullptr
            || a.frontCamera == nullptr
            || a.backCamera == nullptr
            || a.leftCamera == nullptr
            || a.rightCamera == nullptr
            || a.topCamera == nullptr
            || a.bottomCamera == nullptr) {
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

void EditorShell::setAxisUiOrientation(AxisUiOrientation orientation)
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

void EditorShell::restoreDefaultWorkspaceLayout()
{
    if (workspaceManager_ == nullptr) {
        return;
    }
    workspaceManager_->applyDefaultPreset();
    applyViewportUiOperationResult(EditorViewportUiController::buildWorkspaceLayoutResult(), 2000);
}

void EditorShell::restoreBottomPanelLayout()
{
    if (workspaceManager_ != nullptr) {
        workspaceManager_->restoreBottomPanelLayout();
    }
}

void EditorShell::showPolygonPrimitivesWindow()
{
    if (polygonPrimitivesDock_ == nullptr) {
        return;
    }

    polygonPrimitivesDock_->show();
    polygonPrimitivesDock_->raise();
    applyViewportUiOperationResult(EditorViewportUiController::buildPolygonPrimitivesWindowResult(), 0);
}

void EditorShell::updateWindowTitle()
{
    const QString sceneName = currentSceneFilePath_.isEmpty()
        ? "untitled"
        : QFileInfo(currentSceneFilePath_).fileName();
    setWindowTitle(QString("%1 - Phoenix Editor Beta").arg(sceneName));
}

void EditorShell::applyDocumentSceneLoad(const Scene& scene, const QString& filePath, bool frameScene)
{
    EditorSceneRuntimeController::ApplySceneRequest request;
    request.scene = scene;
    request.recordUndo = true;
    request.applyCurrentFrame = true;
    request.currentFrame = scene.currentFrame();
    request.frameEntireScene = frameScene;
    EditorSceneRuntimeController::applyScene(sceneRuntimeContext(), request);
    currentSceneFilePath_ = filePath;
    updateWindowTitle();
}

bool EditorShell::showDocumentOperationFailure(
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

void EditorShell::applyAnimationState(const EditorAnimationState& state, bool logToScript)
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

    playbackController_.sync(animationState_);
    refreshAnimationTimelineUi();

    if (selectedObjectId_ != 0 && viewport_ != nullptr && viewport_->containsObject(selectedObjectId_)) {
        updateChannelBox(selectedObjectId_);
        const SceneObject* object = viewport_->findObject(selectedObjectId_);
        const bool canFrame = object != nullptr && object->isVisible() && object->worldBounds().isValid();
        frameSelectedButton_->setEnabled(canFrame);
        editorMenuBar_->actions().frameSelected->setEnabled(canFrame);
    }

    if (logToScript) {
        EditorScriptLogController::appendHistoryLine(scriptHistoryTextEdit_, QString("currentTime %1;").arg(animationState_.currentFrame));
        EditorScriptLogController::appendHistoryLine(scriptHistoryTextEdit_, QString("// Result: current frame %1 //").arg(animationState_.currentFrame));
    }
}

void EditorShell::setSelectedFrameRange(int startFrame, int endFrame)
{
    applyAnimationState(
        EditorAnimationController::setSelectedFrameRange(animationState_, startFrame, endFrame),
        false);
}

void EditorShell::setKeyForSelection(bool logToScript)
{
    applyAnimationEngineResult(
        EditorAnimationEngineFacade::setKeyForSelection(animationEngineContext(), selectedObjectId_, logToScript));
}

void EditorShell::deleteKeyForSelection(bool logToScript)
{
    applyAnimationEngineResult(
        EditorAnimationEngineFacade::deleteKeyForSelection(animationEngineContext(), selectedObjectId_, logToScript));
}

void EditorShell::duplicateCurrentKeyForSelection(bool logToScript)
{
    applyAnimationEngineResult(
        EditorAnimationEngineFacade::duplicateCurrentKeyForSelection(animationEngineContext(), selectedObjectId_, logToScript));
}

void EditorShell::shiftSelectedObjectKeyframes(int frameDelta, bool logToScript)
{
    applyAnimationEngineResult(
        EditorAnimationEngineFacade::shiftSelectedObjectKeyframes(
            animationEngineContext(),
            selectedObjectId_,
            frameDelta,
            logToScript));
}

void EditorShell::setAutoKeyEnabled(bool enabled, bool logToScript)
{
    applyAnimationEngineResult(
        EditorAnimationEngineFacade::setAutoKeyEnabled(animationEngineContext(), enabled, logToScript));
}

void EditorShell::jumpToSelectedObjectKeyframe(bool forward, bool logToScript)
{
    applyAnimationEngineResult(
        EditorAnimationEngineFacade::jumpToSelectedObjectKeyframe(
            animationEngineContext(),
            selectedObjectId_,
            forward,
            logToScript));
}

PrimitiveMeshFactory::Type EditorShell::primitiveTypeFromItem(const QListWidgetItem* item) const
{
    if (item == nullptr) {
        return PrimitiveMeshFactory::Type::Cube;
    }

    return static_cast<PrimitiveMeshFactory::Type>(item->data(kPrimitiveTypeRole).toInt());
}

void EditorShell::updateChannelBox(std::uint64_t objectId)
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

void EditorShell::refreshAnimationTimelineUi()
{
    const EditorAnimationTimelineViewModel viewModel = buildAnimationTimelineViewModel();
    if (animationTimelinePanel_ != nullptr) {
        animationTimelinePanel_->setViewModel(viewModel);
        if (rangeSliderPanel_ != nullptr) {
            rangeSliderPanel_->setViewModel(viewModel);
        }
    }
    if (viewModel.statusStyle.contains("#d6b06e")) {
        setCommandLineBaseStatus(viewModel.statusText, "warning");
    } else if (viewModel.statusStyle.contains("#8eb3cf")) {
        setCommandLineBaseStatus(viewModel.statusText, "info");
    } else {
        setCommandLineBaseStatus(viewModel.statusText, "muted");
    }
    if (graphEditorPanel_ != nullptr) {
        graphEditorPanel_->setViewModel(buildGraphEditorViewModel());
    }
}

void EditorShell::setCommandLineBaseStatus(const QString& text, const QString& tone)
{
    commandLineBaseText_ = text;
    commandLineBaseTone_ = tone;
    updateCommandLineDisplay();
}

void EditorShell::updateCommandLineDisplay()
{
    if (commandLineStatusLabel_ == nullptr || statusBar() == nullptr) {
        return;
    }

    const QString transientMessage = statusBar()->currentMessage();
    if (!transientMessage.isEmpty()) {
        commandLineStatusLabel_->setText(transientMessage);
        commandLineStatusLabel_->setProperty("statusTone", "info");
    } else {
        commandLineStatusLabel_->setText(commandLineBaseText_.isEmpty() ? "Ready" : commandLineBaseText_);
        commandLineStatusLabel_->setProperty("statusTone", commandLineBaseTone_);
    }

    commandLineStatusLabel_->style()->unpolish(commandLineStatusLabel_);
    commandLineStatusLabel_->style()->polish(commandLineStatusLabel_);
    commandLineStatusLabel_->update();
}

void EditorShell::setChannelBoxEnabled(bool enabled)
{
    EditorInspectorController::setInspectorEnabled(inspectorWidgets(), enabled);
}

EditorInspectorController::InspectorWidgets EditorShell::inspectorWidgets() const
{
    return EditorShellContexts::buildInspectorWidgets(
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

EditorInspectorController::InspectorActions EditorShell::inspectorActions() const
{
    const auto& a = editorMenuBar_->actions();
    return EditorShellContexts::buildInspectorActions(
        a.markHierarchyParent,
        a.parentToMarkedParent,
        a.unparentSelected,
        a.bindSkin,
        a.resetJointOrientation,
        a.alignJointOrientation,
        a.captureBindPose,
        a.captureBindPoseRecursive,
        a.frameSelected,
        frameSelectedButton_,
        resetJointOrientationButton_,
        alignJointOrientationButton_,
        captureBindPoseButton_,
        captureBindPoseRecursiveButton_);
}

EditorChannelBoxController::Context EditorShell::channelBoxContext() const
{
    return EditorShellContexts::buildChannelBoxContext(viewport_);
}

EditorCreationController::Context EditorShell::creationContext() const
{
    return EditorShellContexts::buildCreationContext(
        [this](const QString& prefix) {
            return generateUniqueScriptName(prefix);
        },
        [this](const QString& baseName, std::uint64_t ignoreObjectId) {
            return generateUniqueObjectName(baseName, ignoreObjectId);
        });
}

EditorOutlinerController::SceneAccess EditorShell::outlinerSceneAccess() const
{
    return EditorShellContexts::buildOutlinerSceneAccess(viewport_);
}

EditorRiggingController::Context EditorShell::riggingContext() const
{
    return EditorShellContexts::buildRiggingContext(viewport_);
}

EditorSceneRuntimeController::Context EditorShell::sceneRuntimeContext()
{
    EditorSceneRuntimeController::Context context;
    context.recordUndoState = [this]() {
        recordUndoState();
    };
    context.animationState = [this]() {
        return animationState_;
    };
    context.applyAnimationState = [this](const EditorAnimationState& state) {
        applyAnimationState(state, false);
    };
    context.replaceScene = [this](const Scene& scene) {
        viewport_->replaceScene(scene);
    };
    context.refreshScenePanels = [this]() {
        refreshScenePanels();
    };
    context.containsObject = [this](std::uint64_t objectId) {
        return viewport_->containsObject(objectId);
    };
    context.selectObject = [this](std::uint64_t objectId, bool syncOutliner) {
        selectObject(objectId, syncOutliner);
    };
    context.clearInspector = [this]() {
        clearInspector();
    };
    context.frameScene = [this]() {
        viewport_->frameScene();
    };
    return context;
}

EditorScriptExecutionController::ExecutionContext EditorShell::scriptExecutionContext()
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

EditorSelectionController::Context EditorShell::selectionContext() const
{
    return EditorShellContexts::buildSelectionContext(
        viewport_,
        outlinerTree_,
        scriptHistoryTextEdit_,
        inspectorWidgets(),
        inspectorActions(),
        outlinerSceneAccess());
}

EditorViewportUiController::ActionSet EditorShell::viewportUiActions() const
{
    const auto& a = editorMenuBar_->actions();
    return EditorShellContexts::buildViewportUiActions(
        a.translate,
        a.rotate,
        a.scale,
        a.perspectiveCamera,
        a.frontCamera,
        a.backCamera,
        a.leftCamera,
        a.rightCamera,
        a.topCamera,
        a.bottomCamera,
        a.worldAxis,
        a.localAxis);
}

bool EditorShell::showErrorMessageIfPresent(const QString& errorMessage, int timeoutMs)
{
    if (errorMessage.isEmpty()) {
        return false;
    }

    statusBar()->showMessage(errorMessage, timeoutMs);
    return true;
}

void EditorShell::appendScriptResultLogLines(const QString& commentLine, const QString& commandLine, const QString& resultLine)
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

void EditorShell::showStatusMessageIfPresent(const QString& statusMessage, int timeoutMs)
{
    if (!statusMessage.isEmpty()) {
        statusBar()->showMessage(statusMessage, timeoutMs);
    }
}

void EditorShell::applyRiggingOperationResult(const EditorRiggingController::OperationResult& result, bool logToScript)
{
    if (!result.success) {
        showErrorMessageIfPresent(result.errorMessage, 1500);
        return;
    }

    if (result.markedHierarchyParentId != 0) {
        markedHierarchyParentId_ = result.markedHierarchyParentId;
    }

    if (result.hasScene) {
        EditorSceneRuntimeController::ApplySceneRequest request;
        request.scene = result.scene;
        request.recordUndo = true;
        request.selection.objectId = result.focusObjectId;
        EditorSceneRuntimeController::applyScene(sceneRuntimeContext(), request);
    } else if (result.focusObjectId != 0) {
        updateInspector(result.focusObjectId);
    }

    if (logToScript) {
        appendScriptResultLogLines(result.commentLine, result.commandLine, result.resultLine);
    }

    showStatusMessageIfPresent(result.statusMessage, 2000);
}

void EditorShell::applyFileFlowResult(const EditorFileFlowController::OperationResult& result, bool logToScript, int timeoutMs)
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

EditorAnimationFlowController::Context EditorShell::animationFlowContext() const
{
    return EditorShellContexts::buildAnimationFlowContext(viewport_);
}

EditorAnimationEngineFacade::Context EditorShell::animationEngineContext()
{
    EditorAnimationEngineFacade::Context context;
    context.flow = animationFlowContext();
    context.runtime = sceneRuntimeContext();
    context.animationState = [this]() {
        return animationState_;
    };
    context.applyAnimationState = [this](const EditorAnimationState& state) {
        applyAnimationState(state, false);
    };
    return context;
}

void EditorShell::applyAnimationEngineResult(const EditorAnimationEngineFacade::OperationResult& result)
{
    if (!result.success) {
        showErrorMessageIfPresent(result.errorMessage, 1500);
        return;
    }

    appendScriptResultLogLines(result.commentLine, result.commandLine, result.resultLine);
    showStatusMessageIfPresent(result.statusMessage, 1500);
}

void EditorShell::applyAnimationFlowResult(const EditorAnimationFlowController::OperationResult& result)
{
    applyAnimationEngineResult(
        EditorAnimationEngineFacade::applyFlowResult(animationEngineContext(), result));
}

void EditorShell::applyCreationResult(const EditorCreationController::OperationResult& result)
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

void EditorShell::applyChannelBoxOperationResult(const EditorChannelBoxController::OperationResult& result)
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
        editorMenuBar_->actions().frameSelected->setEnabled(canFrame);
        EditorScriptLogController::logVisibilityChange(scriptHistoryTextEdit_, object, result.visible);
    } else if (!result.commandLine.isEmpty() || !result.resultLine.isEmpty()) {
        appendScriptResultLogLines(QString(), result.commandLine, result.resultLine);
    } else if (object != nullptr) {
        EditorScriptLogController::logTransformChange(scriptHistoryTextEdit_, object, result.transform);
    }

    showStatusMessageIfPresent(result.statusMessage, 1200);
}

void EditorShell::applyViewportUiOperationResult(const EditorViewportUiController::OperationResult& result, int timeoutMs)
{
    if (!result.success) {
        return;
    }

    appendScriptResultLogLines(result.commentLine, result.commandLine, result.resultLine);
    showStatusMessageIfPresent(result.statusMessage, timeoutMs);
}

void EditorShell::applyChannelBoxToSelection()
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

void EditorShell::applyJointOrientationToSelection()
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

void EditorShell::applyVisibilityToSelection(bool visible)
{
    if (updatingChannelBox_ || selectedObjectId_ == 0) {
        return;
    }

    applyChannelBoxOperationResult(
        EditorChannelBoxController::applyVisibility(channelBoxContext(), selectedObjectId_, visible));
}
