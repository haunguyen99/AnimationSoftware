#include "ViewportWorkspaceWidget.h"

#include <QEvent>
#include <QFrame>
#include <QGridLayout>
#include <QAction>
#include <QKeySequence>
#include <QKeyEvent>
#include <QMenu>
#include <QMenuBar>
#include <QMouseEvent>
#include <QShortcut>
#include <QVBoxLayout>

namespace
{
QString inactiveFrameStyle()
{
    return "QFrame { border: 1px solid #343434; background: #1d1f22; }";
}

QString activeFrameStyle()
{
    return "QFrame { border: 2px solid #7bc6ff; background: #1d1f22; }";
}

QString viewportMenuBarStyle()
{
    return "QMenuBar { background: #2b2d31; color: #f1f1f1; padding: 1px 4px; }"
           "QMenuBar::item { background: transparent; padding: 4px 8px; }"
           "QMenuBar::item:selected { background: #3a3d42; }"
           "QMenu { background: #2b2d31; color: #f1f1f1; border: 1px solid #44484f; }"
           "QMenu::item:selected { background: #3d7ea6; }";
}
}

ViewportWorkspaceWidget::ViewportWorkspaceWidget(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    layoutHost_ = new QWidget(this);
    gridLayout_ = new QGridLayout(layoutHost_);
    gridLayout_->setContentsMargins(0, 0, 0, 0);
    gridLayout_->setSpacing(2);
    rootLayout->addWidget(layoutHost_);

    configurePane(perspectivePane_, "perspectiveViewportWidget", CameraViewPreset::Perspective);
    configurePane(topPane_, "topViewportWidget", CameraViewPreset::Top);
    configurePane(frontPane_, "frontViewportWidget", CameraViewPreset::Front);
    configurePane(sidePane_, "sideViewportWidget", CameraViewPreset::Right);

    activeViewport_ = perspectivePane_.viewport;
    refreshActiveViewportFrameStyles();
    rebuildViewportLayout();

    auto* toggleShortcut = new QShortcut(QKeySequence(Qt::Key_Space), this);
    toggleShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    QObject::connect(toggleShortcut, &QShortcut::activated, this, [this]() {
        toggleQuadView();
    });
}

const Scene& ViewportWorkspaceWidget::scene() const
{
    return activeViewport_->scene();
}

Scene ViewportWorkspaceWidget::sceneSnapshot() const
{
    return activeViewport_->scene();
}

bool ViewportWorkspaceWidget::containsObject(SceneObject::Id objectId) const
{
    return activeViewport_->scene().contains(objectId);
}

const SceneObject* ViewportWorkspaceWidget::findObject(SceneObject::Id objectId) const
{
    return activeViewport_->scene().findObject(objectId);
}

QVector<SceneObject::Id> ViewportWorkspaceWidget::rootObjectIds() const
{
    return activeViewport_->scene().rootObjectIds();
}

QVector<SceneObject::Id> ViewportWorkspaceWidget::allObjectIds() const
{
    return activeViewport_->scene().allObjectIds();
}

bool ViewportWorkspaceWidget::isSceneEmpty() const
{
    return activeViewport_->scene().isEmpty();
}

void ViewportWorkspaceWidget::requestRender()
{
    for (ViewportWidget* viewport : allViewports()) {
        viewport->requestRender();
    }
}

void ViewportWorkspaceWidget::resetCamera()
{
    activeViewport_->resetCamera();
}

void ViewportWorkspaceWidget::setCameraViewPreset(CameraViewPreset preset)
{
    activeViewport_->setCameraViewPreset(preset);
}

ViewportWorkspaceWidget::CameraViewPreset ViewportWorkspaceWidget::cameraViewPreset() const
{
    return activeViewport_->cameraViewPreset();
}

QString ViewportWorkspaceWidget::cameraViewLabel() const
{
    return activeViewport_->cameraViewLabel();
}

void ViewportWorkspaceWidget::frameScene()
{
    activeViewport_->frameScene();
}

void ViewportWorkspaceWidget::frameObject(SceneObject::Id objectId)
{
    activeViewport_->frameObject(objectId);
}

void ViewportWorkspaceWidget::setSelectedObject(SceneObject::Id objectId)
{
    selectedObjectId_ = objectId;
    syncSelectionToAllViewports();
}

SceneObject::Id ViewportWorkspaceWidget::selectedObject() const
{
    return selectedObjectId_;
}

void ViewportWorkspaceWidget::setWireframeEnabled(bool enabled)
{
    wireframeEnabled_ = enabled;
    for (ViewportWidget* viewport : allViewports()) {
        viewport->setWireframeEnabled(enabled);
    }
    refreshViewportMenuStates();
}

void ViewportWorkspaceWidget::setAxisVisible(bool visible)
{
    axisVisible_ = visible;
    for (ViewportWidget* viewport : allViewports()) {
        viewport->setAxisVisible(visible);
    }
    refreshViewportMenuStates();
}

void ViewportWorkspaceWidget::setBackfaceCullingEnabled(bool enabled)
{
    backfaceCullingEnabled_ = enabled;
    for (ViewportWidget* viewport : allViewports()) {
        viewport->setBackfaceCullingEnabled(enabled);
    }
    refreshViewportMenuStates();
}

void ViewportWorkspaceWidget::setSelectionOutlineVisible(bool visible)
{
    selectionOutlineVisible_ = visible;
    for (ViewportWidget* viewport : allViewports()) {
        viewport->setSelectionOutlineVisible(visible);
    }
    refreshViewportMenuStates();
}

void ViewportWorkspaceWidget::setTransformMode(TransformMode mode)
{
    transformMode_ = mode;
    for (ViewportWidget* viewport : allViewports()) {
        viewport->setTransformMode(mode);
    }
}

ViewportWorkspaceWidget::TransformMode ViewportWorkspaceWidget::transformMode() const
{
    return transformMode_;
}

void ViewportWorkspaceWidget::setAxisOrientation(AxisOrientation orientation)
{
    axisOrientation_ = orientation;
    for (ViewportWidget* viewport : allViewports()) {
        viewport->setAxisOrientation(orientation);
    }
}

ViewportWorkspaceWidget::AxisOrientation ViewportWorkspaceWidget::axisOrientation() const
{
    return axisOrientation_;
}

void ViewportWorkspaceWidget::setSelectionChangedCallback(std::function<void(SceneObject::Id)> callback)
{
    selectionChangedCallback_ = std::move(callback);
}

void ViewportWorkspaceWidget::setObjectTransformChangedCallback(std::function<void(SceneObject::Id)> callback)
{
    objectTransformChangedCallback_ = std::move(callback);
}

void ViewportWorkspaceWidget::setBeforeSceneMutationCallback(std::function<void()> callback)
{
    beforeSceneMutationCallback_ = std::move(callback);
    for (ViewportWidget* viewport : allViewports()) {
        viewport->setBeforeSceneMutationCallback(beforeSceneMutationCallback_);
    }
}

void ViewportWorkspaceWidget::setAutoKeyEnabled(bool enabled)
{
    autoKeyEnabled_ = enabled;
    for (ViewportWidget* viewport : allViewports()) {
        viewport->setAutoKeyEnabled(enabled);
    }
}

bool ViewportWorkspaceWidget::autoKeyEnabled() const
{
    return autoKeyEnabled_;
}

bool ViewportWorkspaceWidget::importFbx(const QString& filePath)
{
    if (!activeViewport_->importFbx(filePath)) {
        lastImportSucceeded_ = false;
        lastImportMessage_ = activeViewport_->lastImportMessage();
        return false;
    }

    syncSceneToAllViewports(activeViewport_);
    frameAllViewports();
    selectedObjectId_ = activeViewport_->selectedObject();
    syncSelectionToAllViewports();
    lastImportSucceeded_ = true;
    lastImportMessage_ = activeViewport_->lastImportMessage();
    return true;
}

void ViewportWorkspaceWidget::clearScene()
{
    for (ViewportWidget* viewport : allViewports()) {
        viewport->clearScene();
    }

    selectedObjectId_ = 0;
}

void ViewportWorkspaceWidget::replaceScene(const Scene& scene)
{
    for (ViewportWidget* viewport : allViewports()) {
        viewport->replaceScene(scene);
    }

    selectedObjectId_ = 0;
}

void ViewportWorkspaceWidget::optimizeSceneStorage()
{
    activeViewport_->optimizeSceneStorage();
    syncSceneToAllViewports(activeViewport_);
    syncSelectionToAllViewports();
}

QString ViewportWorkspaceWidget::lastImportMessage() const
{
    return lastImportMessage_;
}

bool ViewportWorkspaceWidget::lastImportSucceeded() const
{
    return lastImportSucceeded_;
}

bool ViewportWorkspaceWidget::setObjectLocalTransform(SceneObject::Id objectId, const Transform& transform)
{
    if (!activeViewport_->setObjectLocalTransform(objectId, transform)) {
        return false;
    }

    selectedObjectId_ = objectId;
    syncSceneToAllViewports(activeViewport_);
    syncSelectionToAllViewports();
    return true;
}

bool ViewportWorkspaceWidget::setObjectVisibility(SceneObject::Id objectId, bool visible)
{
    if (!activeViewport_->setObjectVisibility(objectId, visible)) {
        return false;
    }

    syncSceneToAllViewports(activeViewport_);
    syncSelectionToAllViewports();
    return true;
}

int ViewportWorkspaceWidget::currentFrame() const
{
    return activeViewport_->currentFrame();
}

void ViewportWorkspaceWidget::setCurrentFrame(int frame)
{
    for (ViewportWidget* viewport : allViewports()) {
        viewport->setCurrentFrame(frame);
    }
    syncSelectionToAllViewports();
}

int ViewportWorkspaceWidget::nextObjectKeyframe(SceneObject::Id objectId, int frame) const
{
    return activeViewport_->scene().nextObjectKeyframe(objectId, frame);
}

int ViewportWorkspaceWidget::previousObjectKeyframe(SceneObject::Id objectId, int frame) const
{
    return activeViewport_->scene().previousObjectKeyframe(objectId, frame);
}

bool ViewportWorkspaceWidget::setObjectKeyframe(SceneObject::Id objectId, int frame)
{
    if (!activeViewport_->setObjectKeyframe(objectId, frame)) {
        return false;
    }

    syncSceneToAllViewports(activeViewport_);
    selectedObjectId_ = objectId;
    syncSelectionToAllViewports();
    return true;
}

bool ViewportWorkspaceWidget::removeObjectKeyframe(SceneObject::Id objectId, int frame)
{
    if (!activeViewport_->removeObjectKeyframe(objectId, frame)) {
        return false;
    }

    syncSceneToAllViewports(activeViewport_);
    selectedObjectId_ = objectId;
    syncSelectionToAllViewports();
    return true;
}

bool ViewportWorkspaceWidget::setJointOrientation(SceneObject::Id objectId, const QQuaternion& orientation)
{
    if (!activeViewport_->setJointOrientation(objectId, orientation)) {
        return false;
    }

    syncSceneToAllViewports(activeViewport_);
    selectedObjectId_ = objectId;
    syncSelectionToAllViewports();
    return true;
}

bool ViewportWorkspaceWidget::resetJointOrientation(SceneObject::Id objectId)
{
    if (!activeViewport_->resetJointOrientation(objectId)) {
        return false;
    }

    syncSceneToAllViewports(activeViewport_);
    selectedObjectId_ = objectId;
    syncSelectionToAllViewports();
    return true;
}

bool ViewportWorkspaceWidget::alignJointOrientationToChild(SceneObject::Id objectId)
{
    if (!activeViewport_->alignJointOrientationToChild(objectId)) {
        return false;
    }

    syncSceneToAllViewports(activeViewport_);
    selectedObjectId_ = objectId;
    syncSelectionToAllViewports();
    return true;
}

bool ViewportWorkspaceWidget::captureBindPose(SceneObject::Id objectId, bool recursive)
{
    if (!activeViewport_->captureBindPose(objectId, recursive)) {
        return false;
    }

    syncSceneToAllViewports(activeViewport_);
    selectedObjectId_ = objectId;
    syncSelectionToAllViewports();
    return true;
}

SceneObject::Id ViewportWorkspaceWidget::createPrimitive(PrimitiveMeshFactory::Type type, const QString& name)
{
    const SceneObject::Id objectId = activeViewport_->createPrimitive(type, name);
    if (objectId == 0) {
        return 0;
    }

    selectedObjectId_ = objectId;
    syncSceneToAllViewports(activeViewport_);
    syncSelectionToAllViewports();
    return objectId;
}

SceneObject::Id ViewportWorkspaceWidget::createJoint(const QString& name, SceneObject::Id parentId)
{
    const SceneObject::Id objectId = activeViewport_->createJoint(name, parentId);
    if (objectId == 0) {
        return 0;
    }

    selectedObjectId_ = objectId;
    syncSceneToAllViewports(activeViewport_);
    syncSelectionToAllViewports();
    return objectId;
}

bool ViewportWorkspaceWidget::quadViewEnabled() const
{
    return quadViewEnabled_;
}

void ViewportWorkspaceWidget::toggleQuadView()
{
    quadViewEnabled_ = !quadViewEnabled_;
    rebuildViewportLayout();
    refreshViewportMenuStates();
}

ViewportWorkspaceWidget::CameraViewPreset ViewportWorkspaceWidget::activeCameraViewPreset() const
{
    return activeViewport_ == nullptr
        ? CameraViewPreset::Perspective
        : activeViewport_->cameraViewPreset();
}

bool ViewportWorkspaceWidget::isCameraViewVisible(CameraViewPreset preset) const
{
    const ViewPane* pane = paneForPreset(preset);
    return pane != nullptr && pane->frame != nullptr && pane->frame->isVisible();
}

bool ViewportWorkspaceWidget::eventFilter(QObject* watched, QEvent* event)
{
    auto* viewport = dynamic_cast<ViewportWidget*>(watched);
    if (viewport != nullptr && event != nullptr) {
        if (event->type() == QEvent::KeyPress) {
            auto* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Space && !keyEvent->isAutoRepeat()) {
                setActiveViewport(viewport);
                toggleQuadView();
                keyEvent->accept();
                return true;
            }
        }

        if (event->type() == QEvent::MouseButtonPress) {
            setActiveViewport(viewport);
            viewport->setFocus(Qt::MouseFocusReason);
        }
    }

    return QWidget::eventFilter(watched, event);
}

void ViewportWorkspaceWidget::configurePane(ViewPane& pane, const QString& viewportObjectName, CameraViewPreset preset)
{
    pane.frame = new QFrame(layoutHost_);
    pane.frame->setFrameStyle(QFrame::NoFrame);
    pane.frame->setStyleSheet(inactiveFrameStyle());

    QVBoxLayout* frameLayout = new QVBoxLayout(pane.frame);
    frameLayout->setContentsMargins(0, 0, 0, 0);
    frameLayout->setSpacing(0);

    pane.menuBar = new QMenuBar(pane.frame);
    pane.menuBar->setNativeMenuBar(false);
    pane.menuBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    pane.menuBar->setStyleSheet(viewportMenuBarStyle());
    frameLayout->addWidget(pane.menuBar);

    pane.viewport = new ViewportWidget(pane.frame);
    pane.viewport->setObjectName(viewportObjectName);
    pane.viewport->setCameraViewPreset(preset);
    pane.viewport->setTransformMode(transformMode_);
    pane.viewport->setAxisOrientation(axisOrientation_);
    pane.viewport->setAutoKeyEnabled(autoKeyEnabled_);
    pane.viewport->installEventFilter(this);
    frameLayout->addWidget(pane.viewport);

    QMenu* viewMenu = pane.menuBar->addMenu("View");
    QAction* frameSceneAction = viewMenu->addAction("Frame Scene");
    QObject::connect(frameSceneAction, &QAction::triggered, this, [this, viewport = pane.viewport]() {
        setActiveViewport(viewport);
        viewport->frameScene();
    });

    QAction* frameSelectedAction = viewMenu->addAction("Frame Selected");
    QObject::connect(frameSelectedAction, &QAction::triggered, this, [this, viewport = pane.viewport]() {
        if (selectedObjectId_ == 0) {
            return;
        }
        setActiveViewport(viewport);
        viewport->frameObject(selectedObjectId_);
    });

    QAction* resetCameraAction = viewMenu->addAction("Reset Camera");
    QObject::connect(resetCameraAction, &QAction::triggered, this, [this, viewport = pane.viewport]() {
        setActiveViewport(viewport);
        viewport->resetCamera();
    });

    QMenu* cameraMenu = viewMenu->addMenu("Cameras");
    const QList<QPair<QString, CameraViewPreset>> cameraEntries = {
        { "Perspective", CameraViewPreset::Perspective },
        { "Front", CameraViewPreset::Front },
        { "Back", CameraViewPreset::Back },
        { "Left", CameraViewPreset::Left },
        { "Right", CameraViewPreset::Right },
        { "Top", CameraViewPreset::Top },
        { "Bottom", CameraViewPreset::Bottom }
    };
    for (const auto& entry : cameraEntries) {
        QAction* action = cameraMenu->addAction(entry.first);
        QObject::connect(action, &QAction::triggered, this, [this, viewport = pane.viewport, preset = entry.second]() {
            setActiveViewport(viewport);
            viewport->setCameraViewPreset(preset);
        });
    }

    QMenu* shadingMenu = pane.menuBar->addMenu("Shading");
    QAction* wireframeAction = shadingMenu->addAction("Wireframe");
    wireframeAction->setCheckable(true);
    wireframeActions_.append(wireframeAction);
    QObject::connect(wireframeAction, &QAction::toggled, this, [this](bool enabled) {
        setWireframeEnabled(enabled);
    });

    QMenu* lightingMenu = pane.menuBar->addMenu("Lighting");
    QAction* axisAction = lightingMenu->addAction("Show Axis");
    axisAction->setCheckable(true);
    axisActions_.append(axisAction);
    QObject::connect(axisAction, &QAction::toggled, this, [this](bool enabled) {
        setAxisVisible(enabled);
    });

    QMenu* showMenu = pane.menuBar->addMenu("Show");
    QAction* backfaceAction = showMenu->addAction("Backface Culling");
    backfaceAction->setCheckable(true);
    backfaceActions_.append(backfaceAction);
    QObject::connect(backfaceAction, &QAction::toggled, this, [this](bool enabled) {
        setBackfaceCullingEnabled(enabled);
    });
    QAction* selectionOutlineAction = showMenu->addAction("Selection Outline");
    selectionOutlineAction->setCheckable(true);
    selectionOutlineActions_.append(selectionOutlineAction);
    QObject::connect(selectionOutlineAction, &QAction::toggled, this, [this](bool enabled) {
        setSelectionOutlineVisible(enabled);
    });

    pane.menuBar->addMenu("Renderer");

    QMenu* panelsMenu = pane.menuBar->addMenu("Panels");
    QAction* singleViewAction = panelsMenu->addAction("Single View");
    QObject::connect(singleViewAction, &QAction::triggered, this, [this, viewport = pane.viewport]() {
        setActiveViewport(viewport);
        if (quadViewEnabled_) {
            toggleQuadView();
        }
    });

    QAction* quadViewAction = panelsMenu->addAction("Quad View");
    quadViewAction->setCheckable(true);
    quadViewActions_.append(quadViewAction);
    QObject::connect(quadViewAction, &QAction::triggered, this, [this, viewport = pane.viewport]() {
        setActiveViewport(viewport);
        if (!quadViewEnabled_) {
            toggleQuadView();
        }
    });

    panelsMenu->addSeparator();
    const QList<QPair<QString, CameraViewPreset>> panelViewEntries = {
        { "Perspective View", CameraViewPreset::Perspective },
        { "Top View", CameraViewPreset::Top },
        { "Front View", CameraViewPreset::Front },
        { "Side View", CameraViewPreset::Right }
    };
    for (const auto& entry : panelViewEntries) {
        QAction* action = panelsMenu->addAction(entry.first);
        QObject::connect(action, &QAction::triggered, this, [this, viewport = pane.viewport, preset = entry.second]() {
            setActiveViewport(viewport);
            viewport->setCameraViewPreset(preset);
            if (quadViewEnabled_) {
                toggleQuadView();
            }
        });
    }

    pane.viewport->setSelectionChangedCallback([this, viewport = pane.viewport](SceneObject::Id objectId) {
        setActiveViewport(viewport);
        selectedObjectId_ = objectId;
        syncSelectionToAllViewports();
        if (selectionChangedCallback_) {
            selectionChangedCallback_(objectId);
        }
    });

    pane.viewport->setObjectTransformChangedCallback([this, viewport = pane.viewport](SceneObject::Id objectId) {
        setActiveViewport(viewport);
        selectedObjectId_ = objectId;
        syncSceneToAllViewports(viewport);
        syncSelectionToAllViewports();
        if (objectTransformChangedCallback_) {
            objectTransformChangedCallback_(objectId);
        }
    });

    refreshViewportMenuStates();
}

void ViewportWorkspaceWidget::rebuildViewportLayout()
{
    while (QLayoutItem* item = gridLayout_->takeAt(0)) {
        delete item;
    }

    for (ViewPane* pane : QList<ViewPane*> { &perspectivePane_, &topPane_, &frontPane_, &sidePane_ }) {
        pane->frame->hide();
    }

    if (quadViewEnabled_) {
        gridLayout_->addWidget(topPane_.frame, 0, 0);
        gridLayout_->addWidget(perspectivePane_.frame, 0, 1);
        gridLayout_->addWidget(frontPane_.frame, 1, 0);
        gridLayout_->addWidget(sidePane_.frame, 1, 1);
        topPane_.frame->show();
        perspectivePane_.frame->show();
        frontPane_.frame->show();
        sidePane_.frame->show();
    } else {
        ViewPane* activePane = paneForViewport(activeViewport_);
        if (activePane != nullptr) {
            gridLayout_->addWidget(activePane->frame, 0, 0, 2, 2);
            activePane->frame->show();
        }
    }

    gridLayout_->setRowStretch(0, 1);
    gridLayout_->setRowStretch(1, 1);
    gridLayout_->setColumnStretch(0, 1);
    gridLayout_->setColumnStretch(1, 1);

    refreshActiveViewportFrameStyles();
}

void ViewportWorkspaceWidget::setActiveViewport(ViewportWidget* viewport)
{
    if (viewport == nullptr || viewport == activeViewport_) {
        return;
    }

    activeViewport_ = viewport;
    refreshActiveViewportFrameStyles();

    if (!quadViewEnabled_) {
        rebuildViewportLayout();
    }
}

void ViewportWorkspaceWidget::refreshActiveViewportFrameStyles()
{
    for (ViewPane* pane : QList<ViewPane*> { &perspectivePane_, &topPane_, &frontPane_, &sidePane_ }) {
        pane->frame->setStyleSheet(pane->viewport == activeViewport_ ? activeFrameStyle() : inactiveFrameStyle());
    }
}

void ViewportWorkspaceWidget::refreshViewportMenuStates()
{
    for (QAction* action : wireframeActions_) {
        if (action->isChecked() != wireframeEnabled_) {
            action->setChecked(wireframeEnabled_);
        }
    }

    for (QAction* action : axisActions_) {
        if (action->isChecked() != axisVisible_) {
            action->setChecked(axisVisible_);
        }
    }

    for (QAction* action : backfaceActions_) {
        if (action->isChecked() != backfaceCullingEnabled_) {
            action->setChecked(backfaceCullingEnabled_);
        }
    }

    for (QAction* action : selectionOutlineActions_) {
        if (action->isChecked() != selectionOutlineVisible_) {
            action->setChecked(selectionOutlineVisible_);
        }
    }

    for (QAction* action : quadViewActions_) {
        if (action->isChecked() != quadViewEnabled_) {
            action->setChecked(quadViewEnabled_);
        }
    }
}

void ViewportWorkspaceWidget::syncSceneToAllViewports(ViewportWidget* sourceViewport)
{
    const Scene sceneCopy = sourceViewport->scene();
    for (ViewportWidget* viewport : allViewports()) {
        if (viewport == sourceViewport) {
            continue;
        }

        viewport->replaceScene(sceneCopy);
        viewport->setTransformMode(transformMode_);
        viewport->setAxisOrientation(axisOrientation_);
        viewport->setAutoKeyEnabled(autoKeyEnabled_);
    }
}

void ViewportWorkspaceWidget::syncSelectionToAllViewports()
{
    for (ViewportWidget* viewport : allViewports()) {
        viewport->setSelectedObject(selectedObjectId_);
    }
}

void ViewportWorkspaceWidget::frameAllViewports()
{
    for (ViewportWidget* viewport : allViewports()) {
        viewport->frameScene();
    }
}

QList<ViewportWidget*> ViewportWorkspaceWidget::allViewports() const
{
    QList<ViewportWidget*> viewports;
    for (ViewportWidget* viewport : {
             perspectivePane_.viewport,
             topPane_.viewport,
             frontPane_.viewport,
             sidePane_.viewport
         }) {
        if (viewport != nullptr) {
            viewports.append(viewport);
        }
    }

    return viewports;
}

ViewportWorkspaceWidget::ViewPane* ViewportWorkspaceWidget::paneForViewport(ViewportWidget* viewport)
{
    for (ViewPane* pane : QList<ViewPane*> { &perspectivePane_, &topPane_, &frontPane_, &sidePane_ }) {
        if (pane->viewport == viewport) {
            return pane;
        }
    }

    return nullptr;
}

const ViewportWorkspaceWidget::ViewPane* ViewportWorkspaceWidget::paneForViewport(ViewportWidget* viewport) const
{
    for (const ViewPane* pane : QList<const ViewPane*> { &perspectivePane_, &topPane_, &frontPane_, &sidePane_ }) {
        if (pane->viewport == viewport) {
            return pane;
        }
    }

    return nullptr;
}

ViewportWorkspaceWidget::ViewPane* ViewportWorkspaceWidget::paneForPreset(CameraViewPreset preset)
{
    return const_cast<ViewPane*>(std::as_const(*this).paneForPreset(preset));
}

const ViewportWorkspaceWidget::ViewPane* ViewportWorkspaceWidget::paneForPreset(CameraViewPreset preset) const
{
    switch (preset) {
    case CameraViewPreset::Perspective:
        return &perspectivePane_;
    case CameraViewPreset::Top:
        return &topPane_;
    case CameraViewPreset::Front:
        return &frontPane_;
    case CameraViewPreset::Right:
        return &sidePane_;
    case CameraViewPreset::Back:
    case CameraViewPreset::Left:
    case CameraViewPreset::Bottom:
        break;
    }

    return nullptr;
}
