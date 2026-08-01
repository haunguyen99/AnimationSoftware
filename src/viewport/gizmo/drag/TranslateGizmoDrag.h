#pragma once

#include <QMatrix4x4>
#include <QPoint>
#include <QVector3D>

#include "scene/Transform.h"

namespace TranslateGizmoDrag
{
bool applyPlane(
    Transform& transform,
    const Transform& startTransform,
    const QPoint& currentPosition,
    const QVector3D& dragPlaneOrigin,
    const QVector3D& dragPlaneNormal,
    const QVector3D& dragStartWorldPoint,
    const QMatrix4x4& projectionMatrix,
    const QMatrix4x4& viewMatrix,
    int viewportWidth,
    int viewportHeight,
    const QMatrix4x4& parentWorldTransform);

bool applyAxis(
    Transform& transform,
    const Transform& startTransform,
    const QPoint& currentPosition,
    const QPoint& startMousePosition,
    const QVector3D& gizmoOrigin,
    float gizmoSize,
    const QVector3D& worldAxis,
    const QMatrix4x4& projectionMatrix,
    const QMatrix4x4& viewMatrix,
    int viewportWidth,
    int viewportHeight,
    const QMatrix4x4& parentWorldTransform);
}
