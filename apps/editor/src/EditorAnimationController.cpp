#include "EditorAnimationController.h"

#include <QtGlobal>

#include <utility>

namespace EditorAnimationController
{
namespace
{
SceneMutationResult failedResult(const QString& errorMessage)
{
    SceneMutationResult result;
    result.errorMessage = errorMessage;
    return result;
}
}

EditorAnimationState setCurrentFrame(const EditorAnimationState& state, int frame)
{
    EditorAnimationState updatedState = state;
    updatedState.currentFrame = qBound(state.playbackStartFrame, frame, state.playbackEndFrame);
    return updatedState;
}

EditorAnimationState setPlaybackRange(const EditorAnimationState& state, int startFrame, int endFrame)
{
    EditorAnimationState updatedState = state;
    if (startFrame > endFrame) {
        std::swap(startFrame, endFrame);
    }

    updatedState.playbackStartFrame = startFrame;
    updatedState.playbackEndFrame = endFrame;
    updatedState.currentFrame = qBound(startFrame, updatedState.currentFrame, endFrame);
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

EditorAnimationTimelineViewModel buildTimelineViewModel(
    const Scene& scene,
    SceneObject::Id selectedObjectId,
    const EditorAnimationState& state)
{
    EditorAnimationTimelineViewModel viewModel;
    viewModel.playbackStartFrame = state.playbackStartFrame;
    viewModel.playbackEndFrame = state.playbackEndFrame;
    viewModel.currentFrame = state.currentFrame;
    viewModel.autoKeyEnabled = state.autoKeyEnabled;
    viewModel.playing = state.playing;
    viewModel.playPauseText = state.playing ? "||" : ">";
    viewModel.statusText = state.autoKeyEnabled ? "No selection | Auto Key on" : "No selection";
    viewModel.statusStyle = "color: #bdbdbd;";
    viewModel.autoKeyStyle = state.autoKeyEnabled
        ? "QPushButton { background-color: #8f2424; color: white; font-weight: 600; }"
        : QString();

    if (selectedObjectId == 0) {
        return viewModel;
    }

    const SceneObject* object = scene.findObject(selectedObjectId);
    if (object == nullptr) {
        return viewModel;
    }

    viewModel.hasSelection = true;
    viewModel.objectName = object->name().isEmpty() ? QString("Object_%1").arg(object->id()) : object->name();
    const TransformKeyframeTrack& track = object->transformKeyframes();
    viewModel.hasAnyKeys = !track.isEmpty();
    viewModel.keyframes.reserve(track.size());
    for (const TransformKeyframe& keyframe : track) {
        viewModel.keyframes.append(keyframe.frame);
        if (keyframe.frame == state.currentFrame) {
            viewModel.currentFrameKeyed = true;
        }
    }

    viewModel.setKeyStyle = viewModel.currentFrameKeyed
        ? "QPushButton { background-color: #b86d1f; color: white; font-weight: 600; }"
        : "QPushButton { background-color: #4a4a4a; color: white; }";
    viewModel.deleteKeyStyle = viewModel.currentFrameKeyed
        ? "QPushButton { background-color: #565656; color: white; }"
        : QString();

    if (viewModel.currentFrameKeyed) {
        viewModel.statusText = QString("%1 | %2 keys | frame %3 keyed%4")
                                   .arg(viewModel.objectName)
                                   .arg(viewModel.keyframes.size())
                                   .arg(state.currentFrame)
                                   .arg(state.autoKeyEnabled ? " | Auto Key on" : "");
        viewModel.statusStyle = "color: #ffb040; font-weight: 600;";
    } else if (viewModel.hasAnyKeys) {
        viewModel.statusText = QString("%1 | %2 keys | frame %3 has no key%4")
                                   .arg(viewModel.objectName)
                                   .arg(viewModel.keyframes.size())
                                   .arg(state.currentFrame)
                                   .arg(state.autoKeyEnabled ? " | Auto Key on" : "");
        viewModel.statusStyle = "color: #9fdcff;";
    } else {
        viewModel.statusText = QString("%1 | no keys yet%2")
                                   .arg(viewModel.objectName)
                                   .arg(state.autoKeyEnabled ? " | Auto Key on" : "");
        viewModel.statusStyle = "color: #bdbdbd;";
    }

    return viewModel;
}

SceneMutationResult setKeyframe(const Scene& scene, SceneObject::Id objectId, int frame)
{
    if (objectId == 0 || !scene.contains(objectId)) {
        return failedResult("Invalid object for keyframe.");
    }

    SceneMutationResult result;
    result.scene = scene;
    result.success = result.scene.setObjectKeyframe(objectId, frame);
    result.currentFrame = result.scene.currentFrame();
    if (!result.success) {
        result.errorMessage = "Failed to set keyframe.";
    }
    return result;
}

SceneMutationResult deleteKeyframe(const Scene& scene, SceneObject::Id objectId, int frame)
{
    if (objectId == 0 || !scene.contains(objectId)) {
        return failedResult("Invalid object for keyframe deletion.");
    }

    SceneMutationResult result;
    result.scene = scene;
    result.success = result.scene.removeObjectKeyframe(objectId, frame);
    result.currentFrame = result.scene.currentFrame();
    if (!result.success) {
        result.errorMessage = "Failed to delete keyframe.";
    }
    return result;
}

SceneMutationResult duplicateKeyframe(const Scene& scene, SceneObject::Id objectId, int sourceFrame, int targetFrame)
{
    if (objectId == 0 || !scene.contains(objectId)) {
        return failedResult("Invalid object for keyframe duplication.");
    }

    SceneMutationResult result;
    result.scene = scene;
    result.success = result.scene.duplicateObjectKeyframe(objectId, sourceFrame, targetFrame);
    result.currentFrame = result.scene.currentFrame();
    if (!result.success) {
        result.errorMessage = "Failed to duplicate keyframe.";
    }
    return result;
}

SceneMutationResult shiftKeyframes(const Scene& scene, SceneObject::Id objectId, int frameDelta)
{
    if (objectId == 0 || !scene.contains(objectId)) {
        return failedResult("Invalid object for keyframe shifting.");
    }

    SceneMutationResult result;
    result.scene = scene;
    result.success = result.scene.offsetObjectKeyframes(objectId, frameDelta);
    result.currentFrame = result.scene.currentFrame();
    if (!result.success) {
        result.errorMessage = "Failed to shift keyframes.";
    }
    return result;
}

int jumpToKeyframe(const Scene& scene, SceneObject::Id objectId, int frame, bool forward)
{
    if (objectId == 0 || !scene.contains(objectId)) {
        return frame;
    }

    return forward
        ? scene.nextObjectKeyframe(objectId, frame)
        : scene.previousObjectKeyframe(objectId, frame);
}
}
