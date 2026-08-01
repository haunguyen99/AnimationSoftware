#pragma once

#include <QString>

#include "animation/editor/EditorAnimationState.h"

namespace EditorAnimationController
{
EditorAnimationState setCurrentFrame(const EditorAnimationState& state, int frame);
EditorAnimationState setVisibleFrameRange(const EditorAnimationState& state, int startFrame, int endFrame);
EditorAnimationState setPlaybackRange(const EditorAnimationState& state, int startFrame, int endFrame);
EditorAnimationState setFramesPerSecond(const EditorAnimationState& state, int framesPerSecond);
EditorAnimationState setSelectedFrameRange(const EditorAnimationState& state, int startFrame, int endFrame);
EditorAnimationState clearSelectedFrameRange(const EditorAnimationState& state);
EditorAnimationState stepFrame(const EditorAnimationState& state, int delta);
EditorAnimationState advancePlayback(const EditorAnimationState& state);
EditorAnimationState setAutoKeyEnabled(const EditorAnimationState& state, bool enabled);
EditorAnimationState setPlaybackState(const EditorAnimationState& state, bool playing);
}
