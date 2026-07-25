#pragma once

#include <cstdint>
#include <functional>

#include "EditorAnimationFlowController.h"
#include "EditorSceneRuntimeController.h"
#include "ScriptCommandSystem.h"

namespace EditorAnimationEngineFacade
{
struct Context
{
    EditorAnimationFlowController::Context flow;
    EditorSceneRuntimeController::Context runtime;
    std::function<EditorAnimationState()> animationState;
    std::function<void(const EditorAnimationState&)> applyAnimationState;
};

struct ScriptBindings
{
    Context context;
    std::function<SceneObject::Id(const QString&)> findObjectIdByName;
    std::function<void(SceneObject::Id)> selectObjectById;
};

struct OperationResult
{
    bool success = false;
    QString errorMessage;
    QString statusMessage;
    QString commentLine;
    QString commandLine;
    QString resultLine;
};

OperationResult applyFlowResult(const Context& context, const EditorAnimationFlowController::OperationResult& result);
OperationResult setCurrentFrame(const Context& context, int frame, bool logToScript);
OperationResult setKeyForSelection(const Context& context, std::uint64_t selectedObjectId, bool logToScript);
OperationResult deleteKeyForSelection(const Context& context, std::uint64_t selectedObjectId, bool logToScript);
OperationResult duplicateCurrentKeyForSelection(const Context& context, std::uint64_t selectedObjectId, bool logToScript);
OperationResult duplicateKeyframeForSelection(
    const Context& context,
    std::uint64_t selectedObjectId,
    int sourceFrame,
    int targetFrame,
    bool logToScript);
OperationResult shiftSelectedObjectKeyframes(
    const Context& context,
    std::uint64_t selectedObjectId,
    int frameDelta,
    bool logToScript);
OperationResult setAutoKeyEnabled(const Context& context, bool enabled, bool logToScript);
OperationResult setPlaybackRange(const Context& context, int startFrame, int endFrame, bool logToScript);
OperationResult setPlaybackState(const Context& context, bool playing, bool logToScript);
OperationResult jumpToSelectedObjectKeyframe(
    const Context& context,
    std::uint64_t selectedObjectId,
    bool forward,
    bool logToScript);

void bindScriptCommands(ScriptCommandContext& context, const ScriptBindings& bindings);
}
