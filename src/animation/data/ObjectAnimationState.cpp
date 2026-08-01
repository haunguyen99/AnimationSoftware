#include "animation/data/ObjectAnimationState.h"

#include <algorithm>
#include <limits>

namespace
{
bool keyframeLessThan(const TransformKeyframe& lhs, const TransformKeyframe& rhs)
{
    return lhs.frame < rhs.frame;
}
}

bool ObjectAnimationState::hasAnimation() const
{
    return !transformKeyframes_.isEmpty();
}

bool ObjectAnimationState::hasTransformKeyframe(int frame) const
{
    const auto it = std::find_if(transformKeyframes_.cbegin(), transformKeyframes_.cend(), [frame](const TransformKeyframe& keyframe) {
        return keyframe.frame == frame;
    });
    return it != transformKeyframes_.cend();
}

const TransformKeyframeTrack& ObjectAnimationState::transformKeyframes() const
{
    return transformKeyframes_;
}

void ObjectAnimationState::setTransformKeyframes(const TransformKeyframeTrack& keyframes)
{
    transformKeyframes_ = keyframes;
    std::sort(transformKeyframes_.begin(), transformKeyframes_.end(), keyframeLessThan);
}

void ObjectAnimationState::setTransformKeyframe(int frame, const Transform& transform)
{
    const auto it = std::find_if(transformKeyframes_.begin(), transformKeyframes_.end(), [frame](const TransformKeyframe& keyframe) {
        return keyframe.frame == frame;
    });
    if (it != transformKeyframes_.end()) {
        it->transform = transform;
        return;
    }

    transformKeyframes_.append({ frame, transform });
    std::sort(transformKeyframes_.begin(), transformKeyframes_.end(), keyframeLessThan);
}

bool ObjectAnimationState::removeTransformKeyframe(int frame)
{
    const auto it = std::find_if(transformKeyframes_.begin(), transformKeyframes_.end(), [frame](const TransformKeyframe& keyframe) {
        return keyframe.frame == frame;
    });
    if (it == transformKeyframes_.end()) {
        return false;
    }

    transformKeyframes_.erase(it);
    return true;
}

bool ObjectAnimationState::removeTransformKeyframesInRange(int startFrame, int endFrame)
{
    if (startFrame > endFrame) {
        std::swap(startFrame, endFrame);
    }

    const auto newEnd = std::remove_if(
        transformKeyframes_.begin(),
        transformKeyframes_.end(),
        [startFrame, endFrame](const TransformKeyframe& keyframe) {
            return keyframe.frame >= startFrame && keyframe.frame <= endFrame;
        });
    if (newEnd == transformKeyframes_.end()) {
        return false;
    }

    transformKeyframes_.erase(newEnd, transformKeyframes_.end());
    return true;
}

bool ObjectAnimationState::duplicateTransformKeyframe(int sourceFrame, int targetFrame)
{
    const auto it = std::find_if(transformKeyframes_.cbegin(), transformKeyframes_.cend(), [sourceFrame](const TransformKeyframe& keyframe) {
        return keyframe.frame == sourceFrame;
    });
    if (it == transformKeyframes_.cend()) {
        return false;
    }

    setTransformKeyframe(targetFrame, it->transform);
    return true;
}

bool ObjectAnimationState::offsetAllTransformKeyframes(int frameDelta)
{
    if (transformKeyframes_.isEmpty() || frameDelta == 0) {
        return !transformKeyframes_.isEmpty();
    }

    for (TransformKeyframe& keyframe : transformKeyframes_) {
        keyframe.frame += frameDelta;
    }

    std::sort(transformKeyframes_.begin(), transformKeyframes_.end(), keyframeLessThan);

    TransformKeyframeTrack mergedKeyframes;
    mergedKeyframes.reserve(transformKeyframes_.size());
    for (const TransformKeyframe& keyframe : transformKeyframes_) {
        if (!mergedKeyframes.isEmpty() && mergedKeyframes.last().frame == keyframe.frame) {
            mergedKeyframes.last().transform = keyframe.transform;
        } else {
            mergedKeyframes.append(keyframe);
        }
    }

    transformKeyframes_ = mergedKeyframes;
    return true;
}

bool ObjectAnimationState::offsetTransformKeyframesInRange(int startFrame, int endFrame, int frameDelta)
{
    if (startFrame > endFrame) {
        std::swap(startFrame, endFrame);
    }

    bool hasSelectedKeys = false;
    TransformKeyframeTrack stationaryKeyframes;
    TransformKeyframeTrack shiftedKeyframes;
    stationaryKeyframes.reserve(transformKeyframes_.size());
    shiftedKeyframes.reserve(transformKeyframes_.size());

    for (const TransformKeyframe& keyframe : transformKeyframes_) {
        if (keyframe.frame >= startFrame && keyframe.frame <= endFrame) {
            hasSelectedKeys = true;
            shiftedKeyframes.append({ keyframe.frame + frameDelta, keyframe.transform });
        } else {
            stationaryKeyframes.append(keyframe);
        }
    }

    if (!hasSelectedKeys) {
        return false;
    }

    TransformKeyframeTrack mergedKeyframes = stationaryKeyframes;
    for (const TransformKeyframe& keyframe : shiftedKeyframes) {
        const auto it = std::find_if(
            mergedKeyframes.begin(),
            mergedKeyframes.end(),
            [frame = keyframe.frame](const TransformKeyframe& existingKeyframe) {
                return existingKeyframe.frame == frame;
            });
        if (it != mergedKeyframes.end()) {
            it->transform = keyframe.transform;
        } else {
            mergedKeyframes.append(keyframe);
        }
    }

    std::sort(mergedKeyframes.begin(), mergedKeyframes.end(), keyframeLessThan);
    transformKeyframes_ = mergedKeyframes;
    return true;
}

bool ObjectAnimationState::scaleAllTransformKeyframes(double scaleFactor)
{
    if (transformKeyframes_.isEmpty()) {
        return false;
    }

    TransformKeyframeTrack scaledKeyframes;
    scaledKeyframes.reserve(transformKeyframes_.size());
    for (const TransformKeyframe& keyframe : transformKeyframes_) {
        scaledKeyframes.append({ qRound(keyframe.frame * scaleFactor), keyframe.transform });
    }

    setTransformKeyframes(scaledKeyframes);
    return true;
}

int ObjectAnimationState::nextTransformKeyframeAfter(int frame) const
{
    int bestFrame = std::numeric_limits<int>::max();
    for (const TransformKeyframe& keyframe : transformKeyframes_) {
        if (keyframe.frame > frame && keyframe.frame < bestFrame) {
            bestFrame = keyframe.frame;
        }
    }

    return bestFrame == std::numeric_limits<int>::max() ? frame : bestFrame;
}

int ObjectAnimationState::previousTransformKeyframeBefore(int frame) const
{
    int bestFrame = std::numeric_limits<int>::min();
    for (const TransformKeyframe& keyframe : transformKeyframes_) {
        if (keyframe.frame < frame && keyframe.frame > bestFrame) {
            bestFrame = keyframe.frame;
        }
    }

    return bestFrame == std::numeric_limits<int>::min() ? frame : bestFrame;
}
