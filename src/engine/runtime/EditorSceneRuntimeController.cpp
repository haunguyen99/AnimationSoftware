#include "engine/runtime/EditorSceneRuntimeController.h"

#include "EditorAnimationController.h"

namespace EditorSceneRuntimeController
{
void refreshSelection(const Context& context, const SelectionRefreshRequest& request)
{
    if (request.clearSelection) {
        context.clearInspector();
        return;
    }

    if (request.objectId != 0 && context.containsObject(request.objectId)) {
        context.selectObject(request.objectId, request.syncOutliner);
        return;
    }

    context.clearInspector();
}

void applyScene(const Context& context, const ApplySceneRequest& request)
{
    if (request.recordUndo) {
        context.recordUndoState();
    }

    context.replaceScene(request.scene);

    if (request.applyAnimationState) {
        context.applyAnimationState(request.animationState);
    } else if (request.applyCurrentFrame) {
        context.applyAnimationState(EditorAnimationController::setCurrentFrame(context.animationState(), request.currentFrame));
    }

    context.refreshScenePanels();
    refreshSelection(context, request.selection);

    if (request.frameEntireScene) {
        context.frameScene();
    }
}
}
