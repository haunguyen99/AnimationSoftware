#pragma once

#include <QString>

#include <functional>

#include "core/commands/ScriptCommandSystem.h"
#include "ViewportWorkspaceWidget.h"

namespace EditorViewportCommandController
{
struct ScriptBindings
{
    std::function<SceneObject::Id(const QString&)> findObjectIdByName;
    std::function<void()> resetCamera;
    std::function<void()> frameScene;
    std::function<void(SceneObject::Id)> frameObject;
    std::function<void(ViewportWorkspaceWidget::TransformMode)> setTransformMode;
    std::function<void(bool, bool, bool)> setToolActionChecks;
};

void bindScriptCommands(ScriptCommandContext& context, const ScriptBindings& bindings);
}
