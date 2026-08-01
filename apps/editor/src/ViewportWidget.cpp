#include "ViewportWidget.h"

#include <QDebug>
#include <QLineF>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QPainter>
#include <QVector2D>
#include <QtMath>

#include <limits>

#include "core/logging/LogCategories.h"
#include "rendering/scene/ViewportRenderSceneAdapter.h"
#include "viewport/interaction/ViewportInteractionMath.h"
#include "viewport/interaction/gizmo/RotateGizmoInteraction.h"
#include "viewport/interaction/gizmo/ScaleGizmoInteraction.h"
#include "viewport/interaction/gizmo/TranslateGizmoInteraction.h"
#include "viewport/gizmo/drag/RotateGizmoDrag.h"
#include "viewport/gizmo/drag/ScaleGizmoDrag.h"
#include "viewport/gizmo/drag/TranslateGizmoDrag.h"
#include "viewport/runtime/ViewportRenderSync.h"

namespace
{
QVector3D axisVector(int axis)
{
    switch (axis) {
    case 0:
        return QVector3D(1.0f, 0.0f, 0.0f);
    case 1:
        return QVector3D(0.0f, 1.0f, 0.0f);
    case 2:
        return QVector3D(0.0f, 0.0f, 1.0f);
    default:
        return QVector3D();
    }
}
}

ViewportWidget::ViewportWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
    setUpdateBehavior(QOpenGLWidget::PartialUpdate);
    setFocusPolicy(Qt::StrongFocus);
    camera_.setZoomSensitivity(0.9f);
    renderOptions_.showAxis = true;
    renderOptions_.wireframe = false;
    renderOptions_.backfaceCulling = false;
}

const Scene& ViewportWidget::scene() const
{
    return scene_;
}

void ViewportWidget::requestRender()
{
    update();
}

void ViewportWidget::resetCamera()
{
    camera_.reset();
    syncRendererSelection();
    requestRender();
}

void ViewportWidget::setCameraViewPreset(CameraViewPreset preset)
{
    EditorCamera::ViewPreset cameraPreset = EditorCamera::ViewPreset::Perspective;
    switch (preset) {
    case CameraViewPreset::Front:
        cameraPreset = EditorCamera::ViewPreset::Front;
        break;
    case CameraViewPreset::Back:
        cameraPreset = EditorCamera::ViewPreset::Back;
        break;
    case CameraViewPreset::Left:
        cameraPreset = EditorCamera::ViewPreset::Left;
        break;
    case CameraViewPreset::Right:
        cameraPreset = EditorCamera::ViewPreset::Right;
        break;
    case CameraViewPreset::Top:
        cameraPreset = EditorCamera::ViewPreset::Top;
        break;
    case CameraViewPreset::Bottom:
        cameraPreset = EditorCamera::ViewPreset::Bottom;
        break;
    case CameraViewPreset::Perspective:
        break;
    }

    camera_.setViewPreset(cameraPreset);
    syncRendererSelection();
    requestRender();
}

ViewportWidget::CameraViewPreset ViewportWidget::cameraViewPreset() const
{
    switch (camera_.viewPreset()) {
    case EditorCamera::ViewPreset::Front:
        return CameraViewPreset::Front;
    case EditorCamera::ViewPreset::Back:
        return CameraViewPreset::Back;
    case EditorCamera::ViewPreset::Left:
        return CameraViewPreset::Left;
    case EditorCamera::ViewPreset::Right:
        return CameraViewPreset::Right;
    case EditorCamera::ViewPreset::Top:
        return CameraViewPreset::Top;
    case EditorCamera::ViewPreset::Bottom:
        return CameraViewPreset::Bottom;
    case EditorCamera::ViewPreset::Perspective:
        break;
    }

    return CameraViewPreset::Perspective;
}

QString ViewportWidget::cameraViewLabel() const
{
    return cameraViewLabelText();
}

void ViewportWidget::frameScene()
{
    if (!scene_.isEmpty() && scene_.sceneBounds().isValid()) {
        camera_.frameBounds(scene_.sceneBounds().center(), qMax(1.0f, scene_.sceneBounds().radius()));
    } else {
        camera_.frameScene();
    }
    syncRendererSelection();
    requestRender();
}

void ViewportWidget::frameObject(SceneObject::Id objectId)
{
    const SceneObject* object = scene_.findObject(objectId);
    if (object != nullptr && object->worldBounds().isValid()) {
        camera_.frameBounds(object->worldBounds().center(), qMax(1.0f, object->worldBounds().radius()));
    } else {
        frameScene();
        return;
    }

    syncRendererSelection();
    requestRender();
}

void ViewportWidget::setSelectedObject(SceneObject::Id objectId)
{
    selectedObjectId_ = scene_.contains(objectId) ? objectId : 0;
    syncRendererSelection();
    requestRender();
}

SceneObject::Id ViewportWidget::selectedObject() const
{
    return selectedObjectId_;
}

void ViewportWidget::setWireframeEnabled(bool enabled)
{
    renderOptions_.wireframe = enabled;
    requestRender();
}

void ViewportWidget::setAxisVisible(bool visible)
{
    renderOptions_.showAxis = visible;
    requestRender();
}

void ViewportWidget::setBackfaceCullingEnabled(bool enabled)
{
    renderOptions_.backfaceCulling = enabled;
    requestRender();
}

void ViewportWidget::setSelectionOutlineVisible(bool visible)
{
    renderOptions_.showSelectionOutline = visible;
    requestRender();
}

void ViewportWidget::setTransformMode(TransformMode mode)
{
    transformMode_ = mode;
    syncRendererSelection();
    requestRender();
}

ViewportWidget::TransformMode ViewportWidget::transformMode() const
{
    return transformMode_;
}

void ViewportWidget::setAxisOrientation(AxisOrientation orientation)
{
    axisOrientation_ = orientation;
    syncRendererSelection();
    requestRender();
}

ViewportWidget::AxisOrientation ViewportWidget::axisOrientation() const
{
    return axisOrientation_;
}

void ViewportWidget::setSelectionChangedCallback(std::function<void(SceneObject::Id)> callback)
{
    selectionChangedCallback_ = std::move(callback);
}

void ViewportWidget::setObjectTransformChangedCallback(std::function<void(SceneObject::Id)> callback)
{
    objectTransformChangedCallback_ = std::move(callback);
}

void ViewportWidget::setBeforeSceneMutationCallback(std::function<void()> callback)
{
    beforeSceneMutationCallback_ = std::move(callback);
}

void ViewportWidget::setAutoKeyEnabled(bool enabled)
{
    autoKeyEnabled_ = enabled;
}

bool ViewportWidget::autoKeyEnabled() const
{
    return autoKeyEnabled_;
}

bool ViewportWidget::importFbx(const QString& filePath)
{
    const FbxImportResult result = importer_.importFile(filePath);
    if (!result.success) {
        lastImportSucceeded_ = false;
        lastImportMessage_ = result.errorMessage;
        qCWarning(logFbx) << "import failed:" << filePath << result.errorMessage;
        return false;
    }

    EditorViewportSceneController::appendImportedScene(sceneControllerContext(), result.scene, !scene_.isEmpty());
    frameScene();

    lastImportSucceeded_ = true;
    lastImportMessage_ = QString("%1 | scene objects=%2")
                             .arg(result.infoMessage)
                             .arg(scene_.allObjectIds().size());
    qCInfo(logFbx) << "import ok:" << result.infoMessage;
    return true;
}

void ViewportWidget::clearScene()
{
    EditorViewportSceneController::clearScene(sceneControllerContext());
}

void ViewportWidget::replaceScene(const Scene& scene)
{
    EditorViewportSceneController::replaceScene(sceneControllerContext(), scene);
}

void ViewportWidget::optimizeSceneStorage()
{
    EditorViewportSceneController::optimizeSceneStorage(sceneControllerContext());
}

QString ViewportWidget::lastImportMessage() const
{
    return lastImportMessage_;
}

bool ViewportWidget::lastImportSucceeded() const
{
    return lastImportSucceeded_;
}

bool ViewportWidget::setObjectLocalTransform(SceneObject::Id objectId, const Transform& transform)
{
    return EditorViewportSceneController::setObjectLocalTransform(sceneControllerContext(), objectId, transform);
}

bool ViewportWidget::setObjectVisibility(SceneObject::Id objectId, bool visible)
{
    return EditorViewportSceneController::setObjectVisibility(sceneControllerContext(), objectId, visible);
}

int ViewportWidget::currentFrame() const
{
    return scene_.currentFrame();
}

void ViewportWidget::setCurrentFrame(int frame)
{
    EditorViewportSceneController::setCurrentFrame(sceneControllerContext(), frame);
}

bool ViewportWidget::setObjectKeyframe(SceneObject::Id objectId, int frame)
{
    return EditorViewportSceneController::setObjectKeyframe(sceneControllerContext(), objectId, frame);
}

bool ViewportWidget::removeObjectKeyframe(SceneObject::Id objectId, int frame)
{
    return EditorViewportSceneController::removeObjectKeyframe(sceneControllerContext(), objectId, frame);
}

bool ViewportWidget::setJointOrientation(SceneObject::Id objectId, const QQuaternion& orientation)
{
    return EditorViewportSceneController::setJointOrientation(sceneControllerContext(), objectId, orientation);
}

bool ViewportWidget::resetJointOrientation(SceneObject::Id objectId)
{
    return EditorViewportSceneController::resetJointOrientation(sceneControllerContext(), objectId);
}

bool ViewportWidget::alignJointOrientationToChild(SceneObject::Id objectId)
{
    return EditorViewportSceneController::alignJointOrientationToChild(sceneControllerContext(), objectId);
}

bool ViewportWidget::captureBindPose(SceneObject::Id objectId, bool recursive)
{
    return EditorViewportSceneController::captureBindPose(sceneControllerContext(), objectId, recursive);
}

SceneObject::Id ViewportWidget::createPrimitive(PrimitiveMeshFactory::Type type, const QString& name)
{
    return EditorViewportSceneController::createPrimitive(sceneControllerContext(), type, name);
}

SceneObject::Id ViewportWidget::createJoint(const QString& name, SceneObject::Id parentId)
{
    return EditorViewportSceneController::createJoint(sceneControllerContext(), name, parentId);
}

void ViewportWidget::initializeGL()
{
    initializeOpenGLFunctions();

    QOpenGLContext* context = QOpenGLContext::currentContext();
    if (context == nullptr) {
        viewportInitialized_ = false;
        viewportInitFailureReason_ = "No current OpenGL context.";
        qCCritical(logViewport) << "init failed:" << viewportInitFailureReason_;
        return;
    }

    qCInfo(logViewport) << "startup:"
            << "vendor=" << reinterpret_cast<const char*>(glGetString(GL_VENDOR))
            << "renderer=" << reinterpret_cast<const char*>(glGetString(GL_RENDERER))
            << "version=" << reinterpret_cast<const char*>(glGetString(GL_VERSION));

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearColor(0.12f, 0.14f, 0.17f, 1.0f);

    viewportInitialized_ = renderer_.initialize(this);
    if (!viewportInitialized_) {
        viewportInitFailureReason_ = "Viewport renderer initialization failed.";
        qCCritical(logViewport) << "init failed:" << viewportInitFailureReason_;
        return;
    }

    viewportInitFailureReason_.clear();
    ViewportRenderSync::syncScene(renderer_, scene_);
    syncRendererSelection();
    qCInfo(logViewport) << "init ok";
    requestRender();
}

void ViewportWidget::resizeGL(int width, int height)
{
    viewportWidth_ = width;
    viewportHeight_ = height;

    glViewport(0, 0, width, height);
    camera_.setViewportSize(width, height);
    renderer_.resize(width, height);
    syncRendererSelection();
    requestRender();
}

void ViewportWidget::paintGL()
{
    if (!viewportInitialized_) {
        if (!viewportInitFailureReason_.isEmpty()) {
            qCWarning(logViewport) << "paint skipped:" << viewportInitFailureReason_;
        }
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderer_.render(camera_, renderOptions_);

    QPainter painter(this);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setPen(QColor(235, 235, 235));
    painter.setFont(QFont("Segoe UI", 9, QFont::DemiBold));

    const QString label = cameraViewLabelText();
    const QRect labelRect(10, height() - 28, 120, 20);
    painter.drawText(labelRect, Qt::AlignLeft | Qt::AlignVCenter, label);
}

void ViewportWidget::mousePressEvent(QMouseEvent* event)
{
    setFocus(Qt::MouseFocusReason);
    handleHotkeys(event);
    lastMousePosition_ = event->pos();
    activeButtons_ = event->buttons();

    if (!(event->modifiers() & Qt::AltModifier) && event->button() == Qt::LeftButton && selectedObjectId_ != 0) {
        const GizmoHandle handle = pickGizmoHandleAtScreenPos(event->pos());
        if (handle != GizmoHandle::None) {
            dragState_.active = true;
            dragState_.startMousePosition = event->pos();
            dragState_.startScreenVector = QPointF();
            dragState_.handle = handle;
            dragState_.gizmoOrigin = gizmoOrigin();
            dragState_.gizmoSize = gizmoSize();

            if (const SceneObject* object = scene_.findObject(selectedObjectId_)) {
                dragState_.startTransform = object->localTransform();
            }
            dragState_.previewStartWorldMatrix = scene_.worldTransform(selectedObjectId_);

            if (transformMode_ == TransformMode::Translate && gizmoHandleUsesPlaneDrag(handle)) {
                dragState_.dragPlaneOrigin = dragState_.gizmoOrigin;
                dragState_.dragPlaneNormal = gizmoPlaneNormalWorld(handle);

                QVector3D rayOrigin;
                QVector3D rayDirection;
                ViewportInteractionMath::screenPosToWorldRay(
                    camera_.projectionMatrix(),
                    camera_.viewMatrix(),
                    viewportWidth_,
                    viewportHeight_,
                    event->pos(),
                    rayOrigin,
                    rayDirection);

                QVector3D hitPoint;
                if (ViewportInteractionMath::intersectRayPlane(
                        rayOrigin,
                        rayDirection,
                        dragState_.dragPlaneOrigin,
                        dragState_.dragPlaneNormal,
                        &hitPoint)) {
                    dragState_.dragStartWorldPoint = hitPoint;
                } else {
                    dragState_.dragStartWorldPoint = dragState_.gizmoOrigin;
                }
            } else if (transformMode_ == TransformMode::Rotate) {
                dragState_.dragPlaneOrigin = dragState_.gizmoOrigin;
                dragState_.dragPlaneNormal = handle == GizmoHandle::ViewPlane
                    ? -camera_.forwardDirection().normalized()
                    : gizmoAxisDirectionWorld(handle);

                QVector3D rayOrigin;
                QVector3D rayDirection;
                ViewportInteractionMath::screenPosToWorldRay(
                    camera_.projectionMatrix(),
                    camera_.viewMatrix(),
                    viewportWidth_,
                    viewportHeight_,
                    event->pos(),
                    rayOrigin,
                    rayDirection);

                QVector3D hitPoint;
                if (ViewportInteractionMath::intersectRayPlane(
                        rayOrigin,
                        rayDirection,
                        dragState_.dragPlaneOrigin,
                        dragState_.dragPlaneNormal,
                        &hitPoint)) {
                    dragState_.dragStartWorldPoint = hitPoint;
                } else {
                    dragState_.dragStartWorldPoint = dragState_.gizmoOrigin;
                }

                const QPointF originScreen = ViewportInteractionMath::projectWorldToScreen(
                    camera_.projectionMatrix(),
                    camera_.viewMatrix(),
                    dragState_.gizmoOrigin,
                    viewportWidth_,
                    viewportHeight_);
                dragState_.startScreenVector = QPointF(event->pos()) - originScreen;
            }

            beginInteractivePreview();
            syncRendererSelection();
            requestRender();
            event->accept();
            return;
        }
    }

    if (!(event->modifiers() & Qt::AltModifier) && event->button() == Qt::LeftButton) {
        updateSelectedObject(pickObjectAtScreenPos(event->pos()));
    }

    event->accept();
}

void ViewportWidget::mouseMoveEvent(QMouseEvent* event)
{
    const QPoint delta = event->pos() - lastMousePosition_;
    lastMousePosition_ = event->pos();

    if (dragState_.active && (event->buttons() & Qt::LeftButton)) {
        applyDrag(event->pos());
        event->accept();
        return;
    }

    if ((event->modifiers() & Qt::AltModifier) && (event->buttons() & Qt::LeftButton)) {
        camera_.orbit(delta.x() * -0.35f, delta.y() * -0.35f);
        syncRendererSelection();
        requestRender();
    } else if ((event->modifiers() & Qt::AltModifier) && (event->buttons() & Qt::MiddleButton)) {
        camera_.pan(static_cast<float>(delta.x()), static_cast<float>(delta.y()));
        syncRendererSelection();
        requestRender();
    } else if ((event->modifiers() & Qt::AltModifier) && (event->buttons() & Qt::RightButton)) {
        camera_.zoom(static_cast<float>(-delta.y()) * 0.05f);
        syncRendererSelection();
        requestRender();
    }

    event->accept();
}

void ViewportWidget::mouseReleaseEvent(QMouseEvent* event)
{
    activeButtons_ = event->buttons();
    if (dragState_.active && event->button() == Qt::LeftButton) {
        endInteractivePreview();
        syncSceneToRenderer();
        if (objectTransformChangedCallback_) {
            objectTransformChangedCallback_(selectedObjectId_);
        }
        dragRenderSyncTimer_.invalidate();
        dragState_ = {};
        syncRendererSelection();
        requestRender();
    }
    event->accept();
}

void ViewportWidget::wheelEvent(QWheelEvent* event)
{
    camera_.zoom(static_cast<float>(event->angleDelta().y()) / 120.0f);
    syncRendererSelection();
    requestRender();
    event->accept();
}

void ViewportWidget::syncRendererSelection()
{
    if (!viewportInitialized_ || !isValid() || context() == nullptr) {
        return;
    }

    makeCurrent();
    applyRendererSelectionState();
    doneCurrent();
}

void ViewportWidget::applyRendererSelectionState()
{
    const SceneObject* object = scene_.findObject(selectedObjectId_);
    if (object != nullptr) {
        ViewportRenderSelectionState selectionState;
        selectionState.hasSelection = true;
        ViewportRenderer::GizmoMode gizmoMode = ViewportRenderer::GizmoMode::Translate;
        if (transformMode_ == TransformMode::Rotate) {
            gizmoMode = ViewportRenderer::GizmoMode::Rotate;
        } else if (transformMode_ == TransformMode::Scale) {
            gizmoMode = ViewportRenderer::GizmoMode::Scale;
        }

        selectionState.selectedBounds = object->worldBounds();
        selectionState.gizmoOrigin = gizmoOrigin();
        selectionState.gizmoSize = gizmoSize();
        selectionState.gizmoMode = gizmoMode;
        selectionState.gizmoAxes = gizmoAxesWorld();
        selectionState.cameraForward = camera_.forwardDirection();
        selectionState.activeAxis = dragState_.active ? static_cast<int>(dragState_.handle) : -1;
        ViewportRenderSync::applySelectionState(renderer_, selectionState);
    } else {
        ViewportRenderSync::applySelectionState(renderer_, {});
    }
}

void ViewportWidget::syncSceneToRenderer()
{
    if (!viewportInitialized_ || !isValid() || context() == nullptr) {
        return;
    }

    makeCurrent();
    ViewportRenderSync::syncScene(renderer_, scene_);
    applyRendererSelectionState();
    doneCurrent();
}

void ViewportWidget::syncSceneToRendererExcludingSelection()
{
    if (!viewportInitialized_ || !isValid() || context() == nullptr) {
        return;
    }

    makeCurrent();
    renderer_.syncScene(ViewportRenderSceneAdapter::buildSceneDataExcludingSubtree(scene_, selectedObjectId_));
    applyRendererSelectionState();
    doneCurrent();
}

void ViewportWidget::beginInteractivePreview()
{
    if (selectedObjectId_ == 0 || !viewportInitialized_ || !isValid() || context() == nullptr) {
        return;
    }

    makeCurrent();
    renderer_.syncScene(ViewportRenderSceneAdapter::buildSceneDataExcludingSubtree(scene_, selectedObjectId_));
    renderer_.setPreviewScene(
        ViewportRenderSceneAdapter::buildSubtreeSceneData(scene_, selectedObjectId_),
        QMatrix4x4());
    applyRendererSelectionState();
    doneCurrent();
    interactivePreviewActive_ = true;
}

void ViewportWidget::updateInteractivePreview()
{
    if (!interactivePreviewActive_ || selectedObjectId_ == 0 || !viewportInitialized_ || !isValid() || context() == nullptr) {
        return;
    }

    const QMatrix4x4 currentWorldMatrix = scene_.worldTransform(selectedObjectId_);
    const QMatrix4x4 previewDelta = currentWorldMatrix * dragState_.previewStartWorldMatrix.inverted();

    makeCurrent();
    renderer_.updatePreviewTransform(previewDelta);
    applyRendererSelectionState();
    doneCurrent();
}

void ViewportWidget::endInteractivePreview()
{
    if (!interactivePreviewActive_ || !viewportInitialized_ || !isValid() || context() == nullptr) {
        interactivePreviewActive_ = false;
        return;
    }

    makeCurrent();
    renderer_.clearPreviewScene();
    doneCurrent();
    interactivePreviewActive_ = false;
}

void ViewportWidget::updateSelectedObject(SceneObject::Id objectId)
{
    setSelectedObject(objectId);
    if (selectionChangedCallback_) {
        selectionChangedCallback_(selectedObjectId_);
    }
}

SceneObject::Id ViewportWidget::pickObjectAtScreenPos(const QPoint& position) const
{
    return ViewportInteractionMath::pickObjectAtScreenPos(
        scene_,
        camera_.projectionMatrix(),
        camera_.viewMatrix(),
        viewportWidth_,
        viewportHeight_,
        position);
}

ViewportWidget::GizmoHandle ViewportWidget::pickGizmoHandleAtScreenPos(const QPoint& position) const
{
    if (selectedObjectId_ == 0) {
        return GizmoHandle::None;
    }

    const QVector3D origin = gizmoOrigin();
    const float size = gizmoSize();
    const QVector<QVector3D> worldAxes = gizmoAxesWorld();
    const QMatrix4x4 projectionMatrix = camera_.projectionMatrix();
    const QMatrix4x4 viewMatrix = camera_.viewMatrix();

    if (transformMode_ == TransformMode::Rotate) {
        return static_cast<GizmoHandle>(RotateGizmoInteraction::pickHandle(
            position,
            origin,
            size,
            worldAxes,
            camera_.forwardDirection(),
            projectionMatrix,
            viewMatrix,
            viewportWidth_,
            viewportHeight_));
    }

    if (transformMode_ == TransformMode::Translate) {
        return static_cast<GizmoHandle>(TranslateGizmoInteraction::pickHandle(
            position,
            origin,
            size,
            worldAxes,
            camera_.forwardDirection(),
            projectionMatrix,
            viewMatrix,
            viewportWidth_,
            viewportHeight_));
    } else if (transformMode_ == TransformMode::Scale) {
        return static_cast<GizmoHandle>(ScaleGizmoInteraction::pickHandle(
            position,
            origin,
            size,
            worldAxes,
            camera_.forwardDirection(),
            projectionMatrix,
            viewMatrix,
            viewportWidth_,
            viewportHeight_));
    }

    return GizmoHandle::None;
}

QVector<QVector3D> ViewportWidget::gizmoAxesWorld() const
{
    QVector<QVector3D> axes = {
        QVector3D(1.0f, 0.0f, 0.0f),
        QVector3D(0.0f, 1.0f, 0.0f),
        QVector3D(0.0f, 0.0f, 1.0f)
    };

    if (axisOrientation_ == AxisOrientation::World || selectedObjectId_ == 0) {
        return axes;
    }

    const QMatrix4x4 objectWorld = scene_.worldTransform(selectedObjectId_);
    axes[0] = objectWorld.mapVector(axes[0]).normalized();
    axes[1] = objectWorld.mapVector(axes[1]).normalized();
    axes[2] = objectWorld.mapVector(axes[2]).normalized();
    return axes;
}

QVector3D ViewportWidget::gizmoAxisDirectionWorld(GizmoHandle handle) const
{
    if (handle == GizmoHandle::None) {
        return QVector3D();
    }

    const QVector<QVector3D> axes = gizmoAxesWorld();
    const int axisIndex = static_cast<int>(handle);
    if (axisIndex < 0 || axisIndex > 2) {
        return QVector3D();
    }

    return axes[axisIndex];
}

QVector3D ViewportWidget::gizmoPlaneNormalWorld(GizmoHandle handle) const
{
    const QVector<QVector3D> axes = gizmoAxesWorld();
    if (axes.size() < 3) {
        return camera_.forwardDirection();
    }

    switch (handle) {
    case GizmoHandle::XY:
        return QVector3D::crossProduct(axes[0], axes[1]).normalized();
    case GizmoHandle::YZ:
        return QVector3D::crossProduct(axes[1], axes[2]).normalized();
    case GizmoHandle::XZ:
        return QVector3D::crossProduct(axes[0], axes[2]).normalized();
    case GizmoHandle::ViewPlane:
        return -camera_.forwardDirection().normalized();
    case GizmoHandle::None:
    case GizmoHandle::X:
    case GizmoHandle::Y:
    case GizmoHandle::Z:
        break;
    }

    return camera_.forwardDirection();
}

bool ViewportWidget::gizmoHandleUsesPlaneDrag(GizmoHandle handle) const
{
    return isPlaneGizmoHandle(handle) || isScreenGizmoHandle(handle);
}

QVector3D ViewportWidget::selectedObjectWorldOrigin() const
{
    if (selectedObjectId_ == 0) {
        return QVector3D();
    }

    return scene_.worldTransform(selectedObjectId_) * QVector3D(0.0f, 0.0f, 0.0f);
}

QString ViewportWidget::cameraViewLabelText() const
{
    switch (cameraViewPreset()) {
    case CameraViewPreset::Front:
        return "front";
    case CameraViewPreset::Back:
        return "back";
    case CameraViewPreset::Left:
        return "left";
    case CameraViewPreset::Right:
        return "side";
    case CameraViewPreset::Top:
        return "top";
    case CameraViewPreset::Bottom:
        return "bottom";
    case CameraViewPreset::Perspective:
        break;
    }

    return "persp";
}

QMatrix4x4 ViewportWidget::parentWorldTransform() const
{
    const SceneObject* object = scene_.findObject(selectedObjectId_);
    if (object == nullptr || object->parentId() == 0) {
        return QMatrix4x4();
    }

    return scene_.worldTransform(object->parentId());
}

QVector3D ViewportWidget::gizmoOrigin() const
{
    const SceneObject* object = scene_.findObject(selectedObjectId_);
    if (object == nullptr) {
        return QVector3D();
    }

    return selectedObjectWorldOrigin();
}

float ViewportWidget::gizmoSize() const
{
    const SceneObject* object = scene_.findObject(selectedObjectId_);
    if (object == nullptr || !object->worldBounds().isValid()) {
        return 1.0f;
    }

    const float unitsPerPixel = camera_.worldUnitsPerPixelAt(selectedObjectWorldOrigin());
    if (transformMode_ == TransformMode::Rotate) {
        return qMax(0.75f, unitsPerPixel * 110.0f);
    }

    return qMax(0.75f, unitsPerPixel * 90.0f);
}

EditorViewportSceneController::Context ViewportWidget::sceneControllerContext()
{
    EditorViewportSceneController::Context context {
        scene_,
        selectedObjectId_,
    };
    context.autoKeyEnabled = autoKeyEnabled_;
    context.notifyBeforeSceneMutation = [this]() {
        notifyBeforeSceneMutation();
    };
    context.syncSceneToRenderer = [this]() {
        syncSceneToRenderer();
    };
    context.requestRender = [this]() {
        requestRender();
    };
    context.setSelectedObject = [this](SceneObject::Id objectId) {
        setSelectedObject(objectId);
    };
    return context;
}

void ViewportWidget::notifyBeforeSceneMutation()
{
    if (!suppressBeforeSceneMutationCallback_ && beforeSceneMutationCallback_) {
        beforeSceneMutationCallback_();
    }
}

void ViewportWidget::applyDrag(const QPoint& currentPosition)
{
    if (!dragState_.active || selectedObjectId_ == 0) {
        return;
    }

    SceneObject* object = scene_.findObject(selectedObjectId_);
    if (object == nullptr) {
        return;
    }

    Transform transform = dragState_.startTransform;
    const QMatrix4x4 projectionMatrix = camera_.projectionMatrix();
    const QMatrix4x4 viewMatrix = camera_.viewMatrix();
    const QMatrix4x4 parentTransform = parentWorldTransform();
    bool updated = false;

    if (transformMode_ == TransformMode::Translate && gizmoHandleUsesPlaneDrag(dragState_.handle)) {
        updated = TranslateGizmoDrag::applyPlane(
            transform,
            dragState_.startTransform,
            currentPosition,
            dragState_.dragPlaneOrigin,
            dragState_.dragPlaneNormal,
            dragState_.dragStartWorldPoint,
            projectionMatrix,
            viewMatrix,
            viewportWidth_,
            viewportHeight_,
            parentTransform);
    } else if (transformMode_ == TransformMode::Translate) {
        updated = TranslateGizmoDrag::applyAxis(
            transform,
            dragState_.startTransform,
            currentPosition,
            dragState_.startMousePosition,
            dragState_.gizmoOrigin,
            dragState_.gizmoSize,
            gizmoAxisDirectionWorld(dragState_.handle),
            projectionMatrix,
            viewMatrix,
            viewportWidth_,
            viewportHeight_,
            parentTransform);
    } else if (transformMode_ == TransformMode::Rotate && dragState_.handle == GizmoHandle::ViewPlane) {
        updated = RotateGizmoDrag::applyScreen(
            transform,
            dragState_.startTransform,
            currentPosition,
            dragState_.startScreenVector,
            dragState_.gizmoOrigin,
            dragState_.dragPlaneNormal,
            projectionMatrix,
            viewMatrix,
            viewportWidth_,
            viewportHeight_,
            parentTransform);
    } else if (transformMode_ == TransformMode::Rotate) {
        updated = RotateGizmoDrag::applyAxis(
            transform,
            dragState_.startTransform,
            currentPosition,
            dragState_.gizmoOrigin,
            dragState_.dragPlaneOrigin,
            dragState_.dragPlaneNormal,
            dragState_.dragStartWorldPoint,
            projectionMatrix,
            viewMatrix,
            viewportWidth_,
            viewportHeight_,
            parentTransform,
            axisOrientation_ == AxisOrientation::World,
            axisVector(static_cast<int>(dragState_.handle)));
    } else if (transformMode_ == TransformMode::Scale
        && (dragState_.handle == GizmoHandle::XY
            || dragState_.handle == GizmoHandle::YZ
            || dragState_.handle == GizmoHandle::XZ
            || dragState_.handle == GizmoHandle::ViewPlane)) {
        updated = ScaleGizmoDrag::applyMultiAxis(
            transform,
            dragState_.startTransform,
            dragState_.handle,
            currentPosition,
            dragState_.startMousePosition);
    } else {
        const QVector3D worldAxis = gizmoAxisDirectionWorld(dragState_.handle);
        const QPointF originScreen = ViewportInteractionMath::projectWorldToScreen(
            camera_.projectionMatrix(),
            camera_.viewMatrix(),
            dragState_.gizmoOrigin,
            viewportWidth_,
            viewportHeight_);
        const QPointF axisScreen = ViewportInteractionMath::projectWorldToScreen(
            projectionMatrix,
            viewMatrix,
            dragState_.gizmoOrigin + worldAxis * dragState_.gizmoSize,
            viewportWidth_,
            viewportHeight_);
        QVector2D axisDirection(axisScreen - originScreen);
        if (qFuzzyIsNull(axisDirection.lengthSquared())) {
            return;
        }
        axisDirection.normalize();
        const QVector2D mouseDelta(currentPosition - dragState_.startMousePosition);
        const float screenDelta = QVector2D::dotProduct(mouseDelta, axisDirection);
        const float axisPixelLength = QLineF(originScreen, axisScreen).length();
        updated = ScaleGizmoDrag::applyAxis(
            transform,
            dragState_.startTransform,
            dragState_.handle,
            screenDelta,
            axisPixelLength);
    }

    if (!updated) {
        return;
    }

    if (!dragState_.historyCaptured) {
        notifyBeforeSceneMutation();
        dragState_.historyCaptured = true;
    }

    suppressBeforeSceneMutationCallback_ = true;
    if (!scene_.setLocalTransformInteractive(selectedObjectId_, transform, autoKeyEnabled_)) {
        suppressBeforeSceneMutationCallback_ = false;
        return;
    }
    suppressBeforeSceneMutationCallback_ = false;

    updateInteractivePreview();

    const bool shouldNotifyUi = !dragRenderSyncTimer_.isValid() || dragRenderSyncTimer_.elapsed() >= 16;
    if (shouldNotifyUi) {
        dragRenderSyncTimer_.restart();
        if (objectTransformChangedCallback_) {
            objectTransformChangedCallback_(selectedObjectId_);
        }
    }

    requestRender();
}

void ViewportWidget::handleHotkeys(QMouseEvent* event)
{
    if (event == nullptr) {
        return;
    }
}
