#pragma once

#include <QMatrix4x4>
#include <QPoint>
#include <QPointF>
#include <QVector>
#include <QVector3D>

#include "scene/Bounds3D.h"
#include "scene/Scene.h"

namespace ViewportInteractionMath
{
float rayBoundsDistance(const QVector3D& rayOrigin, const QVector3D& rayDirection, const Bounds3D& bounds);

QPointF projectWorldToScreen(
    const QMatrix4x4& projectionMatrix,
    const QMatrix4x4& viewMatrix,
    const QVector3D& worldPosition,
    int viewportWidth,
    int viewportHeight);

void screenPosToWorldRay(
    const QMatrix4x4& projectionMatrix,
    const QMatrix4x4& viewMatrix,
    int viewportWidth,
    int viewportHeight,
    const QPoint& position,
    QVector3D& rayOrigin,
    QVector3D& rayDirection);

bool intersectRayPlane(
    const QVector3D& rayOrigin,
    const QVector3D& rayDirection,
    const QVector3D& planeOrigin,
    const QVector3D& planeNormal,
    QVector3D* hitPoint);

SceneObject::Id pickObjectAtScreenPos(
    const Scene& scene,
    const QMatrix4x4& projectionMatrix,
    const QMatrix4x4& viewMatrix,
    int viewportWidth,
    int viewportHeight,
    const QPoint& position);

int pickRotateGizmoAxisAtScreenPos(
    const QPoint& position,
    const QVector3D& origin,
    float size,
    const QVector<QVector3D>& worldAxes,
    const QVector3D& cameraForward,
    const QMatrix4x4& projectionMatrix,
    const QMatrix4x4& viewMatrix,
    int viewportWidth,
    int viewportHeight);

int pickTranslateScaleGizmoAxisAtScreenPos(
    const QPoint& position,
    const QVector3D& origin,
    float size,
    const QVector<QVector3D>& worldAxes,
    const QMatrix4x4& projectionMatrix,
    const QMatrix4x4& viewMatrix,
    int viewportWidth,
    int viewportHeight,
    bool includeScaleHandle);

int pickTranslateGizmoHandleAtScreenPos(
    const QPoint& position,
    const QVector3D& origin,
    float size,
    const QVector<QVector3D>& worldAxes,
    const QVector3D& cameraForward,
    const QMatrix4x4& projectionMatrix,
    const QMatrix4x4& viewMatrix,
    int viewportWidth,
    int viewportHeight);

int pickScaleGizmoHandleAtScreenPos(
    const QPoint& position,
    const QVector3D& origin,
    float size,
    const QVector<QVector3D>& worldAxes,
    const QVector3D& cameraForward,
    const QMatrix4x4& projectionMatrix,
    const QMatrix4x4& viewMatrix,
    int viewportWidth,
    int viewportHeight);
}
