#include "EditorShell.h"

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
#include <QSignalBlocker>
#include <QSlider>
#include <QSpinBox>
#include <QStatusBar>
#include <QStyle>
#include <QToolBar>
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>
#include <QTreeWidgetItem>
#include <QHBoxLayout>
#include <QFrame>
#include <QVBoxLayout>

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

QString editorShellStyleSheet()
{
    return
        "QMainWindow { background: #25282d; color: #d8dde6; }"
        "QWidget { color: #d8dde6; font-size: 12px; }"
        "QMenuBar { background: #2c3035; color: #e6ebf2; border-bottom: 1px solid #3a4047; }"
        "QMenuBar::item { background: transparent; padding: 7px 12px; }"
        "QMenuBar::item:selected { background: #3a4048; }"
        "QMenu { background: #2b2f34; color: #e6ebf2; border: 1px solid #434952; padding: 4px 0; }"
        "QMenu::item { padding: 6px 20px; }"
        "QMenu::separator { height: 1px; background: #3c424a; margin: 4px 8px; }"
        "QMenu::item:selected { background: #4e667a; }"
        "QToolBar { background: #2b2f34; border: none; spacing: 4px; padding: 5px; }"
        "QToolBar::separator { background: #3e444c; width: 1px; margin: 4px 6px; }"
        "QToolButton { background: #393f46; color: #dbe1ea; border: 1px solid #4c525b; border-radius: 4px; padding: 4px 8px; }"
        "QToolButton:hover { background: #444b54; }"
        "QToolButton:checked { background: #4f6b82; border-color: #6d8aa3; }"
        "QStatusBar { background: #2a2e33; color: #c6ccd6; border-top: 1px solid #3a4047; }"
        "QLabel#activePanelStatusLabel { color: #9fb8cf; font-weight: 600; padding: 0 8px; }"
        "QDockWidget { background: #262a2f; }"
        "QDockWidget::title { background: #32363d; color: #eef2f7; padding: 7px 10px; border-bottom: 1px solid #434952; }"
        "QDockWidget[activePanel=\"true\"]::title { background: #455766; color: #f4f7fb; border-bottom: 1px solid #6d8aa3; }"
        "QDockWidget::close-button, QDockWidget::float-button { background: transparent; border: none; padding: 2px; }"
        "QDockWidget::close-button:hover, QDockWidget::float-button:hover { background: #414750; border-radius: 3px; }"
        "QWidget[panelSurface=\"true\"] { border: 1px solid #2d3137; }"
        "QWidget[panelSurface=\"true\"][activePanel=\"true\"] { border: 1px solid #6d8aa3; }"
        "QTreeWidget, QListWidget, QPlainTextEdit { background: #23272c; color: #dde3ec; border: 1px solid #3f454d; }"
        "QHeaderView::section { background: #2f3338; color: #c9d0da; border: none; border-bottom: 1px solid #3f454d; padding: 6px 8px; }"
        "QAbstractItemView::item { padding: 4px 6px; }"
        "QAbstractItemView::item:selected { background: #536c82; color: #f3f6fa; }"
        "QAbstractItemView::item:hover { background: #343a42; }"
        "QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox { background: #30353b; color: #eef2f7; border: 1px solid #4a5058; border-radius: 4px; padding: 4px 6px; }"
        "QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus { border-color: #6d8aa3; }"
        "QLineEdit:disabled, QSpinBox:disabled, QDoubleSpinBox:disabled, QComboBox:disabled { background: #2a2e33; color: #7c848f; border-color: #3c4047; }"
        "QPushButton { background: #393f46; color: #e7ebf1; border: 1px solid #4c525b; border-radius: 4px; padding: 4px 10px; }"
        "QPushButton:hover { background: #444b53; }"
        "QPushButton:pressed { background: #2f3338; }"
        "QPushButton:disabled { background: #2b2f34; color: #7a818c; border-color: #3b4047; }"
        "QPushButton[variant=\"transport\"] { background: transparent; color: #cfd5dd; border: none; padding: 0px; }"
        "QPushButton[variant=\"transport\"]:hover { background: #3a4048; border-radius: 3px; color: #f4f7fb; }"
        "QPushButton[variant=\"transport\"]:pressed { background: #31363d; }"
        "QPushButton[tone=\"warning\"] { background: #8f6832; color: #f5f7fa; border-color: #ab8144; font-weight: 600; }"
        "QPushButton[tone=\"danger\"] { background: #6d3a3a; color: #f5f7fa; border-color: #8b4d4d; font-weight: 600; }"
        "QPushButton[tone=\"muted\"] { background: #4b5058; color: #f5f7fa; border-color: #5c636d; }"
        "QCheckBox { spacing: 6px; }"
        "QWidget#channelBoxPanel { background: #262a2f; }"
        "QLabel#inspectorEmptyStateLabel { color: #97a1ae; padding: 0 0 4px 0; }"
        "QLabel#channelObjectNameLabel { font-weight: 600; padding-bottom: 4px; color: #eef2f7; }"
        "QWidget#channelBoxSection { background: #2b2f34; border: 1px solid #3b4149; border-radius: 4px; }"
        "QLabel#channelSectionLabel { color: #c8ced7; font-size: 11px; font-weight: 600; padding: 0 0 2px 0; }"
        "QLabel#channelRowLabel { color: #9fa7b2; }"
        "QLabel#timelineStatusLabel[statusTone=\"muted\"] { color: #aeb6c1; }"
        "QLabel#timelineStatusLabel[statusTone=\"warning\"] { color: #d6b06e; font-weight: 600; }"
        "QLabel#timelineStatusLabel[statusTone=\"info\"] { color: #8eb3cf; }"
        "QWidget#commandLinePanel { background: #23272c; border-top: 1px solid #3a4047; }"
        "QLabel#commandLinePrefixLabel { color: #9ea7b2; font-size: 11px; font-weight: 600; letter-spacing: 0.4px; }"
        "QLabel#commandLineStatusLabel[statusTone=\"muted\"] { color: #aeb6c1; }"
        "QLabel#commandLineStatusLabel[statusTone=\"warning\"] { color: #d6b06e; font-weight: 600; }"
        "QLabel#commandLineStatusLabel[statusTone=\"info\"] { color: #8eb3cf; }"
        "QSpinBox[variant=\"frameBox\"] { background: #2f3338; color: #f0f3f8; border: 1px solid #4b515a; padding: 2px 6px; }"
        "QDoubleSpinBox[variant=\"channelBox\"] { background: #343941; color: #eef2f7; border: 1px solid #4a515a; border-radius: 3px; padding: 3px 6px; }"
        "QDoubleSpinBox[variant=\"channelBox\"]:focus { border-color: #6d8aa3; }"
        "QDoubleSpinBox[variant=\"channelBox\"]:disabled { background: #2a2e33; color: #7c848f; border-color: #3c4047; }"
        "QCheckBox[variant=\"channelVisibility\"] { color: #d8dde6; }"
        "QLabel[variant=\"channelMeta\"] { color: #9ea7b2; }"
        "QScrollBar:vertical { background: #272b30; width: 12px; margin: 0; }"
        "QScrollBar::handle:vertical { background: #4a515a; min-height: 24px; border-radius: 5px; }"
        "QScrollBar:horizontal { background: #272b30; height: 12px; margin: 0; }"
        "QScrollBar::handle:horizontal { background: #4a515a; min-width: 24px; border-radius: 5px; }"
        "QSplitter::handle { background: #2b2f34; }"
        "QSplitter::handle:hover { background: #3a4048; }";
}

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

void hideDockTitleBar(QDockWidget* dock)
{
    if (dock == nullptr) {
        return;
    }

    QWidget* titleBar = new QWidget(dock);
    titleBar->setFixedHeight(0);
    titleBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    dock->setTitleBarWidget(titleBar);
}

QWidget* createFloatingDockTitleBar(QDockWidget* dock)
{
    if (dock == nullptr) {
        return nullptr;
    }

    QWidget* titleBar = new QWidget(dock);
    titleBar->setObjectName("floatingDockTitleBar");
    titleBar->setFixedHeight(26);

    QHBoxLayout* layout = new QHBoxLayout(titleBar);
    layout->setContentsMargins(8, 3, 8, 3);
    layout->setSpacing(6);

    QLabel* label = new QLabel(dock->windowTitle(), titleBar);
    label->setStyleSheet("color: #eef2f7; font-weight: 600;");
    layout->addWidget(label, 1);

    return titleBar;
}

void updateDockTitleBarForFloatingState(QDockWidget* dock)
{
    if (dock == nullptr) {
        return;
    }

    if (dock->isFloating()) {
        dock->setTitleBarWidget(createFloatingDockTitleBar(dock));
        return;
    }

    hideDockTitleBar(dock);
}

void bindCompactDockTitleBehavior(QDockWidget* dock)
{
    if (dock == nullptr) {
        return;
    }

    updateDockTitleBarForFloatingState(dock);
    QObject::connect(dock, &QDockWidget::topLevelChanged, dock, [dock](bool floating) {
        updateDockTitleBarForFloatingState(dock);
        if (!floating) {
            if (auto* shell = static_cast<EditorShell*>(dock->parentWidget())) {
                shell->restoreBottomPanelLayout();
            }
        }
    });
}

}

EditorShell::EditorShell()
{
    setWindowTitle("Phoenix Editor Beta");
    resize(1440, 820);
    applyUnifiedTheme();
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

    createMenus();
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

void EditorShell::applyUnifiedTheme()
{
    setStyleSheet(editorShellStyleSheet());
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
    case ActivePanel::None:
    default:
        return "None";
    }
}

void EditorShell::createMenus()
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

void EditorShell::createEditMenu()
{
    QMenu* editMenu = menuBar()->addMenu("&Edit");
    undoAction_ = configureAction(editMenu->addAction("Undo"), "undoAction", true, false, QKeySequence::Undo);
    QObject::connect(undoAction_, &QAction::triggered, this, &EditorShell::undoLastChange);

    redoAction_ = configureAction(editMenu->addAction("Redo"), "redoAction", true, false, QKeySequence::Redo);
    QObject::connect(redoAction_, &QAction::triggered, this, &EditorShell::redoLastChange);

    editMenu->addSeparator();
}

void EditorShell::createFileMenu()
{
    QMenu* fileMenu = menuBar()->addMenu("&File");
    newSceneAction_ = configureAction(fileMenu->addAction("New Scene"), nullptr, true, false, QKeySequence::New);
    QObject::connect(newSceneAction_, &QAction::triggered, this, &EditorShell::newScene);

    openSceneAction_ = configureAction(fileMenu->addAction("Open Scene..."), nullptr, true, false, QKeySequence::Open);
    QObject::connect(openSceneAction_, &QAction::triggered, this, &EditorShell::openScene);

    saveSceneAction_ = configureAction(fileMenu->addAction("Save Scene"), nullptr, true, false, QKeySequence::Save);
    QObject::connect(saveSceneAction_, &QAction::triggered, this, &EditorShell::saveScene);

    saveSceneAsAction_ = configureAction(fileMenu->addAction("Save Scene As..."), nullptr, true, false, QKeySequence::SaveAs);
    QObject::connect(saveSceneAsAction_, &QAction::triggered, this, &EditorShell::saveSceneAs);

    incrementAndSaveAction_ = configureAction(
        fileMenu->addAction("Increment and Save"),
        nullptr,
        true,
        false,
        QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_S));
    QObject::connect(incrementAndSaveAction_, &QAction::triggered, this, &EditorShell::incrementAndSave);

    archiveSceneAction_ = fileMenu->addAction("Archive Scene");
    QObject::connect(archiveSceneAction_, &QAction::triggered, this, &EditorShell::archiveScene);

    savePreferencesAction_ = fileMenu->addAction("Save Preferences");
    QObject::connect(savePreferencesAction_, &QAction::triggered, this, &EditorShell::savePreferences);

    optimizeSceneSizeAction_ = fileMenu->addAction("Optimize Scene Size");
    QObject::connect(optimizeSceneSizeAction_, &QAction::triggered, this, &EditorShell::optimizeSceneStorage);

    fileMenu->addSection("Import/Export");
    importFbxAction_ = configureAction(fileMenu->addAction("Import..."), "importFbxAction");
    QObject::connect(importFbxAction_, &QAction::triggered, this, &EditorShell::importFbx);

    exportAllAction_ = fileMenu->addAction("Export All...");
    QObject::connect(exportAllAction_, &QAction::triggered, this, &EditorShell::exportAll);

    exportSelectionAction_ = fileMenu->addAction("Export Selection...");
    QObject::connect(exportSelectionAction_, &QAction::triggered, this, &EditorShell::exportSelection);
}

void EditorShell::createCreateMenu()
{
    QMenu* createMenu = menuBar()->addMenu("&Create");
    polygonPrimitivesAction_ = configureAction(createMenu->addAction("Polygon Primitives"), "polygonPrimitivesAction");
    QObject::connect(polygonPrimitivesAction_, &QAction::triggered, this, &EditorShell::showPolygonPrimitivesWindow);
    createJointAction_ = configureAction(createMenu->addAction("Joint"), "createJointAction");
    QObject::connect(createJointAction_, &QAction::triggered, this, &EditorShell::createJoint);
}

void EditorShell::createRigMenu()
{
    QMenu* rigMenu = menuBar()->addMenu("&Rig");
    markHierarchyParentAction_ = configureAction(
        rigMenu->addAction("Mark Selected As Parent"),
        "markHierarchyParentAction",
        false);
    QObject::connect(markHierarchyParentAction_, &QAction::triggered, this, &EditorShell::markSelectionAsHierarchyParent);

    parentToMarkedParentAction_ = configureAction(
        rigMenu->addAction("Parent Selected To Marked Parent"),
        "parentToMarkedParentAction",
        false,
        false,
        QKeySequence(Qt::Key_P));
    QObject::connect(parentToMarkedParentAction_, &QAction::triggered, this, &EditorShell::parentSelectionToMarkedParent);

    unparentSelectedAction_ = configureAction(
        rigMenu->addAction("Unparent Selected"),
        "unparentSelectedAction",
        false,
        false,
        QKeySequence(Qt::SHIFT | Qt::Key_P));
    QObject::connect(unparentSelectedAction_, &QAction::triggered, this, &EditorShell::unparentSelection);

    bindSkinAction_ = configureAction(rigMenu->addAction("Bind Selected Mesh To Marked Joint"), "bindSkinAction", false);
    QObject::connect(bindSkinAction_, &QAction::triggered, this, &EditorShell::bindSelectedMeshToMarkedJoint);

    rigMenu->addSeparator();
    resetJointOrientationAction_ = configureAction(
        rigMenu->addAction("Reset Joint Orientation"),
        "resetJointOrientationAction",
        false);
    QObject::connect(resetJointOrientationAction_, &QAction::triggered, this, &EditorShell::resetSelectedJointOrientation);

    alignJointOrientationAction_ = configureAction(
        rigMenu->addAction("Align Joint Orientation To Child"),
        "alignJointOrientationAction",
        false);
    QObject::connect(alignJointOrientationAction_, &QAction::triggered, this, &EditorShell::alignSelectedJointOrientationToChild);

    captureBindPoseAction_ = configureAction(rigMenu->addAction("Capture Bind Pose"), "captureBindPoseAction", false);
    QObject::connect(captureBindPoseAction_, &QAction::triggered, this, &EditorShell::captureSelectedBindPose);

    captureBindPoseRecursiveAction_ = configureAction(
        rigMenu->addAction("Capture Bind Pose Recursive"),
        "captureBindPoseRecursiveAction",
        false);
    QObject::connect(captureBindPoseRecursiveAction_, &QAction::triggered, this, &EditorShell::captureSelectedBindPoseRecursive);
}

void EditorShell::createWindowsMenu()
{
    QMenu* windowsMenu = menuBar()->addMenu("&Windows");
    scriptEditorAction_ = configureAction(windowsMenu->addAction("Script Editor"), "scriptEditorAction");
    QObject::connect(scriptEditorAction_, &QAction::triggered, this, &EditorShell::showScriptEditorWindow);
    graphEditorAction_ = configureAction(windowsMenu->addAction("Graph Editor"), "graphEditorAction");
    QObject::connect(graphEditorAction_, &QAction::triggered, this, &EditorShell::showGraphEditorWindow);
}

void EditorShell::createAnimationMenu()
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

void EditorShell::createViewMenu()
{
    QMenu* viewMenu = menuBar()->addMenu("&View");

    resetCameraAction_ = viewMenu->addAction("Reset Camera");
    QObject::connect(resetCameraAction_, &QAction::triggered, this, &EditorShell::resetSceneCamera);

    frameSceneAction_ = viewMenu->addAction("Frame Scene");
    QObject::connect(frameSceneAction_, &QAction::triggered, this, &EditorShell::frameEntireScene);

    frameSelectedAction_ = configureAction(viewMenu->addAction("Frame Selected"), nullptr, false);
    QObject::connect(frameSelectedAction_, &QAction::triggered, this, &EditorShell::frameSelectedObject);

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
    QObject::connect(wireframeAction_, &QAction::toggled, this, &EditorShell::setWireframeDisplayEnabled);

    showAxisAction_ = configureAction(viewMenu->addAction("Show Axis"), nullptr, true, true);
    showAxisAction_->setChecked(true);
    QObject::connect(showAxisAction_, &QAction::toggled, this, &EditorShell::setAxisVisibilityEnabled);

    backfaceCullingAction_ = configureAction(viewMenu->addAction("Backface Culling"), nullptr, true, true);
    QObject::connect(backfaceCullingAction_, &QAction::toggled, this, &EditorShell::setBackfaceCullingEnabled);

    viewMenu->addSeparator();
    restoreWorkspaceLayoutAction_ = configureAction(
        viewMenu->addAction("Restore Default Layout"),
        "restoreWorkspaceLayoutAction");
    QObject::connect(restoreWorkspaceLayoutAction_, &QAction::triggered, this, &EditorShell::restoreDefaultWorkspaceLayout);
}

void EditorShell::createTransformMenu()
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

void EditorShell::createToolbar()
{
    toolbar_ = addToolBar("Viewport");
    toolbar_->setMovable(false);
    addImportCreateToolbarSection();
    addRigToolbarSection();
    addViewToolbarSection();
    addTransformToolbarSection();
    addDisplayToolbarSection();
}

void EditorShell::addImportCreateToolbarSection()
{
    toolbar_->addAction(importFbxAction_);
    toolbar_->addAction(polygonPrimitivesAction_);
    toolbar_->addAction(createJointAction_);
}

void EditorShell::addRigToolbarSection()
{
    toolbar_->addSeparator();
    toolbar_->addAction(markHierarchyParentAction_);
    toolbar_->addAction(parentToMarkedParentAction_);
    toolbar_->addAction(unparentSelectedAction_);
    toolbar_->addAction(alignJointOrientationAction_);
    toolbar_->addAction(captureBindPoseAction_);
}

void EditorShell::addViewToolbarSection()
{
    toolbar_->addSeparator();
    toolbar_->addAction(resetCameraAction_);
    toolbar_->addAction(frameSceneAction_);
    toolbar_->addAction(frameSelectedAction_);
}

void EditorShell::addTransformToolbarSection()
{
    toolbar_->addSeparator();
    toolbar_->addAction(translateAction_);
    toolbar_->addAction(rotateAction_);
    toolbar_->addAction(scaleAction_);
    toolbar_->addAction(worldAxisAction_);
    toolbar_->addAction(localAxisAction_);
}

void EditorShell::addDisplayToolbarSection()
{
    toolbar_->addSeparator();
    toolbar_->addAction(wireframeAction_);
    toolbar_->addAction(showAxisAction_);
    toolbar_->addAction(backfaceCullingAction_);
}

void EditorShell::createDocks()
{
    viewport_->setObjectName("viewportWidget");
    viewport_->setProperty("panelSurface", "true");
    setCentralWidget(viewport_);
    installPanelActivationTracking(viewport_, ActivePanel::Viewport);
    viewportDock_ = nullptr;
    createOutlinerDock();
    createInspectorDock();
    resizeDocks({ outlinerDock_, inspectorDock_ }, { 280, 320 }, Qt::Horizontal);
    createPrimitivePaletteDock();
    createScriptEditorDock();
    createTimelineDock();
    createRangeSliderDock();
    createCommandLineDock();
    createGraphEditorDock();
}

void EditorShell::createOutlinerDock()
{
    outlinerDock_ = new QDockWidget("Outliner", this);
    configureDockWidget(
        outlinerDock_,
        "OutlinerDock",
        Qt::AllDockWidgetAreas,
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    QWidget* panel = createOutlinerPanel();
    panel->setProperty("panelSurface", "true");
    outlinerDock_->setWidget(panel);
    installPanelActivationTracking(panel, ActivePanel::Outliner, outlinerDock_);
    addDockWidget(Qt::LeftDockWidgetArea, outlinerDock_);
}

void EditorShell::createInspectorDock()
{
    inspectorDock_ = new QDockWidget("Channel Box", this);
    configureDockWidget(
        inspectorDock_,
        "InspectorDock",
        Qt::AllDockWidgetAreas,
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    QWidget* panel = createInspectorPanel();
    panel->setProperty("panelSurface", "true");
    inspectorDock_->setWidget(panel);
    installPanelActivationTracking(panel, ActivePanel::ChannelBox, inspectorDock_);
    addDockWidget(Qt::RightDockWidgetArea, inspectorDock_);
}

void EditorShell::createPrimitivePaletteDock()
{
    polygonPrimitivesDock_ = new QDockWidget("Polygon Primitives", this);
    configureDockWidget(
        polygonPrimitivesDock_,
        "PolygonPrimitivesDock",
        Qt::AllDockWidgetAreas,
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);
    QWidget* panel = createPrimitivePalettePanel();
    panel->setProperty("panelSurface", "true");
    polygonPrimitivesDock_->setWidget(panel);
    installPanelActivationTracking(panel, ActivePanel::PrimitivePalette, polygonPrimitivesDock_);
    addDockWidget(Qt::RightDockWidgetArea, polygonPrimitivesDock_);
    polygonPrimitivesDock_->setFloating(true);
    polygonPrimitivesDock_->hide();
}

void EditorShell::createScriptEditorDock()
{
    scriptEditorDock_ = new QDockWidget("Script Editor", this);
    configureDockWidget(
        scriptEditorDock_,
        "ScriptEditorDock",
        Qt::AllDockWidgetAreas,
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);
    QWidget* panel = createScriptEditorPanel();
    panel->setProperty("panelSurface", "true");
    scriptEditorDock_->setWidget(panel);
    installPanelActivationTracking(panel, ActivePanel::ScriptEditor, scriptEditorDock_);
    addDockWidget(Qt::BottomDockWidgetArea, scriptEditorDock_);
    scriptEditorDock_->setFloating(true);
    scriptEditorDock_->hide();
}

void EditorShell::createTimelineDock()
{
    timeSliderDock_ = new QDockWidget("Time Slider", this);
    configureDockWidget(
        timeSliderDock_,
        "TimeSliderDock",
        Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea,
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    QWidget* panel = createTimeSliderPanel();
    panel->setProperty("panelSurface", "true");
    timeSliderDock_->setWidget(panel);
    bindCompactDockTitleBehavior(timeSliderDock_);
    installPanelActivationTracking(panel, ActivePanel::TimeSlider, timeSliderDock_);
    addDockWidget(Qt::BottomDockWidgetArea, timeSliderDock_);
    resizeDocks({ timeSliderDock_ }, { 150 }, Qt::Vertical);
}

void EditorShell::createRangeSliderDock()
{
    rangeSliderDock_ = new QDockWidget("Range Slider", this);
    configureDockWidget(
        rangeSliderDock_,
        "RangeSliderDock",
        Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea,
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    QWidget* panel = createRangeSliderPanel();
    panel->setProperty("panelSurface", "true");
    rangeSliderDock_->setWidget(panel);
    bindCompactDockTitleBehavior(rangeSliderDock_);
    installPanelActivationTracking(panel, ActivePanel::RangeSlider, rangeSliderDock_);
    addDockWidget(Qt::BottomDockWidgetArea, rangeSliderDock_);
    splitDockWidget(timeSliderDock_, rangeSliderDock_, Qt::Vertical);
    resizeDocks({ timeSliderDock_, rangeSliderDock_ }, { 112, 56 }, Qt::Vertical);
}

void EditorShell::createCommandLineDock()
{
    commandLineDock_ = new QDockWidget("Command Line", this);
    configureDockWidget(
        commandLineDock_,
        "CommandLineDock",
        Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea,
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    QWidget* panel = createCommandLinePanel();
    panel->setProperty("panelSurface", "true");
    commandLineDock_->setWidget(panel);
    bindCompactDockTitleBehavior(commandLineDock_);
    installPanelActivationTracking(panel, ActivePanel::CommandLine, commandLineDock_);
    addDockWidget(Qt::BottomDockWidgetArea, commandLineDock_);
    splitDockWidget(rangeSliderDock_ != nullptr ? rangeSliderDock_ : timeSliderDock_, commandLineDock_, Qt::Vertical);
    if (rangeSliderDock_ != nullptr) {
        resizeDocks({ timeSliderDock_, rangeSliderDock_, commandLineDock_ }, { 112, 56, 34 }, Qt::Vertical);
    } else {
        resizeDocks({ timeSliderDock_, commandLineDock_ }, { 150, 34 }, Qt::Vertical);
    }
}

void EditorShell::createGraphEditorDock()
{
    graphEditorDock_ = new QDockWidget("Graph Editor", this);
    configureDockWidget(
        graphEditorDock_,
        "GraphEditorDock",
        Qt::AllDockWidgetAreas,
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);
    QWidget* panel = createGraphEditorPanel();
    panel->setProperty("panelSurface", "true");
    graphEditorDock_->setWidget(panel);
    installPanelActivationTracking(panel, ActivePanel::GraphEditor, graphEditorDock_);
    addDockWidget(Qt::BottomDockWidgetArea, graphEditorDock_);
    tabifyDockWidget(timeSliderDock_, graphEditorDock_);
    graphEditorDock_->hide();
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

    jointToolsWidget_ = new QWidget(inspectorDetailsWidget_);
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

    QVBoxLayout* jointSectionLayout = nullptr;
    QWidget* jointSection = createChannelSection("Joint Tools", inspectorDetailsWidget_, &jointSectionLayout);
    jointSectionLayout->addWidget(jointToolsWidget_);

    inspectorDetailsLayout->addWidget(transformSection);
    inspectorDetailsLayout->addWidget(jointSection);

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
    QObject::connect(visibilityCheckBox_, &QCheckBox::toggled, this, &EditorShell::applyVisibilityToSelection);
    QObject::connect(resetJointOrientationButton_, &QPushButton::clicked, this, &EditorShell::resetSelectedJointOrientation);
    QObject::connect(alignJointOrientationButton_, &QPushButton::clicked, this, &EditorShell::alignSelectedJointOrientationToChild);
    QObject::connect(captureBindPoseButton_, &QPushButton::clicked, this, &EditorShell::captureSelectedBindPose);
    QObject::connect(captureBindPoseRecursiveButton_, &QPushButton::clicked, this, &EditorShell::captureSelectedBindPoseRecursive);

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

QWidget* EditorShell::createTimeSliderPanel()
{
    QWidget* panel = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    animationTimelinePanel_ = new AnimationTimelinePanel(panel);
    animationTimelinePanel_->bindTimelineActions(
        duplicateKeyAction_,
        shiftKeysLeftAction_,
        shiftKeysRightAction_,
        previousKeyAction_,
        nextKeyAction_);
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
    graphEditorPanel_->setViewModel(buildGraphEditorViewModel());
    graphEditorPanel_->setCurrentFrameChangedCallback([this](int frame) {
        if (!updatingTimeSlider_) {
            playbackController_.setCurrentFrame(frame);
        }
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
        translateAction_->setChecked(translateChecked);
        rotateAction_->setChecked(rotateChecked);
        scaleAction_->setChecked(scaleChecked);
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
    if (undoAction_ != nullptr) {
        undoAction_->setEnabled(historyController_.canUndo());
    }
    if (redoAction_ != nullptr) {
        redoAction_->setEnabled(historyController_.canRedo());
    }
}

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
        points.reserve(object->transformKeyframes().size());
        for (const TransformKeyframe& keyframe : object->transformKeyframes()) {
            points.append({ keyframe.frame, valueFn(keyframe) });
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
    if (outlinerDock_ == nullptr || inspectorDock_ == nullptr || timeSliderDock_ == nullptr) {
        return;
    }

    outlinerDock_->setFloating(false);
    inspectorDock_->setFloating(false);
    timeSliderDock_->setFloating(false);
    if (rangeSliderDock_ != nullptr) {
        rangeSliderDock_->setFloating(false);
    }
    if (commandLineDock_ != nullptr) {
        commandLineDock_->setFloating(false);
    }
    if (graphEditorDock_ != nullptr) {
        graphEditorDock_->setFloating(false);
    }

    addDockWidget(Qt::LeftDockWidgetArea, outlinerDock_);
    addDockWidget(Qt::RightDockWidgetArea, inspectorDock_);
    addDockWidget(Qt::BottomDockWidgetArea, timeSliderDock_);
    if (rangeSliderDock_ != nullptr) {
        addDockWidget(Qt::BottomDockWidgetArea, rangeSliderDock_);
        splitDockWidget(timeSliderDock_, rangeSliderDock_, Qt::Vertical);
    }
    if (commandLineDock_ != nullptr) {
        addDockWidget(Qt::BottomDockWidgetArea, commandLineDock_);
        splitDockWidget(rangeSliderDock_ != nullptr ? rangeSliderDock_ : timeSliderDock_, commandLineDock_, Qt::Vertical);
    }
    if (graphEditorDock_ != nullptr) {
        addDockWidget(Qt::BottomDockWidgetArea, graphEditorDock_);
        tabifyDockWidget(timeSliderDock_, graphEditorDock_);
    }
    resizeDocks({ outlinerDock_, inspectorDock_ }, { 280, 320 }, Qt::Horizontal);
    if (rangeSliderDock_ != nullptr && commandLineDock_ != nullptr) {
        resizeDocks({ timeSliderDock_, rangeSliderDock_, commandLineDock_ }, { 112, 56, 34 }, Qt::Vertical);
    } else if (commandLineDock_ != nullptr) {
        resizeDocks({ timeSliderDock_, commandLineDock_ }, { 150, 34 }, Qt::Vertical);
    } else {
        resizeDocks({ timeSliderDock_ }, { 150 }, Qt::Vertical);
    }

    outlinerDock_->show();
    inspectorDock_->show();
    timeSliderDock_->show();
    if (rangeSliderDock_ != nullptr) {
        rangeSliderDock_->show();
    }
    if (commandLineDock_ != nullptr) {
        commandLineDock_->show();
    }
    if (graphEditorDock_ != nullptr) {
        graphEditorDock_->show();
        timeSliderDock_->raise();
    }
    applyViewportUiOperationResult(EditorViewportUiController::buildWorkspaceLayoutResult(), 2000);
}

void EditorShell::restoreBottomPanelLayout()
{
    if (timeSliderDock_ == nullptr) {
        return;
    }

    timeSliderDock_->setFloating(false);
    addDockWidget(Qt::BottomDockWidgetArea, timeSliderDock_);

    if (rangeSliderDock_ != nullptr) {
        rangeSliderDock_->setFloating(false);
        addDockWidget(Qt::BottomDockWidgetArea, rangeSliderDock_);
        splitDockWidget(timeSliderDock_, rangeSliderDock_, Qt::Vertical);
    }

    if (commandLineDock_ != nullptr) {
        commandLineDock_->setFloating(false);
        addDockWidget(Qt::BottomDockWidgetArea, commandLineDock_);
        splitDockWidget(rangeSliderDock_ != nullptr ? rangeSliderDock_ : timeSliderDock_, commandLineDock_, Qt::Vertical);
    }

    if (graphEditorDock_ != nullptr) {
        graphEditorDock_->setFloating(false);
        addDockWidget(Qt::BottomDockWidgetArea, graphEditorDock_);
        tabifyDockWidget(timeSliderDock_, graphEditorDock_);
    }

    if (rangeSliderDock_ != nullptr && commandLineDock_ != nullptr) {
        resizeDocks({ timeSliderDock_, rangeSliderDock_, commandLineDock_ }, { 112, 56, 34 }, Qt::Vertical);
    } else if (commandLineDock_ != nullptr) {
        resizeDocks({ timeSliderDock_, commandLineDock_ }, { 150, 34 }, Qt::Vertical);
    } else {
        resizeDocks({ timeSliderDock_ }, { 150 }, Qt::Vertical);
    }

    timeSliderDock_->show();
    if (rangeSliderDock_ != nullptr) {
        rangeSliderDock_->show();
    }
    if (commandLineDock_ != nullptr) {
        commandLineDock_->show();
    }
    if (graphEditorDock_ != nullptr) {
        graphEditorDock_->show();
        timeSliderDock_->raise();
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
        frameSelectedAction_->setEnabled(canFrame);
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
    return EditorShellContexts::buildInspectorActions(
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
    return EditorShellContexts::buildViewportUiActions(
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
        frameSelectedAction_->setEnabled(canFrame);
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
