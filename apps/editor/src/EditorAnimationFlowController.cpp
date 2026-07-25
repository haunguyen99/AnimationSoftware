#include "EditorAnimationFlowController.h"

#include "EditorSceneQueryController.h"

namespace
{
using EditorSceneQueryController::objectDisplayName;

EditorAnimationFlowController::OperationResult invalidSelectionResult(const QString& message, const EditorAnimationState& state)
{
    EditorAnimationFlowController::OperationResult result;
    result.state = state;
    result.errorMessage = message;
    return result;
}
}

namespace EditorAnimationFlowController
{
OperationResult setCurrentFrame(const EditorAnimationState& state, int frame, bool logToScript)
{
    OperationResult result;
    result.success = true;
    result.state = EditorAnimationController::setCurrentFrame(state, frame);
    result.statusMessage = QString("Current frame: %1").arg(result.state.currentFrame);
    if (logToScript) {
        result.commandLine = QString("currentTime %1;").arg(result.state.currentFrame);
        result.resultLine = QString("// Result: current frame %1 //").arg(result.state.currentFrame);
    }
    return result;
}

OperationResult setKeyForSelection(const Context& context, const EditorAnimationState& state, std::uint64_t selectedObjectId, bool logToScript)
{
    if (selectedObjectId == 0) {
        return invalidSelectionResult("Select an object to key", state);
    }

    const SceneObject* object = context.findObject(selectedObjectId);
    if (object == nullptr) {
        return invalidSelectionResult("Selected object is no longer available", state);
    }

    const EditorAnimationController::SceneMutationResult mutation =
        EditorAnimationController::setKeyframe(context.sceneSnapshot(), selectedObjectId, state.currentFrame);
    if (!mutation.success) {
        return invalidSelectionResult("Set key failed", state);
    }

    OperationResult result;
    result.success = true;
    result.sceneChanged = true;
    result.focusObjectId = selectedObjectId;
    result.scene = mutation.scene;
    result.state = EditorAnimationController::setCurrentFrame(state, mutation.currentFrame);
    result.statusMessage = QString("Key set at frame %1").arg(result.state.currentFrame);
    if (logToScript) {
        const QString objectName = objectDisplayName(*object);
        result.commandLine = QString("setKeyframe %1 -t %2;").arg(objectName).arg(result.state.currentFrame);
        result.resultLine = QString("// Result: key set on %1 at frame %2 //").arg(objectName).arg(result.state.currentFrame);
    }
    return result;
}

OperationResult deleteKeyForSelection(const Context& context, const EditorAnimationState& state, std::uint64_t selectedObjectId, bool logToScript)
{
    if (selectedObjectId == 0) {
        return invalidSelectionResult("Select an object to delete a key", state);
    }

    const SceneObject* object = context.findObject(selectedObjectId);
    if (object == nullptr) {
        return invalidSelectionResult("Selected object is no longer available", state);
    }

    if (!object->hasTransformKeyframe(state.currentFrame)) {
        return invalidSelectionResult(QString("No key at frame %1").arg(state.currentFrame), state);
    }

    const EditorAnimationController::SceneMutationResult mutation =
        EditorAnimationController::deleteKeyframe(context.sceneSnapshot(), selectedObjectId, state.currentFrame);
    if (!mutation.success) {
        return invalidSelectionResult("Delete key failed", state);
    }

    OperationResult result;
    result.success = true;
    result.sceneChanged = true;
    result.focusObjectId = selectedObjectId;
    result.scene = mutation.scene;
    result.state = EditorAnimationController::setCurrentFrame(state, mutation.currentFrame);
    result.statusMessage = QString("Deleted key at frame %1").arg(result.state.currentFrame);
    if (logToScript) {
        const QString objectName = objectDisplayName(*object);
        result.commandLine = QString("cutKey %1 -t %2;").arg(objectName).arg(result.state.currentFrame);
        result.resultLine = QString("// Result: deleted key on %1 at frame %2 //").arg(objectName).arg(result.state.currentFrame);
    }
    return result;
}

OperationResult duplicateCurrentKeyForSelection(const Context& context, const EditorAnimationState& state, std::uint64_t selectedObjectId, bool logToScript)
{
    return duplicateKeyframeForSelection(
        context,
        state,
        selectedObjectId,
        state.currentFrame,
        state.currentFrame + 1,
        logToScript);
}

OperationResult duplicateKeyframeForSelection(
    const Context& context,
    const EditorAnimationState& state,
    std::uint64_t selectedObjectId,
    int sourceFrame,
    int targetFrame,
    bool logToScript)
{
    if (selectedObjectId == 0) {
        return invalidSelectionResult("Select an object to duplicate a key", state);
    }

    const SceneObject* object = context.findObject(selectedObjectId);
    if (object == nullptr) {
        return invalidSelectionResult("Selected object is no longer available", state);
    }

    if (!object->hasTransformKeyframe(sourceFrame)) {
        return invalidSelectionResult(QString("No key at frame %1 to duplicate").arg(sourceFrame), state);
    }

    const EditorAnimationController::SceneMutationResult mutation =
        EditorAnimationController::duplicateKeyframe(context.sceneSnapshot(), selectedObjectId, sourceFrame, targetFrame);
    if (!mutation.success) {
        return invalidSelectionResult("Duplicate key failed", state);
    }

    OperationResult result;
    result.success = true;
    result.sceneChanged = true;
    result.focusObjectId = selectedObjectId;
    result.scene = mutation.scene;
    result.state = EditorAnimationController::setCurrentFrame(state, mutation.currentFrame);
    result.statusMessage = QString("Duplicated key to frame %1").arg(targetFrame);
    if (logToScript) {
        const QString objectName = objectDisplayName(*object);
        result.commandLine = QString("copyKey %1 -t %2 -to %3;").arg(objectName).arg(sourceFrame).arg(targetFrame);
        result.resultLine = QString("// Result: copied key on %1 from frame %2 to %3 //").arg(objectName).arg(sourceFrame).arg(targetFrame);
    }
    return result;
}

OperationResult shiftSelectedObjectKeyframes(const Context& context, const EditorAnimationState& state, std::uint64_t selectedObjectId, int frameDelta, bool logToScript)
{
    if (selectedObjectId == 0) {
        return invalidSelectionResult("Select an object to shift keys", state);
    }

    const SceneObject* object = context.findObject(selectedObjectId);
    if (object == nullptr) {
        return invalidSelectionResult("Selected object is no longer available", state);
    }

    if (object->transformKeyframes().isEmpty()) {
        return invalidSelectionResult("Selected object has no keys to shift", state);
    }

    const EditorAnimationController::SceneMutationResult mutation =
        EditorAnimationController::shiftKeyframes(context.sceneSnapshot(), selectedObjectId, frameDelta);
    if (!mutation.success) {
        return invalidSelectionResult("Shift keys failed", state);
    }

    OperationResult result;
    result.success = true;
    result.sceneChanged = true;
    result.focusObjectId = selectedObjectId;
    result.scene = mutation.scene;
    result.state = EditorAnimationController::setCurrentFrame(state, mutation.currentFrame);
    result.statusMessage = QString("Shifted keys by %1").arg(frameDelta);
    if (logToScript) {
        const QString objectName = objectDisplayName(*object);
        result.commandLine = QString("shiftKey %1 -by %2;").arg(objectName).arg(frameDelta);
        result.resultLine = QString("// Result: shifted keys on %1 by %2 //").arg(objectName).arg(frameDelta);
    }
    return result;
}

OperationResult setAutoKeyEnabled(const EditorAnimationState& state, bool enabled, bool logToScript)
{
    OperationResult result;
    result.success = true;
    result.state = EditorAnimationController::setAutoKeyEnabled(state, enabled);
    result.statusMessage = enabled ? "Auto Key enabled" : "Auto Key disabled";
    if (logToScript) {
        result.commandLine = QString("autoKeyframe -state %1;").arg(enabled ? "on" : "off");
        result.resultLine = QString("// Result: auto key %1 //").arg(enabled ? "on" : "off");
    }
    return result;
}

OperationResult setPlaybackRange(const EditorAnimationState& state, int startFrame, int endFrame, bool logToScript)
{
    OperationResult result;
    result.success = true;
    result.state = EditorAnimationController::setPlaybackRange(state, startFrame, endFrame);
    if (logToScript) {
        result.commandLine = QString("playbackOptions -min %1 -max %2;").arg(result.state.playbackStartFrame).arg(result.state.playbackEndFrame);
        result.resultLine = QString("// Result: playback range %1 to %2 //").arg(result.state.playbackStartFrame).arg(result.state.playbackEndFrame);
    }
    return result;
}

OperationResult setPlaybackState(const EditorAnimationState& state, bool playing, bool logToScript)
{
    OperationResult result;
    result.success = true;
    result.state = EditorAnimationController::setPlaybackState(state, playing);
    result.statusMessage = playing ? "Playback started" : "Playback stopped";
    if (logToScript) {
        result.commandLine = playing ? "play -state on;" : "play -state off;";
        result.resultLine = playing ? "// Result: playback started //" : "// Result: playback stopped //";
    }
    return result;
}

OperationResult stepFrame(const EditorAnimationState& state, int delta)
{
    OperationResult result;
    result.success = true;
    result.state = EditorAnimationController::stepFrame(state, delta);
    result.commandLine = QString("currentTime %1;").arg(result.state.currentFrame);
    result.resultLine = QString("// Result: current frame %1 //").arg(result.state.currentFrame);
    result.statusMessage = QString("Current frame: %1").arg(result.state.currentFrame);
    return result;
}

OperationResult jumpToSelectedObjectKeyframe(const Context& context, const EditorAnimationState& state, std::uint64_t selectedObjectId, bool forward, bool logToScript)
{
    if (selectedObjectId == 0) {
        return invalidSelectionResult("Select an object to jump keys", state);
    }

    const SceneObject* object = context.findObject(selectedObjectId);
    if (object == nullptr) {
        return invalidSelectionResult("Selected object is no longer available", state);
    }

    const int targetFrame = EditorAnimationController::jumpToKeyframe(
        context.sceneSnapshot(),
        selectedObjectId,
        state.currentFrame,
        forward);
    if (targetFrame == state.currentFrame) {
        return invalidSelectionResult(forward ? "No next key" : "No previous key", state);
    }

    OperationResult result;
    result.success = true;
    result.state = EditorAnimationController::setCurrentFrame(state, targetFrame);
    result.statusMessage = QString("Jumped to frame %1").arg(targetFrame);
    if (logToScript) {
        result.commentLine = QString("%1 key on %2 -> frame %3")
            .arg(forward ? "next" : "previous", objectDisplayName(*object))
            .arg(targetFrame);
    }
    return result;
}

OperationResult togglePlayback(const EditorAnimationState& state)
{
    return setPlaybackState(state, !state.playing, true);
}

OperationResult advancePlayback(const EditorAnimationState& state)
{
    OperationResult result;
    result.success = true;
    result.state = EditorAnimationController::advancePlayback(state);
    return result;
}
}
