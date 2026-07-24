#include "EditorViewportCommandController.h"

namespace EditorViewportCommandController
{
void bindScriptCommands(ScriptCommandContext& context, const ScriptBindings& bindings)
{
    context.frameView = [bindings](const QString& targetName) {
        if (targetName.trimmed().isEmpty()) {
            bindings.frameScene();
            return true;
        }

        const SceneObject::Id objectId = bindings.findObjectIdByName(targetName);
        if (objectId == 0) {
            return false;
        }

        bindings.frameObject(objectId);
        return true;
    };
    context.resetCamera = [bindings]() {
        bindings.resetCamera();
    };
    context.activateTool = [bindings](const QString& toolName) {
        if (toolName == "MoveSuperContext") {
            bindings.setToolActionChecks(true, false, false);
            bindings.setTransformMode(ViewportWorkspaceWidget::TransformMode::Translate);
            return true;
        }

        if (toolName == "RotateSuperContext") {
            bindings.setToolActionChecks(false, true, false);
            bindings.setTransformMode(ViewportWorkspaceWidget::TransformMode::Rotate);
            return true;
        }

        if (toolName == "ScaleSuperContext") {
            bindings.setToolActionChecks(false, false, true);
            bindings.setTransformMode(ViewportWorkspaceWidget::TransformMode::Scale);
            return true;
        }

        return false;
    };
}
}
