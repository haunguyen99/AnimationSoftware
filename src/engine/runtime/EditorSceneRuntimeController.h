#pragma once

#include <cstdint>
#include <functional>

#include "animation/editor/EditorAnimationState.h"
#include "scene/Scene.h"

namespace EditorSceneRuntimeController
{
struct SelectionRefreshRequest
{
    std::uint64_t objectId = 0;
    bool clearSelection = false;
    bool syncOutliner = true;
};

struct ApplySceneRequest
{
    Scene scene;
    bool recordUndo = false;
    bool applyCurrentFrame = false;
    int currentFrame = 0;
    bool applyAnimationState = false;
    EditorAnimationState animationState;
    SelectionRefreshRequest selection;
    bool frameEntireScene = false;
};

struct Context
{
    std::function<void()> recordUndoState;
    std::function<EditorAnimationState()> animationState;
    std::function<void(const EditorAnimationState&)> applyAnimationState;
    std::function<void(const Scene&)> replaceScene;
    std::function<void()> refreshScenePanels;
    std::function<bool(std::uint64_t)> containsObject;
    std::function<void(std::uint64_t, bool)> selectObject;
    std::function<void()> clearInspector;
    std::function<void()> frameScene;
};

void refreshSelection(const Context& context, const SelectionRefreshRequest& request);
void applyScene(const Context& context, const ApplySceneRequest& request);
}
