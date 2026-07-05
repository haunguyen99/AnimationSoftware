#pragma once

#include <QString>
#include <QVector>

#include <cstdint>

#include "scene/Bounds3D.h"
#include "scene/Transform.h"

class SceneObject
{
public:
    using Id = std::uint64_t;

    explicit SceneObject(Id id = 0);

    Id id() const;

    const QString& name() const;
    void setName(const QString& name);

    Id parentId() const;
    void setParentId(Id parentId);

    const QVector<Id>& childIds() const;
    void addChildId(Id childId);
    void clearChildren();

    const Transform& localTransform() const;
    void setLocalTransform(const Transform& transform);

    const Bounds3D& localBounds() const;
    void setLocalBounds(const Bounds3D& bounds);

    const Bounds3D& worldBounds() const;
    void setWorldBounds(const Bounds3D& bounds);

    const QVector<int>& meshHandles() const;
    void addMeshHandle(int meshHandle);
    void clearMeshHandles();

private:
    Id id_ = 0;
    QString name_;
    Id parentId_ = 0;
    QVector<Id> childIds_;
    QVector<int> meshHandles_;
    Transform localTransform_;
    Bounds3D localBounds_;
    Bounds3D worldBounds_;
};
