#include "animation/scene/SceneAnimationState.h"

namespace
{
Transform interpolateTransform(const Transform& a, const Transform& b, float t)
{
    Transform result;
    result.translation = a.translation * (1.0f - t) + b.translation * t;
    result.rotation = QQuaternion::slerp(a.rotation, b.rotation, t);
    result.scale = a.scale * (1.0f - t) + b.scale * t;
    return result;
}
}

void SceneAnimationState::clear()
{
    currentFrame_ = 0;
}

int SceneAnimationState::currentFrame() const
{
    return currentFrame_;
}

void SceneAnimationState::setCurrentFrame(int frame)
{
    currentFrame_ = frame;
}

bool SceneAnimationState::setLocalTransform(SceneObject& object, const Transform& transform, bool autoKeyEnabled)
{
    if (object.hasAnimation()) {
        object.setTransformKeyframe(currentFrame_, transform);
    } else if (autoKeyEnabled && currentFrame_ != 0) {
        object.setTransformKeyframe(0, object.authoredTransform());
        object.setTransformKeyframe(currentFrame_, transform);
    } else {
        object.setAuthoredTransform(transform);
    }

    object.setLocalTransform(transform);
    return true;
}

bool SceneAnimationState::setObjectKeyframe(SceneObject& object, int frame)
{
    object.setTransformKeyframe(frame, object.localTransform());
    currentFrame_ = frame;
    return true;
}

bool SceneAnimationState::removeObjectKeyframe(SceneObject& object, int frame)
{
    return object.removeTransformKeyframe(frame);
}

bool SceneAnimationState::removeObjectKeyframesInRange(SceneObject& object, int startFrame, int endFrame)
{
    return object.removeTransformKeyframesInRange(startFrame, endFrame);
}

bool SceneAnimationState::duplicateObjectKeyframe(SceneObject& object, int sourceFrame, int targetFrame)
{
    if (!object.duplicateTransformKeyframe(sourceFrame, targetFrame)) {
        return false;
    }

    currentFrame_ = targetFrame;
    return true;
}

bool SceneAnimationState::offsetObjectKeyframes(SceneObject& object, int frameDelta)
{
    if (!object.offsetAllTransformKeyframes(frameDelta)) {
        return false;
    }

    currentFrame_ += frameDelta;
    return true;
}

bool SceneAnimationState::offsetObjectKeyframesInRange(SceneObject& object, int startFrame, int endFrame, int frameDelta)
{
    if (!object.offsetTransformKeyframesInRange(startFrame, endFrame, frameDelta)) {
        return false;
    }

    if (currentFrame_ >= startFrame && currentFrame_ <= endFrame) {
        currentFrame_ += frameDelta;
    }
    return true;
}

int SceneAnimationState::nextObjectKeyframe(const SceneObject& object, int frame) const
{
    return object.nextTransformKeyframeAfter(frame);
}

int SceneAnimationState::previousObjectKeyframe(const SceneObject& object, int frame) const
{
    return object.previousTransformKeyframeBefore(frame);
}

Transform SceneAnimationState::evaluateObjectTransformAtCurrentFrame(const SceneObject& object) const
{
    return evaluateObjectTransformAtFrame(object, currentFrame_);
}

Transform SceneAnimationState::evaluateObjectTransformAtFrame(const SceneObject& object, int frame) const
{
    const TransformKeyframeTrack& keyframes = object.transformKeyframes();
    if (keyframes.isEmpty()) {
        return object.authoredTransform();
    }

    if (frame <= keyframes.first().frame) {
        return keyframes.first().transform;
    }

    if (frame >= keyframes.last().frame) {
        return keyframes.last().transform;
    }

    for (int index = 0; index < keyframes.size() - 1; ++index) {
        const TransformKeyframe& a = keyframes.at(index);
        const TransformKeyframe& b = keyframes.at(index + 1);
        if (frame < a.frame || frame > b.frame) {
            continue;
        }

        if (a.frame == b.frame) {
            return b.transform;
        }

        const float t = static_cast<float>(frame - a.frame) / static_cast<float>(b.frame - a.frame);
        return interpolateTransform(a.transform, b.transform, t);
    }

    return object.authoredTransform();
}
