#include "EditorAnimationEngineFacade.h"

namespace
{
EditorAnimationEngineFacade::OperationResult fromFlowFeedback(const EditorAnimationFlowController::OperationResult& result)
{
    EditorAnimationEngineFacade::OperationResult feedback;
    feedback.success = result.success;
    feedback.errorMessage = result.errorMessage;
    feedback.statusMessage = result.statusMessage;
    feedback.commentLine = result.commentLine;
    feedback.commandLine = result.commandLine;
    feedback.resultLine = result.resultLine;
    return feedback;
}
}

namespace EditorAnimationEngineFacade
{
OperationResult applyFlowResult(const Context& context, const EditorAnimationFlowController::OperationResult& result)
{
    OperationResult feedback = fromFlowFeedback(result);
    if (!result.success) {
        return feedback;
    }

    if (result.sceneChanged) {
        EditorSceneRuntimeController::ApplySceneRequest request;
        request.scene = result.scene;
        request.recordUndo = true;
        request.applyAnimationState = true;
        request.animationState = result.state;
        request.selection.objectId = result.focusObjectId;
        EditorSceneRuntimeController::applyScene(context.runtime, request);
    } else {
        context.applyAnimationState(result.state);
    }

    return feedback;
}

OperationResult setCurrentFrame(const Context& context, int frame, bool logToScript)
{
    return applyFlowResult(
        context,
        EditorAnimationFlowController::setCurrentFrame(context.animationState(), frame, logToScript));
}

OperationResult setKeyForSelection(const Context& context, std::uint64_t selectedObjectId, bool logToScript)
{
    return applyFlowResult(
        context,
        EditorAnimationFlowController::setKeyForSelection(context.flow, context.animationState(), selectedObjectId, logToScript));
}

OperationResult deleteKeyForSelection(const Context& context, std::uint64_t selectedObjectId, bool logToScript)
{
    return applyFlowResult(
        context,
        EditorAnimationFlowController::deleteKeyForSelection(context.flow, context.animationState(), selectedObjectId, logToScript));
}

OperationResult duplicateCurrentKeyForSelection(const Context& context, std::uint64_t selectedObjectId, bool logToScript)
{
    return applyFlowResult(
        context,
        EditorAnimationFlowController::duplicateCurrentKeyForSelection(
            context.flow,
            context.animationState(),
            selectedObjectId,
            logToScript));
}

OperationResult duplicateKeyframeForSelection(
    const Context& context,
    std::uint64_t selectedObjectId,
    int sourceFrame,
    int targetFrame,
    bool logToScript)
{
    return applyFlowResult(
        context,
        EditorAnimationFlowController::duplicateKeyframeForSelection(
            context.flow,
            context.animationState(),
            selectedObjectId,
            sourceFrame,
            targetFrame,
            logToScript));
}

OperationResult shiftSelectedObjectKeyframes(
    const Context& context,
    std::uint64_t selectedObjectId,
    int frameDelta,
    bool logToScript)
{
    return applyFlowResult(
        context,
        EditorAnimationFlowController::shiftSelectedObjectKeyframes(
            context.flow,
            context.animationState(),
            selectedObjectId,
            frameDelta,
            logToScript));
}

OperationResult setAutoKeyEnabled(const Context& context, bool enabled, bool logToScript)
{
    return applyFlowResult(
        context,
        EditorAnimationFlowController::setAutoKeyEnabled(context.animationState(), enabled, logToScript));
}

OperationResult setPlaybackRange(const Context& context, int startFrame, int endFrame, bool logToScript)
{
    return applyFlowResult(
        context,
        EditorAnimationFlowController::setPlaybackRange(context.animationState(), startFrame, endFrame, logToScript));
}

OperationResult setPlaybackState(const Context& context, bool playing, bool logToScript)
{
    return applyFlowResult(
        context,
        EditorAnimationFlowController::setPlaybackState(context.animationState(), playing, logToScript));
}

OperationResult jumpToSelectedObjectKeyframe(
    const Context& context,
    std::uint64_t selectedObjectId,
    bool forward,
    bool logToScript)
{
    return applyFlowResult(
        context,
        EditorAnimationFlowController::jumpToSelectedObjectKeyframe(
            context.flow,
            context.animationState(),
            selectedObjectId,
            forward,
            logToScript));
}

void bindScriptCommands(ScriptCommandContext& context, const ScriptBindings& bindings)
{
    context.setCurrentFrame = [bindings](int frame) {
        setCurrentFrame(bindings.context, frame, false);
    };
    context.setKeyframe = [bindings](const QString& objectName, int frame) {
        const SceneObject::Id objectId = bindings.findObjectIdByName(objectName);
        if (objectId == 0) {
            return false;
        }

        bindings.selectObjectById(objectId);
        if (frame >= 0 && !setCurrentFrame(bindings.context, frame, false).success) {
            return false;
        }

        return setKeyForSelection(bindings.context, objectId, false).success;
    };
    context.deleteKeyframe = [bindings](const QString& objectName, int frame) {
        const SceneObject::Id objectId = bindings.findObjectIdByName(objectName);
        if (objectId == 0) {
            return false;
        }

        bindings.selectObjectById(objectId);
        if (frame >= 0 && !setCurrentFrame(bindings.context, frame, false).success) {
            return false;
        }

        return deleteKeyForSelection(bindings.context, objectId, false).success;
    };
    context.copyKeyframe = [bindings](const QString& objectName, int sourceFrame, int targetFrame) {
        const SceneObject::Id objectId = bindings.findObjectIdByName(objectName);
        if (objectId == 0) {
            return false;
        }

        bindings.selectObjectById(objectId);
        return duplicateKeyframeForSelection(bindings.context, objectId, sourceFrame, targetFrame, false).success;
    };
    context.shiftKeyframes = [bindings](const QString& objectName, int frameDelta) {
        const SceneObject::Id objectId = bindings.findObjectIdByName(objectName);
        if (objectId == 0) {
            return false;
        }

        bindings.selectObjectById(objectId);
        return shiftSelectedObjectKeyframes(bindings.context, objectId, frameDelta, false).success;
    };
    context.setAutoKey = [bindings](bool enabled) {
        setAutoKeyEnabled(bindings.context, enabled, false);
    };
    context.setPlaybackRange = [bindings](int startFrame, int endFrame) {
        setPlaybackRange(bindings.context, startFrame, endFrame, false);
    };
    context.setPlaybackState = [bindings](bool playing) {
        setPlaybackState(bindings.context, playing, false);
    };
}
}
