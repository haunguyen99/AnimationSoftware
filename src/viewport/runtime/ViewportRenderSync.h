#pragma once

#include <QVector>
#include <QVector3D>

#include "rendering/ViewportRenderer.h"
#include "scene/Scene.h"

struct ViewportRenderSelectionState
{
    bool hasSelection = false;
    Bounds3D selectedBounds;
    QVector3D gizmoOrigin;
    float gizmoSize = 1.0f;
    ViewportRenderer::GizmoMode gizmoMode = ViewportRenderer::GizmoMode::Translate;
    QVector<QVector3D> gizmoAxes;
    QVector3D cameraForward;
    int activeAxis = -1;
};

namespace ViewportRenderSync
{
void syncScene(ViewportRenderer& renderer, const Scene& scene);
void applySelectionState(ViewportRenderer& renderer, const ViewportRenderSelectionState& state);
}
