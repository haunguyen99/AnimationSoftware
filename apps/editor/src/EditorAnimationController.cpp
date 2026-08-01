#include "EditorAnimationController.h"

#include <QtGlobal>

#include <utility>

namespace EditorAnimationController
{
namespace
{
void normalizeRange(int& startFrame, int& endFrame)
{
    if (startFrame > endFrame) {
        std::swap(startFrame, endFrame);
    }
}

void clampPlaybackToVisible(EditorAnimationState& state)
{
    state.playbackStartFrame = qBound(state.visibleStartFrame, state.playbackStartFrame, state.visibleEndFrame);
    state.playbackEndFrame = qBound(state.visibleStartFrame, state.playbackEndFrame, state.visibleEndFrame);
    if (state.playbackStartFrame > state.playbackEndFrame) {
        std::swap(state.playbackStartFrame, state.playbackEndFrame);
    }
}

void clampSelectedRangeToPlayback(EditorAnimationState& state)
{
    if (!state.hasSelectedRange) {
        return;
    }

    state.selectedRangeStartFrame =
        qBound(state.playbackStartFrame, state.selectedRangeStartFrame, state.playbackEndFrame);
    state.selectedRangeEndFrame =
        qBound(state.playbackStartFrame, state.selectedRangeEndFrame, state.playbackEndFrame);
    if (state.selectedRangeStartFrame > state.selectedRangeEndFrame) {
        std::swap(state.selectedRangeStartFrame, state.selectedRangeEndFrame);
    }
}
}

EditorAnimationState setCurrentFrame(const EditorAnimationState& state, int frame)
{
    EditorAnimationState updatedState = state;
    updatedState.currentFrame = qBound(state.playbackStartFrame, frame, state.playbackEndFrame);
    return updatedState;
}

EditorAnimationState setVisibleFrameRange(const EditorAnimationState& state, int startFrame, int endFrame)
{
    EditorAnimationState updatedState = state;
    normalizeRange(startFrame, endFrame);
    updatedState.visibleStartFrame = startFrame;
    updatedState.visibleEndFrame = qMax(startFrame + 1, endFrame);
    clampPlaybackToVisible(updatedState);
    updatedState.currentFrame = qBound(updatedState.playbackStartFrame, updatedState.currentFrame, updatedState.playbackEndFrame);
    clampSelectedRangeToPlayback(updatedState);
    return updatedState;
}

EditorAnimationState setPlaybackRange(const EditorAnimationState& state, int startFrame, int endFrame)
{
    EditorAnimationState updatedState = state;
    normalizeRange(startFrame, endFrame);

    updatedState.playbackStartFrame = qBound(state.visibleStartFrame, startFrame, state.visibleEndFrame);
    updatedState.playbackEndFrame = qBound(state.visibleStartFrame, endFrame, state.visibleEndFrame);
    normalizeRange(updatedState.playbackStartFrame, updatedState.playbackEndFrame);
    updatedState.currentFrame =
        qBound(updatedState.playbackStartFrame, updatedState.currentFrame, updatedState.playbackEndFrame);
    clampSelectedRangeToPlayback(updatedState);
    return updatedState;
}

EditorAnimationState setFramesPerSecond(const EditorAnimationState& state, int framesPerSecond)
{
    EditorAnimationState updatedState = state;
    updatedState.framesPerSecond = qBound(1, framesPerSecond, 240);
    return updatedState;
}

EditorAnimationState setSelectedFrameRange(const EditorAnimationState& state, int startFrame, int endFrame)
{
    EditorAnimationState updatedState = state;
    if (startFrame > endFrame) {
        std::swap(startFrame, endFrame);
    }

    updatedState.selectedRangeStartFrame = qBound(state.playbackStartFrame, startFrame, state.playbackEndFrame);
    updatedState.selectedRangeEndFrame = qBound(state.playbackStartFrame, endFrame, state.playbackEndFrame);
    updatedState.hasSelectedRange = true;
    return updatedState;
}

EditorAnimationState clearSelectedFrameRange(const EditorAnimationState& state)
{
    EditorAnimationState updatedState = state;
    updatedState.hasSelectedRange = false;
    return updatedState;
}

EditorAnimationState stepFrame(const EditorAnimationState& state, int delta)
{
    return setCurrentFrame(state, state.currentFrame + delta);
}

EditorAnimationState advancePlayback(const EditorAnimationState& state)
{
    return setCurrentFrame(
        state,
        state.currentFrame >= state.playbackEndFrame ? state.playbackStartFrame : state.currentFrame + 1);
}

EditorAnimationState setAutoKeyEnabled(const EditorAnimationState& state, bool enabled)
{
    EditorAnimationState updatedState = state;
    updatedState.autoKeyEnabled = enabled;
    return updatedState;
}

EditorAnimationState setPlaybackState(const EditorAnimationState& state, bool playing)
{
    EditorAnimationState updatedState = state;
    updatedState.playing = playing;
    return updatedState;
}
}
