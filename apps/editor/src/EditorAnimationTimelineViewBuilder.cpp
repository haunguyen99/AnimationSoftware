#include "EditorAnimationTimelineViewBuilder.h"

namespace EditorAnimationTimelineViewBuilder
{
namespace
{
QString buildRangeSelectionText(const EditorAnimationState& state)
{
    if (!state.hasSelectedRange) {
        return QString();
    }

    if (state.selectedRangeStartFrame == state.selectedRangeEndFrame) {
        return QString(" | frame %1 selected").arg(state.selectedRangeStartFrame);
    }

    return QString(" | range %1-%2 selected")
        .arg(state.selectedRangeStartFrame)
        .arg(state.selectedRangeEndFrame);
}
}

EditorAnimationTimelineViewModel build(
    const Scene& scene,
    SceneObject::Id selectedObjectId,
    const EditorAnimationState& state)
{
    EditorAnimationTimelineViewModel viewModel;
    viewModel.visibleStartFrame = state.visibleStartFrame;
    viewModel.visibleEndFrame = state.visibleEndFrame;
    viewModel.playbackStartFrame = state.playbackStartFrame;
    viewModel.playbackEndFrame = state.playbackEndFrame;
    viewModel.framesPerSecond = state.framesPerSecond;
    viewModel.currentFrame = state.currentFrame;
    viewModel.selectedRangeStartFrame = state.selectedRangeStartFrame;
    viewModel.selectedRangeEndFrame = state.selectedRangeEndFrame;
    viewModel.hasSelectedRange = state.hasSelectedRange;
    viewModel.autoKeyEnabled = state.autoKeyEnabled;
    viewModel.playing = state.playing;
    viewModel.playPauseText = state.playing ? "||" : ">";
    const QString rangeSelectionText = buildRangeSelectionText(state);
    viewModel.statusText =
        state.autoKeyEnabled ? QString("No selection%1 | Auto Key on").arg(rangeSelectionText)
                             : QString("No selection%1").arg(rangeSelectionText);
    viewModel.statusStyle = "color: #aeb6c1;";
    viewModel.autoKeyStyle = state.autoKeyEnabled
        ? "QPushButton { background-color: #6d3a3a; color: #f5f7fa; font-weight: 600; border: 1px solid #8b4d4d; }"
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
        ? "QPushButton { background-color: #8f6832; color: #f5f7fa; font-weight: 600; border: 1px solid #ab8144; }"
        : "QPushButton { background-color: #3a3f46; color: #e7ebf1; border: 1px solid #4c525b; }";
    viewModel.deleteKeyStyle = viewModel.currentFrameKeyed
        ? "QPushButton { background-color: #4b5058; color: #f5f7fa; border: 1px solid #5c636d; }"
        : QString();

    if (viewModel.currentFrameKeyed) {
        viewModel.statusText = QString("%1 | %2 keys | frame %3 keyed%4")
                                   .arg(viewModel.objectName)
                                   .arg(viewModel.keyframes.size())
                                   .arg(state.currentFrame)
                                   .arg(QString("%1%2")
                                            .arg(rangeSelectionText)
                                            .arg(state.autoKeyEnabled ? " | Auto Key on" : ""));
        viewModel.statusStyle = "color: #d6b06e; font-weight: 600;";
    } else if (viewModel.hasAnyKeys) {
        viewModel.statusText = QString("%1 | %2 keys | frame %3 has no key%4")
                                   .arg(viewModel.objectName)
                                   .arg(viewModel.keyframes.size())
                                   .arg(state.currentFrame)
                                   .arg(QString("%1%2")
                                            .arg(rangeSelectionText)
                                            .arg(state.autoKeyEnabled ? " | Auto Key on" : ""));
        viewModel.statusStyle = "color: #8eb3cf;";
    } else {
        viewModel.statusText = QString("%1 | no keys yet%2")
                                   .arg(viewModel.objectName)
                                   .arg(QString("%1%2")
                                            .arg(rangeSelectionText)
                                            .arg(state.autoKeyEnabled ? " | Auto Key on" : ""));
        viewModel.statusStyle = "color: #aeb6c1;";
    }

    return viewModel;
}
}
