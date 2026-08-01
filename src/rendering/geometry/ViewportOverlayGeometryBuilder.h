#pragma once

#include <QVector>
#include <QVector3D>

#include "rendering/RenderTypes.h"
#include "rendering/ViewportRenderer.h"
#include "scene/Bounds3D.h"

namespace ViewportOverlayGeometryBuilder
{
QVector<RenderVertex> buildSelectionBounds(const Bounds3D& bounds);
QVector<RenderVertex> buildGizmo(
    const QVector3D& origin,
    float size,
    ViewportRenderer::GizmoMode mode,
    const QVector<QVector3D>& axes,
    const QVector3D& cameraForward,
    int activeAxis);
}
