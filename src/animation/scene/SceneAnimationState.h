#pragma once

#include "scene/SceneObject.h"
#include "scene/Transform.h"

class SceneAnimationState
{
public:
    void clear();

    int currentFrame() const;
    void setCurrentFrame(int frame);

    bool setLocalTransform(SceneObject& object, const Transform& transform, bool autoKeyEnabled);
    bool setObjectKeyframe(SceneObject& object, int frame);
    bool removeObjectKeyframe(SceneObject& object, int frame);
    bool removeObjectKeyframesInRange(SceneObject& object, int startFrame, int endFrame);
    bool duplicateObjectKeyframe(SceneObject& object, int sourceFrame, int targetFrame);
    bool offsetObjectKeyframes(SceneObject& object, int frameDelta);
    bool offsetObjectKeyframesInRange(SceneObject& object, int startFrame, int endFrame, int frameDelta);
    int nextObjectKeyframe(const SceneObject& object, int frame) const;
    int previousObjectKeyframe(const SceneObject& object, int frame) const;
    Transform evaluateObjectTransformAtCurrentFrame(const SceneObject& object) const;

private:
    Transform evaluateObjectTransformAtFrame(const SceneObject& object, int frame) const;

    int currentFrame_ = 0;
};
