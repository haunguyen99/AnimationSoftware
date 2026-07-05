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
