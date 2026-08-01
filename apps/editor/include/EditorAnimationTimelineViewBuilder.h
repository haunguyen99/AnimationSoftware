#pragma once

#include "EditorAnimationTimelineViewModel.h"
#include "animation/editor/EditorAnimationState.h"
#include "scene/Scene.h"

namespace EditorAnimationTimelineViewBuilder
{
EditorAnimationTimelineViewModel build(
    const Scene& scene,
    SceneObject::Id selectedObjectId,
    const EditorAnimationState& state);
}
