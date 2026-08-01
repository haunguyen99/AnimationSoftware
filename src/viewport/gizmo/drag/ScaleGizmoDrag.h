#pragma once

#include <QPoint>
#include <QVector3D>

#include "scene/Transform.h"
#include "viewport/gizmo/GizmoTypes.h"

namespace ScaleGizmoDrag
{
bool applyMultiAxis(
    Transform& transform,
    const Transform& startTransform,
    GizmoHandle handle,
    const QPoint& currentPosition,
    const QPoint& startMousePosition);

bool applyAxis(
    Transform& transform,
    const Transform& startTransform,
    GizmoHandle handle,
    float screenDelta,
    float axisPixelLength);
}
