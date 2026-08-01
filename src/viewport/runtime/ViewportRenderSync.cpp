#include "viewport/runtime/ViewportRenderSync.h"

#include "rendering/scene/ViewportRenderSceneAdapter.h"

namespace ViewportRenderSync
{
void syncScene(ViewportRenderer& renderer, const Scene& scene)
{
    renderer.syncScene(ViewportRenderSceneAdapter::buildSceneData(scene));
}

void applySelectionState(ViewportRenderer& renderer, const ViewportRenderSelectionState& state)
{
    if (!state.hasSelection) {
        renderer.setSelectedBounds(Bounds3D());
        renderer.clearGizmo();
        return;
    }

    renderer.setSelectedBounds(state.selectedBounds);
    renderer.setGizmo(
        state.gizmoOrigin,
        state.gizmoSize,
        state.gizmoMode,
        state.gizmoAxes,
        state.cameraForward,
        state.activeAxis);
}
}
