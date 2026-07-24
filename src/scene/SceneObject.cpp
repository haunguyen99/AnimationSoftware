#include "scene/SceneObject.h"

#include <algorithm>
#include <limits>

namespace
{
bool keyframeLessThan(const TransformKeyframe& lhs, const TransformKeyframe& rhs)
{
    return lhs.frame < rhs.frame;
}
}

SceneObject::SceneObject(Id id)
    : id_(id)
{
}

SceneObject::Id SceneObject::id() const
{
    return id_;
}

const QString& SceneObject::name() const
{
    return name_;
}

void SceneObject::setName(const QString& name)
{
    name_ = name;
}

SceneObject::Kind SceneObject::kind() const
{
    return kind_;
}

void SceneObject::setKind(Kind kind)
{
    kind_ = kind;
}

bool SceneObject::isJoint() const
{
    return kind_ == Kind::Joint;
}

SceneObject::Id SceneObject::parentId() const
{
    return parentId_;
}

void SceneObject::setParentId(Id parentId)
{
    parentId_ = parentId;
}

const QVector<SceneObject::Id>& SceneObject::childIds() const
{
    return childIds_;
}

void SceneObject::addChildId(Id childId)
{
    childIds_.append(childId);
}

bool SceneObject::removeChildId(Id childId)
{
    const auto it = std::find(childIds_.begin(), childIds_.end(), childId);
    if (it == childIds_.end()) {
        return false;
    }

    childIds_.erase(it);
    return true;
}

void SceneObject::clearChildren()
{
    childIds_.clear();
}

const Transform& SceneObject::localTransform() const
{
    return localTransform_;
}

void SceneObject::setLocalTransform(const Transform& transform)
{
    localTransform_ = transform;
}

const Transform& SceneObject::authoredTransform() const
{
    return authoredTransform_;
}

void SceneObject::setAuthoredTransform(const Transform& transform)
{
    authoredTransform_ = transform;
}

const QQuaternion& SceneObject::jointOrientation() const
{
    return jointOrientation_;
}

void SceneObject::setJointOrientation(const QQuaternion& orientation)
{
    jointOrientation_ = orientation.normalized();
}

const Transform& SceneObject::bindPoseLocalTransform() const
{
    return bindPoseLocalTransform_;
}

void SceneObject::setBindPoseLocalTransform(const Transform& transform)
{
    bindPoseLocalTransform_ = transform;
}

bool SceneObject::hasBindPose() const
{
    return hasBindPose_;
}

void SceneObject::setHasBindPose(bool hasBindPose)
{
    hasBindPose_ = hasBindPose;
}

bool SceneObject::hasSkinBinding() const
{
    return hasSkinBinding_;
}

void SceneObject::setHasSkinBinding(bool hasSkinBinding)
{
    hasSkinBinding_ = hasSkinBinding;
}

const Transform& SceneObject::skinBindLocalTransform() const
{
    return skinBindLocalTransform_;
}

void SceneObject::setSkinBindLocalTransform(const Transform& transform)
{
    skinBindLocalTransform_ = transform;
}

const QVector<SceneObject::Id>& SceneObject::skinJointIds() const
{
    return skinJointIds_;
}

void SceneObject::setSkinJointIds(const QVector<Id>& jointIds)
{
    skinJointIds_ = jointIds;
}

const SkinWeightTable& SceneObject::skinWeights() const
{
    return skinWeights_;
}

void SceneObject::setSkinWeights(const SkinWeightTable& weights)
{
    skinWeights_ = weights;
}

void SceneObject::clearSkinBinding()
{
    hasSkinBinding_ = false;
    skinBindLocalTransform_ = Transform();
    skinJointIds_.clear();
    skinWeights_.clear();
}

bool SceneObject::hasAnimation() const
{
    return !transformKeyframes_.isEmpty();
}

bool SceneObject::hasTransformKeyframe(int frame) const
{
    const auto it = std::find_if(transformKeyframes_.cbegin(), transformKeyframes_.cend(), [frame](const TransformKeyframe& keyframe) {
        return keyframe.frame == frame;
    });
    return it != transformKeyframes_.cend();
}

const TransformKeyframeTrack& SceneObject::transformKeyframes() const
{
    return transformKeyframes_;
}

void SceneObject::setTransformKeyframes(const TransformKeyframeTrack& keyframes)
{
    transformKeyframes_ = keyframes;
    std::sort(transformKeyframes_.begin(), transformKeyframes_.end(), keyframeLessThan);
}

void SceneObject::setTransformKeyframe(int frame, const Transform& transform)
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

bool SceneObject::removeTransformKeyframe(int frame)
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

bool SceneObject::duplicateTransformKeyframe(int sourceFrame, int targetFrame)
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

bool SceneObject::offsetAllTransformKeyframes(int frameDelta)
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

int SceneObject::nextTransformKeyframeAfter(int frame) const
{
    int bestFrame = std::numeric_limits<int>::max();
    for (const TransformKeyframe& keyframe : transformKeyframes_) {
        if (keyframe.frame > frame && keyframe.frame < bestFrame) {
            bestFrame = keyframe.frame;
        }
    }

    return bestFrame == std::numeric_limits<int>::max() ? frame : bestFrame;
}

int SceneObject::previousTransformKeyframeBefore(int frame) const
{
    int bestFrame = std::numeric_limits<int>::min();
    for (const TransformKeyframe& keyframe : transformKeyframes_) {
        if (keyframe.frame < frame && keyframe.frame > bestFrame) {
            bestFrame = keyframe.frame;
        }
    }

    return bestFrame == std::numeric_limits<int>::min() ? frame : bestFrame;
}

const Bounds3D& SceneObject::localBounds() const
{
    return localBounds_;
}

void SceneObject::setLocalBounds(const Bounds3D& bounds)
{
    localBounds_ = bounds;
}

const Bounds3D& SceneObject::worldBounds() const
{
    return worldBounds_;
}

void SceneObject::setWorldBounds(const Bounds3D& bounds)
{
    worldBounds_ = bounds;
}

const QVector<int>& SceneObject::meshHandles() const
{
    return meshHandles_;
}

void SceneObject::addMeshHandle(int meshHandle)
{
    meshHandles_.append(meshHandle);
}

void SceneObject::clearMeshHandles()
{
    meshHandles_.clear();
}

bool SceneObject::isVisible() const
{
    return visible_;
}

void SceneObject::setVisible(bool visible)
{
    visible_ = visible;
}
