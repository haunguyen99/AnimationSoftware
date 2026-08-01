#pragma once

#include <QString>

#include "scene/Scene.h"

namespace EditorAnimationSceneMutation
{
struct Result
{
    bool success = false;
    Scene scene;
    int currentFrame = 0;
    QString errorMessage;
};

Result setKeyframe(const Scene& scene, SceneObject::Id objectId, int frame);
Result deleteKeyframe(const Scene& scene, SceneObject::Id objectId, int frame);
Result deleteKeyframeRange(const Scene& scene, SceneObject::Id objectId, int startFrame, int endFrame);
Result duplicateKeyframe(const Scene& scene, SceneObject::Id objectId, int sourceFrame, int targetFrame);
Result shiftKeyframes(const Scene& scene, SceneObject::Id objectId, int frameDelta);
Result shiftKeyframeRange(const Scene& scene, SceneObject::Id objectId, int startFrame, int endFrame, int frameDelta);
Result retimeSceneKeyframes(const Scene& scene, double scaleFactor);
int jumpToKeyframe(const Scene& scene, SceneObject::Id objectId, int frame, bool forward);
}
