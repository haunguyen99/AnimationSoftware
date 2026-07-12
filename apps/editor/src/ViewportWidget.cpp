#include "ViewportWidget.h"

#include <QDebug>
#include <QLineF>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QVector2D>
#include <QtMath>

#include <limits>

#include "logging/LogCategories.h"

namespace
{
constexpr int kRotatePickSegments = 48;

float rayBoundsDistance(const QVector3D& rayOrigin, const QVector3D& rayDirection, const Bounds3D& bounds)
{
    if (!bounds.isValid()) {
        return -1.0f;
    }

    float tMin = 0.0f;
    float tMax = std::numeric_limits<float>::max();
    const QVector3D minPoint = bounds.min();
    const QVector3D maxPoint = bounds.max();

    for (int axis = 0; axis < 3; ++axis) {
        const float origin = axis == 0 ? rayOrigin.x() : (axis == 1 ? rayOrigin.y() : rayOrigin.z());
        const float direction = axis == 0 ? rayDirection.x() : (axis == 1 ? rayDirection.y() : rayDirection.z());
        const float minValue = axis == 0 ? minPoint.x() : (axis == 1 ? minPoint.y() : minPoint.z());
        const float maxValue = axis == 0 ? maxPoint.x() : (axis == 1 ? maxPoint.y() : maxPoint.z());

        if (qFuzzyIsNull(direction)) {
            if (origin < minValue || origin > maxValue) {
                return -1.0f;
            }
            continue;
        }

        const float inverse = 1.0f / direction;
        float t1 = (minValue - origin) * inverse;
        float t2 = (maxValue - origin) * inverse;
        if (t1 > t2) {
            std::swap(t1, t2);
        }

        tMin = qMax(tMin, t1);
        tMax = qMin(tMax, t2);
        if (tMin > tMax) {
            return -1.0f;
        }
    }

    return tMin;
}

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

qreal distancePointToSegment(const QPointF& point, const QPointF& a, const QPointF& b)
{
    const QVector2D line(b - a);
    if (qFuzzyIsNull(line.lengthSquared())) {
        return QLineF(point, a).length();
    }

    const QVector2D offset(point - a);
    const float t = qBound(0.0f, QVector2D::dotProduct(offset, line) / line.lengthSquared(), 1.0f);
    const QPointF closest = a + (b - a) * t;
    return QLineF(point, closest).length();
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
    requestRender();
}

void ViewportWidget::frameScene()
{
    if (!scene_.isEmpty() && scene_.sceneBounds().isValid()) {
        camera_.frameBounds(scene_.sceneBounds().center(), qMax(1.0f, scene_.sceneBounds().radius()));
    } else {
        camera_.frameScene();
    }
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

    if (scene_.isEmpty()) {
        scene_ = result.scene;
    } else {
        scene_.appendScene(result.scene);
    }

    syncSceneToRenderer();

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
    scene_.clear();
    selectedObjectId_ = 0;
    syncSceneToRenderer();
    requestRender();
}

void ViewportWidget::replaceScene(const Scene& scene)
{
    scene_ = scene;
    selectedObjectId_ = 0;
    syncSceneToRenderer();
    requestRender();
}

void ViewportWidget::optimizeSceneStorage()
{
    scene_.optimizeStorage();
    syncSceneToRenderer();
    requestRender();
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
    if (!scene_.setLocalTransform(objectId, transform, autoKeyEnabled_)) {
        return false;
    }

    syncSceneToRenderer();
    requestRender();
    return true;
}

bool ViewportWidget::setObjectVisibility(SceneObject::Id objectId, bool visible)
{
    if (!scene_.setObjectVisible(objectId, visible)) {
        return false;
    }

    syncSceneToRenderer();
    requestRender();
    return true;
}

int ViewportWidget::currentFrame() const
{
    return scene_.currentFrame();
}

void ViewportWidget::setCurrentFrame(int frame)
{
    scene_.setCurrentFrame(frame);
    syncSceneToRenderer();
    requestRender();
}

bool ViewportWidget::setObjectKeyframe(SceneObject::Id objectId, int frame)
{
    if (!scene_.setObjectKeyframe(objectId, frame)) {
        return false;
    }

    syncSceneToRenderer();
    requestRender();
    return true;
}

bool ViewportWidget::removeObjectKeyframe(SceneObject::Id objectId, int frame)
{
    if (!scene_.removeObjectKeyframe(objectId, frame)) {
        return false;
    }

    syncSceneToRenderer();
    requestRender();
    return true;
}

SceneObject::Id ViewportWidget::createPrimitive(PrimitiveMeshFactory::Type type, const QString& name)
{
    if (!PrimitiveMeshFactory::isImplemented(type)) {
        return 0;
    }

    const MeshData meshData = PrimitiveMeshFactory::createMesh(type);
    if (meshData.positions.isEmpty() || meshData.indices.isEmpty() || !meshData.bounds.isValid()) {
        return 0;
    }

    const SceneObject::Id objectId = scene_.createObject(name.isEmpty() ? PrimitiveMeshFactory::displayName(type) : name);
    SceneObject* object = scene_.findObject(objectId);
    if (object == nullptr) {
        return 0;
    }

    const int meshHandle = scene_.addMesh(meshData);
    object->addMeshHandle(meshHandle);
    object->setLocalBounds(meshData.bounds);
    scene_.rebuildWorldData();

    syncSceneToRenderer();

    setSelectedObject(objectId);
    requestRender();
    return objectId;
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
    renderer_.syncScene(scene_);
    syncRendererSelection();
    qCInfo(logViewport) << "init ok";
}

void ViewportWidget::resizeGL(int width, int height)
{
    viewportWidth_ = width;
    viewportHeight_ = height;

    glViewport(0, 0, width, height);
    camera_.setViewportSize(width, height);
    renderer_.resize(width, height);
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
}

void ViewportWidget::mousePressEvent(QMouseEvent* event)
{
    handleHotkeys(event);
    lastMousePosition_ = event->pos();
    activeButtons_ = event->buttons();

    if (!(event->modifiers() & Qt::AltModifier) && event->button() == Qt::LeftButton && selectedObjectId_ != 0) {
        const GizmoAxis axis = pickGizmoAxisAtScreenPos(event->pos());
        if (axis != GizmoAxis::None) {
            dragState_.active = true;
            dragState_.startMousePosition = event->pos();
            dragState_.axis = axis;
            dragState_.gizmoOrigin = gizmoOrigin();
            dragState_.gizmoSize = gizmoSize();

            if (const SceneObject* object = scene_.findObject(selectedObjectId_)) {
                dragState_.startTransform = object->localTransform();
            }

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
        requestRender();
    } else if ((event->modifiers() & Qt::AltModifier) && (event->buttons() & Qt::MiddleButton)) {
        camera_.pan(static_cast<float>(delta.x()), static_cast<float>(delta.y()));
        requestRender();
    }

    event->accept();
}

void ViewportWidget::mouseReleaseEvent(QMouseEvent* event)
{
    activeButtons_ = event->buttons();
    if (dragState_.active && event->button() == Qt::LeftButton) {
        dragState_ = {};
        syncRendererSelection();
        requestRender();
    }
    event->accept();
}

void ViewportWidget::wheelEvent(QWheelEvent* event)
{
    camera_.zoom(static_cast<float>(event->angleDelta().y()) / 120.0f);
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
        ViewportRenderer::GizmoMode gizmoMode = ViewportRenderer::GizmoMode::Translate;
        if (transformMode_ == TransformMode::Rotate) {
            gizmoMode = ViewportRenderer::GizmoMode::Rotate;
        } else if (transformMode_ == TransformMode::Scale) {
            gizmoMode = ViewportRenderer::GizmoMode::Scale;
        }

        renderer_.setSelectedBounds(object->worldBounds());
        renderer_.setGizmo(
            gizmoOrigin(),
            gizmoSize(),
            gizmoMode,
            gizmoAxesWorld(),
            camera_.forwardDirection(),
            dragState_.active ? static_cast<int>(dragState_.axis) : -1);
    } else {
        renderer_.setSelectedBounds(Bounds3D());
        renderer_.clearGizmo();
    }
}

void ViewportWidget::syncSceneToRenderer()
{
    if (!viewportInitialized_ || !isValid() || context() == nullptr) {
        return;
    }

    makeCurrent();
    renderer_.syncScene(scene_);
    applyRendererSelectionState();
    doneCurrent();
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
    const QMatrix4x4 inverseProjectionView = (camera_.projectionMatrix() * camera_.viewMatrix()).inverted();
    const float x = (2.0f * static_cast<float>(position.x()) / qMax(1, viewportWidth_)) - 1.0f;
    const float y = 1.0f - (2.0f * static_cast<float>(position.y()) / qMax(1, viewportHeight_));
    const QVector4D nearPoint = inverseProjectionView * QVector4D(x, y, -1.0f, 1.0f);
    const QVector4D farPoint = inverseProjectionView * QVector4D(x, y, 1.0f, 1.0f);
    const QVector3D rayOrigin = nearPoint.toVector3DAffine();
    const QVector3D rayDirection = (farPoint.toVector3DAffine() - rayOrigin).normalized();

    SceneObject::Id bestId = 0;
    float bestDistance = std::numeric_limits<float>::max();
    for (SceneObject::Id objectId : scene_.allObjectIds()) {
        const SceneObject* object = scene_.findObject(objectId);
        if (object == nullptr || !object->isVisible() || !object->worldBounds().isValid()) {
            continue;
        }

        const float distance = rayBoundsDistance(rayOrigin, rayDirection, object->worldBounds());
        if (distance >= 0.0f && distance < bestDistance) {
            bestDistance = distance;
            bestId = objectId;
        }
    }

    return bestId;
}

ViewportWidget::GizmoAxis ViewportWidget::pickGizmoAxisAtScreenPos(const QPoint& position) const
{
    if (selectedObjectId_ == 0) {
        return GizmoAxis::None;
    }

    const QVector3D origin = gizmoOrigin();
    const float size = gizmoSize();
    const QPointF mousePoint(position);
    const QVector<QVector3D> worldAxes = gizmoAxesWorld();

    if (transformMode_ == TransformMode::Rotate) {
        GizmoAxis bestAxis = GizmoAxis::None;
        qreal bestDistance = 14.0;

        for (GizmoAxis axis : { GizmoAxis::X, GizmoAxis::Y, GizmoAxis::Z }) {
            const QVector3D normalAxis = worldAxes[static_cast<int>(axis)].normalized();
            QVector3D basisA = QVector3D::crossProduct(normalAxis, QVector3D(0.0f, 1.0f, 0.0f));
            if (basisA.lengthSquared() < 0.0001f) {
                basisA = QVector3D::crossProduct(normalAxis, QVector3D(1.0f, 0.0f, 0.0f));
            }
            basisA.normalize();
            const QVector3D basisB = QVector3D::crossProduct(normalAxis, basisA).normalized();

            qreal axisDistance = bestDistance;
            for (int segment = 0; segment < kRotatePickSegments; ++segment) {
                const float angleA = (static_cast<float>(segment) / kRotatePickSegments) * 360.0f;
                const float angleB = (static_cast<float>(segment + 1) / kRotatePickSegments) * 360.0f;
                const float radiansA = qDegreesToRadians(angleA);
                const float radiansB = qDegreesToRadians(angleB);

                const QVector3D pointA3D = origin + (basisA * qCos(radiansA) + basisB * qSin(radiansA)) * (size * 0.9f);
                const QVector3D pointB3D = origin + (basisA * qCos(radiansB) + basisB * qSin(radiansB)) * (size * 0.9f);
                const QPointF pointA = projectWorldToScreen(pointA3D);
                const QPointF pointB = projectWorldToScreen(pointB3D);

                axisDistance = qMin(axisDistance, distancePointToSegment(mousePoint, pointA, pointB));
            }

            if (axisDistance < bestDistance) {
                bestDistance = axisDistance;
                bestAxis = axis;
            }
        }

        return bestAxis;
    }

    GizmoAxis bestAxis = GizmoAxis::None;
    qreal bestDistance = transformMode_ == TransformMode::Scale ? 12.0 : 8.0;

    for (GizmoAxis axis : { GizmoAxis::X, GizmoAxis::Y, GizmoAxis::Z }) {
        const QPointF a = projectWorldToScreen(origin);
        const QPointF b = projectWorldToScreen(origin + worldAxes[static_cast<int>(axis)] * size);
        qreal distance = distancePointToSegment(mousePoint, a, b);

        if (transformMode_ == TransformMode::Scale) {
            const QPointF handleCenter = b;
            distance = qMin(distance, QLineF(mousePoint, handleCenter).length());
        }

        if (distance < bestDistance) {
            bestDistance = distance;
            bestAxis = axis;
        }
    }

    return bestAxis;
}

QPointF ViewportWidget::projectWorldToScreen(const QVector3D& worldPosition) const
{
    const QVector4D clipPosition = camera_.projectionMatrix() * camera_.viewMatrix() * QVector4D(worldPosition, 1.0f);
    if (qFuzzyIsNull(clipPosition.w())) {
        return QPointF();
    }

    const QVector3D ndc = clipPosition.toVector3DAffine();
    const qreal screenX = (ndc.x() * 0.5 + 0.5) * viewportWidth_;
    const qreal screenY = (1.0 - (ndc.y() * 0.5 + 0.5)) * viewportHeight_;
    return QPointF(screenX, screenY);
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

QVector3D ViewportWidget::gizmoAxisDirectionWorld(GizmoAxis axis) const
{
    if (axis == GizmoAxis::None) {
        return QVector3D();
    }

    const QVector<QVector3D> axes = gizmoAxesWorld();
    return axes[static_cast<int>(axis)];
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
    return object != nullptr && object->worldBounds().isValid()
        ? object->worldBounds().center()
        : QVector3D();
}

float ViewportWidget::gizmoSize() const
{
    const SceneObject* object = scene_.findObject(selectedObjectId_);
    if (object == nullptr || !object->worldBounds().isValid()) {
        return 1.0f;
    }

    const float unitsPerPixel = camera_.worldUnitsPerPixelAt(object->worldBounds().center());
    if (transformMode_ == TransformMode::Rotate) {
        return qMax(0.75f, unitsPerPixel * 110.0f);
    }

    return qMax(0.75f, unitsPerPixel * 90.0f);
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
    const QVector3D worldAxis = gizmoAxisDirectionWorld(dragState_.axis);
    const QPointF originScreen = projectWorldToScreen(dragState_.gizmoOrigin);
    const QPointF axisScreen = projectWorldToScreen(dragState_.gizmoOrigin + worldAxis * dragState_.gizmoSize);
    QVector2D axisDirection(axisScreen - originScreen);
    if (qFuzzyIsNull(axisDirection.lengthSquared())) {
        return;
    }

    axisDirection.normalize();
    const QVector2D mouseDelta(currentPosition - dragState_.startMousePosition);
    const float screenDelta = QVector2D::dotProduct(mouseDelta, axisDirection);
    const float axisPixelLength = QLineF(originScreen, axisScreen).length();
    if (qFuzzyIsNull(axisPixelLength)) {
        return;
    }

    if (transformMode_ == TransformMode::Translate) {
        const float worldDelta = screenDelta * (dragState_.gizmoSize / axisPixelLength);
        const QMatrix4x4 parentInverse = parentWorldTransform().inverted();
        transform.translation += parentInverse.mapVector(worldAxis * worldDelta);
    } else if (transformMode_ == TransformMode::Rotate) {
        // Rotate drag should follow the perceived mouse direction instead of
        // feeling mirrored relative to the screen-space gizmo guide.
        const float angleDegrees = screenDelta * -0.5f;
        if (axisOrientation_ == AxisOrientation::World) {
            const QMatrix4x4 parentInverse = parentWorldTransform().inverted();
            const QVector3D localAxis = parentInverse.mapVector(worldAxis).normalized();
            transform.rotation = QQuaternion::fromAxisAndAngle(localAxis, angleDegrees) * transform.rotation;
        } else {
            transform.rotation = transform.rotation * QQuaternion::fromAxisAndAngle(axisVector(static_cast<int>(dragState_.axis)), angleDegrees);
        }
    } else {
        const float scaleDelta = screenDelta / axisPixelLength;
        QVector3D scale = transform.scale;
        if (dragState_.axis == GizmoAxis::X) {
            scale.setX(qMax(0.05f, dragState_.startTransform.scale.x() + scaleDelta));
        } else if (dragState_.axis == GizmoAxis::Y) {
            scale.setY(qMax(0.05f, dragState_.startTransform.scale.y() + scaleDelta));
        } else if (dragState_.axis == GizmoAxis::Z) {
            scale.setZ(qMax(0.05f, dragState_.startTransform.scale.z() + scaleDelta));
        }
        transform.scale = scale;
    }

    if (!setObjectLocalTransform(selectedObjectId_, transform)) {
        return;
    }

    if (objectTransformChangedCallback_) {
        objectTransformChangedCallback_(selectedObjectId_);
    }

    requestRender();
}

void ViewportWidget::handleHotkeys(QMouseEvent* event)
{
    if (event == nullptr) {
        return;
    }
}
