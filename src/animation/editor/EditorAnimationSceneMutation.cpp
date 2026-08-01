#include "animation/editor/EditorAnimationSceneMutation.h"

namespace EditorAnimationSceneMutation
{
namespace
{
Result failedResult(const QString& errorMessage)
{
    Result result;
    result.errorMessage = errorMessage;
    return result;
}
}

Result setKeyframe(const Scene& scene, SceneObject::Id objectId, int frame)
{
    if (objectId == 0 || !scene.contains(objectId)) {
        return failedResult("Invalid object for keyframe.");
    }

    Result result;
    result.scene = scene;
    result.success = result.scene.setObjectKeyframe(objectId, frame);
    result.currentFrame = result.scene.currentFrame();
    if (!result.success) {
        result.errorMessage = "Failed to set keyframe.";
    }
    return result;
}

Result deleteKeyframe(const Scene& scene, SceneObject::Id objectId, int frame)
{
    if (objectId == 0 || !scene.contains(objectId)) {
        return failedResult("Invalid object for keyframe deletion.");
    }

    Result result;
    result.scene = scene;
    result.success = result.scene.removeObjectKeyframe(objectId, frame);
    result.currentFrame = result.scene.currentFrame();
    if (!result.success) {
        result.errorMessage = "Failed to delete keyframe.";
    }
    return result;
}

Result deleteKeyframeRange(const Scene& scene, SceneObject::Id objectId, int startFrame, int endFrame)
{
    if (objectId == 0 || !scene.contains(objectId)) {
        return failedResult("Invalid object for keyframe range deletion.");
    }

    Result result;
    result.scene = scene;
    result.success = result.scene.removeObjectKeyframesInRange(objectId, startFrame, endFrame);
    result.currentFrame = result.scene.currentFrame();
    if (!result.success) {
        result.errorMessage = "Failed to delete keyframe range.";
    }
    return result;
}

Result duplicateKeyframe(const Scene& scene, SceneObject::Id objectId, int sourceFrame, int targetFrame)
{
    if (objectId == 0 || !scene.contains(objectId)) {
        return failedResult("Invalid object for keyframe duplication.");
    }

    Result result;
    result.scene = scene;
    result.success = result.scene.duplicateObjectKeyframe(objectId, sourceFrame, targetFrame);
    result.currentFrame = result.scene.currentFrame();
    if (!result.success) {
        result.errorMessage = "Failed to duplicate keyframe.";
    }
    return result;
}

Result shiftKeyframes(const Scene& scene, SceneObject::Id objectId, int frameDelta)
{
    if (objectId == 0 || !scene.contains(objectId)) {
        return failedResult("Invalid object for keyframe shifting.");
    }

    Result result;
    result.scene = scene;
    result.success = result.scene.offsetObjectKeyframes(objectId, frameDelta);
    result.currentFrame = result.scene.currentFrame();
    if (!result.success) {
        result.errorMessage = "Failed to shift keyframes.";
    }
    return result;
}

Result shiftKeyframeRange(const Scene& scene, SceneObject::Id objectId, int startFrame, int endFrame, int frameDelta)
{
    if (objectId == 0 || !scene.contains(objectId)) {
        return failedResult("Invalid object for keyframe range shifting.");
    }

    Result result;
    result.scene = scene;
    result.success = result.scene.offsetObjectKeyframesInRange(objectId, startFrame, endFrame, frameDelta);
    result.currentFrame = result.scene.currentFrame();
    if (!result.success) {
        result.errorMessage = "Failed to shift keyframe range.";
    }
    return result;
}

Result retimeSceneKeyframes(const Scene& scene, double scaleFactor)
{
    Result result;
    result.scene = scene;
    result.success = result.scene.scaleAllObjectKeyframes(scaleFactor);
    result.currentFrame = result.scene.currentFrame();
    if (!result.success) {
        result.errorMessage = "Failed to retime keyframes.";
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
