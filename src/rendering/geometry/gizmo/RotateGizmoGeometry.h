#pragma once

#include <QVector>
#include <QVector3D>

#include "rendering/RenderTypes.h"

namespace RotateGizmoGeometry
{
void appendGizmo(
    QVector<RenderVertex>& vertices,
    const QVector3D& origin,
    float size,
    const QVector<QVector3D>& axes,
    const QVector3D& cameraForward,
    const QVector3D& xColor,
    const QVector3D& yColor,
    const QVector3D& zColor,
    int activeAxis);
}
