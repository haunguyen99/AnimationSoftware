#pragma once

#include <QHash>
#include <QMatrix4x4>
#include <QString>
#include <QVector>

#include "scene/Bounds3D.h"
#include "scene/MeshData.h"
#include "scene/SceneObject.h"

class Scene
{
public:
    Scene();

    SceneObject::Id createObject(const QString& name = {});
    bool contains(SceneObject::Id id) const;

    SceneObject* findObject(SceneObject::Id id);
    const SceneObject* findObject(SceneObject::Id id) const;

    QVector<SceneObject::Id> rootObjectIds() const;
    QVector<SceneObject::Id> allObjectIds() const;
    int addMesh(const MeshData& meshData);
    const MeshData* findMesh(int meshHandle) const;
    QVector<int> allMeshHandles() const;

    void clear();
    bool isEmpty() const;
    void appendScene(const Scene& other);
    bool setObjectName(SceneObject::Id id, const QString& name);
    int currentFrame() const;
    void setCurrentFrame(int frame);
    bool setLocalTransform(SceneObject::Id id, const Transform& transform, bool autoKeyEnabled = false);
    bool setObjectKeyframe(SceneObject::Id id, int frame);
    bool removeObjectKeyframe(SceneObject::Id id, int frame);
    bool setObjectVisible(SceneObject::Id id, bool visible);
    bool reparentObject(SceneObject::Id id, SceneObject::Id newParentId);
    bool removeObject(SceneObject::Id id);
    SceneObject::Id duplicateSubtree(SceneObject::Id id, SceneObject::Id newParentId = 0);
    void optimizeStorage();
    QMatrix4x4 worldTransform(SceneObject::Id id) const;

    void rebuildSceneBounds();
    void rebuildWorldData();
    const Bounds3D& sceneBounds() const;

    QString debugDump() const;

private:
    Transform evaluateObjectTransformAtFrame(const SceneObject& object, int frame) const;
    SceneObject::Id duplicateSubtreeRecursive(const Scene& sourceScene, SceneObject::Id sourceId, SceneObject::Id newParentId);
    void removeObjectRecursive(SceneObject::Id id);
    void rebuildWorldDataForObject(SceneObject::Id objectId, const QMatrix4x4& parentWorldMatrix);

    SceneObject::Id nextId_ = 1;
    int nextMeshHandle_ = 1;
    QHash<SceneObject::Id, SceneObject> objects_;
    QHash<int, MeshData> meshes_;
    Bounds3D sceneBounds_;
    int currentFrame_ = 0;
};
