#include "animation/scene/SceneAnimationState.h"

#include "animation/data/CurveInterpolator.h"

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

    const float f = static_cast<float>(frame);

    // Build a per-channel key array and evaluate via CurveInterpolator.
    auto buildKeys = [&keyframes](auto valueFn) {
        QVector<CurveInterpolator::CurveKey> keys;
        keys.reserve(keyframes.size());
        for (const TransformKeyframe& kf : keyframes) {
            keys.append({ static_cast<float>(kf.frame), valueFn(kf), kf.tangent });
        }
        return keys;
    };

    const auto txKeys = buildKeys([](const TransformKeyframe& kf) { return static_cast<double>(kf.transform.translation.x()); });
    const auto tyKeys = buildKeys([](const TransformKeyframe& kf) { return static_cast<double>(kf.transform.translation.y()); });
    const auto tzKeys = buildKeys([](const TransformKeyframe& kf) { return static_cast<double>(kf.transform.translation.z()); });
    const auto rxKeys = buildKeys([](const TransformKeyframe& kf) { return static_cast<double>(kf.transform.rotation.toEulerAngles().x()); });
    const auto ryKeys = buildKeys([](const TransformKeyframe& kf) { return static_cast<double>(kf.transform.rotation.toEulerAngles().y()); });
    const auto rzKeys = buildKeys([](const TransformKeyframe& kf) { return static_cast<double>(kf.transform.rotation.toEulerAngles().z()); });
    const auto sxKeys = buildKeys([](const TransformKeyframe& kf) { return static_cast<double>(kf.transform.scale.x()); });
    const auto syKeys = buildKeys([](const TransformKeyframe& kf) { return static_cast<double>(kf.transform.scale.y()); });
    const auto szKeys = buildKeys([](const TransformKeyframe& kf) { return static_cast<double>(kf.transform.scale.z()); });

    Transform result;
    result.translation.setX(static_cast<float>(CurveInterpolator::evaluateChannel(txKeys, f)));
    result.translation.setY(static_cast<float>(CurveInterpolator::evaluateChannel(tyKeys, f)));
    result.translation.setZ(static_cast<float>(CurveInterpolator::evaluateChannel(tzKeys, f)));
    result.rotation = QQuaternion::fromEulerAngles(
        static_cast<float>(CurveInterpolator::evaluateChannel(rxKeys, f)),
        static_cast<float>(CurveInterpolator::evaluateChannel(ryKeys, f)),
        static_cast<float>(CurveInterpolator::evaluateChannel(rzKeys, f)));
    result.scale.setX(static_cast<float>(CurveInterpolator::evaluateChannel(sxKeys, f)));
    result.scale.setY(static_cast<float>(CurveInterpolator::evaluateChannel(syKeys, f)));
    result.scale.setZ(static_cast<float>(CurveInterpolator::evaluateChannel(szKeys, f)));
    return result;
}
