#pragma once

#include <QHash>
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

    void rebuildSceneBounds();
    const Bounds3D& sceneBounds() const;

    QString debugDump() const;

private:
    SceneObject::Id nextId_ = 1;
    int nextMeshHandle_ = 1;
    QHash<SceneObject::Id, SceneObject> objects_;
    QHash<int, MeshData> meshes_;
    Bounds3D sceneBounds_;
};
