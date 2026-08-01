#pragma once

#include "animation/data/TransformKeyframeTrack.h"
#include "scene/Transform.h"

class ObjectAnimationState
{
public:
    bool hasAnimation() const;
    bool hasTransformKeyframe(int frame) const;
    const TransformKeyframeTrack& transformKeyframes() const;
    void setTransformKeyframes(const TransformKeyframeTrack& keyframes);
    void setTransformKeyframe(int frame, const Transform& transform);
    bool removeTransformKeyframe(int frame);
    bool removeTransformKeyframesInRange(int startFrame, int endFrame);
    bool duplicateTransformKeyframe(int sourceFrame, int targetFrame);
    bool offsetAllTransformKeyframes(int frameDelta);
    bool offsetTransformKeyframesInRange(int startFrame, int endFrame, int frameDelta);
    bool scaleAllTransformKeyframes(double scaleFactor);
    int nextTransformKeyframeAfter(int frame) const;
    int previousTransformKeyframeBefore(int frame) const;

private:
    TransformKeyframeTrack transformKeyframes_;
};
