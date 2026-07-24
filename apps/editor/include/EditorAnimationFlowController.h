#pragma once

#include <cstdint>
#include <functional>

#include "EditorAnimationController.h"
#include "scene/Scene.h"

namespace EditorAnimationFlowController
{
struct Context
{
    std::function<const SceneObject*(std::uint64_t)> findObject;
    std::function<Scene()> sceneSnapshot;
};

struct OperationResult
{
    bool success = false;
    bool sceneChanged = false;
    std::uint64_t focusObjectId = 0;
    Scene scene;
    EditorAnimationState state;
    QString errorMessage;
    QString statusMessage;
    QString commentLine;
    QString commandLine;
    QString resultLine;
};

OperationResult setCurrentFrame(const EditorAnimationState& state, int frame, bool logToScript);
OperationResult setKeyForSelection(const Context& context, const EditorAnimationState& state, std::uint64_t selectedObjectId, bool logToScript);
OperationResult deleteKeyForSelection(const Context& context, const EditorAnimationState& state, std::uint64_t selectedObjectId, bool logToScript);
OperationResult duplicateCurrentKeyForSelection(const Context& context, const EditorAnimationState& state, std::uint64_t selectedObjectId, bool logToScript);
OperationResult shiftSelectedObjectKeyframes(const Context& context, const EditorAnimationState& state, std::uint64_t selectedObjectId, int frameDelta, bool logToScript);
OperationResult setAutoKeyEnabled(const EditorAnimationState& state, bool enabled, bool logToScript);
OperationResult setPlaybackRange(const EditorAnimationState& state, int startFrame, int endFrame, bool logToScript);
OperationResult stepFrame(const EditorAnimationState& state, int delta);
OperationResult jumpToSelectedObjectKeyframe(const Context& context, const EditorAnimationState& state, std::uint64_t selectedObjectId, bool forward, bool logToScript);
OperationResult togglePlayback(const EditorAnimationState& state);
OperationResult advancePlayback(const EditorAnimationState& state);
}
