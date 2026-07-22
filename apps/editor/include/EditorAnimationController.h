#pragma once

#include <QString>
#include <QVector>

#include <functional>

#include "ScriptCommandSystem.h"
#include "scene/Scene.h"

struct EditorAnimationState
{
    int currentFrame = 0;
    int playbackStartFrame = 0;
    int playbackEndFrame = 24;
    bool autoKeyEnabled = false;
    bool playing = false;
};

struct EditorAnimationTimelineViewModel
{
    int playbackStartFrame = 0;
    int playbackEndFrame = 24;
    int currentFrame = 0;
    QVector<int> keyframes;
    QString objectName = "No selection";
    QString statusText = "No selection";
    QString statusStyle = "color: #bdbdbd;";
    QString setKeyStyle;
    QString deleteKeyStyle;
    QString autoKeyStyle;
    QString playPauseText = ">";
    bool hasSelection = false;
    bool currentFrameKeyed = false;
    bool hasAnyKeys = false;
    bool autoKeyEnabled = false;
    bool playing = false;
};

struct EditorAnimationScriptBindings
{
    std::function<SceneObject::Id(const QString&)> findObjectIdByName;
    std::function<void(SceneObject::Id)> selectObjectById;
    std::function<Scene()> sceneSnapshot;
    std::function<EditorAnimationState()> animationState;
    std::function<void(const Scene&, SceneObject::Id, int)> applySceneMutation;
    std::function<void(const EditorAnimationState&)> applyAnimationState;
};

namespace EditorAnimationController
{
struct SceneMutationResult
{
    bool success = false;
    Scene scene;
    int currentFrame = 0;
    QString errorMessage;
};

EditorAnimationState setCurrentFrame(const EditorAnimationState& state, int frame);
EditorAnimationState setPlaybackRange(const EditorAnimationState& state, int startFrame, int endFrame);
EditorAnimationState stepFrame(const EditorAnimationState& state, int delta);
EditorAnimationState advancePlayback(const EditorAnimationState& state);
EditorAnimationState setAutoKeyEnabled(const EditorAnimationState& state, bool enabled);
EditorAnimationState setPlaybackState(const EditorAnimationState& state, bool playing);
EditorAnimationTimelineViewModel buildTimelineViewModel(
    const Scene& scene,
    SceneObject::Id selectedObjectId,
    const EditorAnimationState& state);
void bindScriptCommands(ScriptCommandContext& context, const EditorAnimationScriptBindings& bindings);

SceneMutationResult setKeyframe(const Scene& scene, SceneObject::Id objectId, int frame);
SceneMutationResult deleteKeyframe(const Scene& scene, SceneObject::Id objectId, int frame);
SceneMutationResult duplicateKeyframe(const Scene& scene, SceneObject::Id objectId, int sourceFrame, int targetFrame);
SceneMutationResult shiftKeyframes(const Scene& scene, SceneObject::Id objectId, int frameDelta);
int jumpToKeyframe(const Scene& scene, SceneObject::Id objectId, int frame, bool forward);
}
