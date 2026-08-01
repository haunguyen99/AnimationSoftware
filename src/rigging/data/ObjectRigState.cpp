#include "rigging/data/ObjectRigState.h"

const QQuaternion& ObjectRigState::jointOrientation() const
{
    return jointOrientation_;
}

void ObjectRigState::setJointOrientation(const QQuaternion& orientation)
{
    jointOrientation_ = orientation.normalized();
}

const Transform& ObjectRigState::bindPoseLocalTransform() const
{
    return bindPoseLocalTransform_;
}

void ObjectRigState::setBindPoseLocalTransform(const Transform& transform)
{
    bindPoseLocalTransform_ = transform;
}

bool ObjectRigState::hasBindPose() const
{
    return hasBindPose_;
}

void ObjectRigState::setHasBindPose(bool hasBindPose)
{
    hasBindPose_ = hasBindPose;
}

bool ObjectRigState::hasSkinBinding() const
{
    return hasSkinBinding_;
}

void ObjectRigState::setHasSkinBinding(bool hasSkinBinding)
{
    hasSkinBinding_ = hasSkinBinding;
}

const Transform& ObjectRigState::skinBindLocalTransform() const
{
    return skinBindLocalTransform_;
}

void ObjectRigState::setSkinBindLocalTransform(const Transform& transform)
{
    skinBindLocalTransform_ = transform;
}

const QVector<std::uint64_t>& ObjectRigState::skinJointIds() const
{
    return skinJointIds_;
}

void ObjectRigState::setSkinJointIds(const QVector<std::uint64_t>& jointIds)
{
    skinJointIds_ = jointIds;
}

const SkinWeightTable& ObjectRigState::skinWeights() const
{
    return skinWeights_;
}

void ObjectRigState::setSkinWeights(const SkinWeightTable& weights)
{
    skinWeights_ = weights;
}

void ObjectRigState::clearSkinBinding()
{
    hasSkinBinding_ = false;
    skinBindLocalTransform_ = Transform();
    skinJointIds_.clear();
    skinWeights_.clear();
}
