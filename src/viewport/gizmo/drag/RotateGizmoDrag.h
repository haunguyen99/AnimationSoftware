#pragma once

#include <QMatrix4x4>
#include <QPoint>
#include <QPointF>
#include <QQuaternion>
#include <QVector3D>

#include "scene/Transform.h"

namespace RotateGizmoDrag
{
bool applyScreen(
    Transform& transform,
    const Transform& startTransform,
    const QPoint& currentPosition,
    const QPointF& startScreenVector,
    const QVector3D& gizmoOrigin,
    const QVector3D& dragPlaneNormal,
    const QMatrix4x4& projectionMatrix,
    const QMatrix4x4& viewMatrix,
    int viewportWidth,
    int viewportHeight,
    const QMatrix4x4& parentWorldTransform);

bool applyAxis(
    Transform& transform,
    const Transform& startTransform,
    const QPoint& currentPosition,
    const QVector3D& gizmoOrigin,
    const QVector3D& dragPlaneOrigin,
    const QVector3D& dragPlaneNormal,
    const QVector3D& dragStartWorldPoint,
    const QMatrix4x4& projectionMatrix,
    const QMatrix4x4& viewMatrix,
    int viewportWidth,
    int viewportHeight,
    const QMatrix4x4& parentWorldTransform,
    bool worldOrientation,
    const QVector3D& localAxis);
}
