#include "scene/SceneObject.h"

#include <algorithm>

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
