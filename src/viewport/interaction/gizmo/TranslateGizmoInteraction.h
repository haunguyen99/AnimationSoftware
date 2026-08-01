#pragma once

#include <QMatrix4x4>
#include <QPoint>
#include <QVector>
#include <QVector3D>

namespace TranslateGizmoInteraction
{
int pickHandle(
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
