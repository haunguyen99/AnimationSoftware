#include "scene/SceneObject.h"

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
    return rigState_.jointOrientation();
}

void SceneObject::setJointOrientation(const QQuaternion& orientation)
{
    rigState_.setJointOrientation(orientation);
}

const Transform& SceneObject::bindPoseLocalTransform() const
{
    return rigState_.bindPoseLocalTransform();
}

void SceneObject::setBindPoseLocalTransform(const Transform& transform)
{
    rigState_.setBindPoseLocalTransform(transform);
}

bool SceneObject::hasBindPose() const
{
    return rigState_.hasBindPose();
}

void SceneObject::setHasBindPose(bool hasBindPose)
{
    rigState_.setHasBindPose(hasBindPose);
}

bool SceneObject::hasSkinBinding() const
{
    return rigState_.hasSkinBinding();
}

void SceneObject::setHasSkinBinding(bool hasSkinBinding)
{
    rigState_.setHasSkinBinding(hasSkinBinding);
}

const Transform& SceneObject::skinBindLocalTransform() const
{
    return rigState_.skinBindLocalTransform();
}

void SceneObject::setSkinBindLocalTransform(const Transform& transform)
{
    rigState_.setSkinBindLocalTransform(transform);
}

const QVector<SceneObject::Id>& SceneObject::skinJointIds() const
{
    return rigState_.skinJointIds();
}

void SceneObject::setSkinJointIds(const QVector<Id>& jointIds)
{
    rigState_.setSkinJointIds(jointIds);
}

const SkinWeightTable& SceneObject::skinWeights() const
{
    return rigState_.skinWeights();
}

void SceneObject::setSkinWeights(const SkinWeightTable& weights)
{
    rigState_.setSkinWeights(weights);
}

void SceneObject::clearSkinBinding()
{
    rigState_.clearSkinBinding();
}

bool SceneObject::hasAnimation() const
{
    return animationState_.hasAnimation();
}

bool SceneObject::hasTransformKeyframe(int frame) const
{
    return animationState_.hasTransformKeyframe(frame);
}

const TransformKeyframeTrack& SceneObject::transformKeyframes() const
{
    return animationState_.transformKeyframes();
}

void SceneObject::setTransformKeyframes(const TransformKeyframeTrack& keyframes)
{
    animationState_.setTransformKeyframes(keyframes);
}

void SceneObject::setTransformKeyframe(int frame, const Transform& transform)
{
    animationState_.setTransformKeyframe(frame, transform);
}

bool SceneObject::removeTransformKeyframe(int frame)
{
    return animationState_.removeTransformKeyframe(frame);
}

bool SceneObject::removeTransformKeyframesInRange(int startFrame, int endFrame)
{
    return animationState_.removeTransformKeyframesInRange(startFrame, endFrame);
}

bool SceneObject::duplicateTransformKeyframe(int sourceFrame, int targetFrame)
{
    return animationState_.duplicateTransformKeyframe(sourceFrame, targetFrame);
}

bool SceneObject::offsetAllTransformKeyframes(int frameDelta)
{
    return animationState_.offsetAllTransformKeyframes(frameDelta);
}

bool SceneObject::offsetTransformKeyframesInRange(int startFrame, int endFrame, int frameDelta)
{
    return animationState_.offsetTransformKeyframesInRange(startFrame, endFrame, frameDelta);
}

bool SceneObject::scaleAllTransformKeyframes(double scaleFactor)
{
    return animationState_.scaleAllTransformKeyframes(scaleFactor);
}

int SceneObject::nextTransformKeyframeAfter(int frame) const
{
    return animationState_.nextTransformKeyframeAfter(frame);
}

int SceneObject::previousTransformKeyframeBefore(int frame) const
{
    return animationState_.previousTransformKeyframeBefore(frame);
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
