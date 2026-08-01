#include "viewport/gizmo/drag/ScaleGizmoDrag.h"

namespace ScaleGizmoDrag
{
bool applyMultiAxis(
    Transform& transform,
    const Transform& startTransform,
    GizmoHandle handle,
    const QPoint& currentPosition,
    const QPoint& startMousePosition)
{
    const float scaleDelta =
        (static_cast<float>(currentPosition.x() - startMousePosition.x())
            - static_cast<float>(currentPosition.y() - startMousePosition.y())) / 160.0f;
    QVector3D scale = startTransform.scale;

    if (handle == GizmoHandle::XY) {
        scale.setX(qMax(0.05f, startTransform.scale.x() + scaleDelta));
        scale.setY(qMax(0.05f, startTransform.scale.y() + scaleDelta));
    } else if (handle == GizmoHandle::YZ) {
        scale.setY(qMax(0.05f, startTransform.scale.y() + scaleDelta));
        scale.setZ(qMax(0.05f, startTransform.scale.z() + scaleDelta));
    } else if (handle == GizmoHandle::XZ) {
        scale.setX(qMax(0.05f, startTransform.scale.x() + scaleDelta));
        scale.setZ(qMax(0.05f, startTransform.scale.z() + scaleDelta));
    } else if (handle == GizmoHandle::ViewPlane) {
        scale.setX(qMax(0.05f, startTransform.scale.x() + scaleDelta));
        scale.setY(qMax(0.05f, startTransform.scale.y() + scaleDelta));
        scale.setZ(qMax(0.05f, startTransform.scale.z() + scaleDelta));
    } else {
        return false;
    }

    transform = startTransform;
    transform.scale = scale;
    return true;
}

bool applyAxis(
    Transform& transform,
    const Transform& startTransform,
    GizmoHandle handle,
    float screenDelta,
    float axisPixelLength)
{
    if (qFuzzyIsNull(axisPixelLength)) {
        return false;
    }

    const float scaleDelta = screenDelta / axisPixelLength;
    QVector3D scale = startTransform.scale;
    if (handle == GizmoHandle::X) {
        scale.setX(qMax(0.05f, startTransform.scale.x() + scaleDelta));
    } else if (handle == GizmoHandle::Y) {
        scale.setY(qMax(0.05f, startTransform.scale.y() + scaleDelta));
    } else if (handle == GizmoHandle::Z) {
        scale.setZ(qMax(0.05f, startTransform.scale.z() + scaleDelta));
    } else {
        return false;
    }

    transform = startTransform;
    transform.scale = scale;
    return true;
}
}
