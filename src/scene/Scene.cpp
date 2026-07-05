#include "scene/Scene.h"

#include <QDebug>
#include <QStringList>

Scene::Scene() = default;

SceneObject::Id Scene::createObject(const QString& name)
{
    const SceneObject::Id id = nextId_++;
    SceneObject object(id);
    object.setName(name.isEmpty() ? QString("Object_%1").arg(id) : name);
    objects_.insert(id, object);
    return id;
}

bool Scene::contains(SceneObject::Id id) const
{
    return objects_.contains(id);
}

SceneObject* Scene::findObject(SceneObject::Id id)
{
    auto it = objects_.find(id);
    return it == objects_.end() ? nullptr : &it.value();
}

const SceneObject* Scene::findObject(SceneObject::Id id) const
{
    auto it = objects_.constFind(id);
    return it == objects_.cend() ? nullptr : &it.value();
}

QVector<SceneObject::Id> Scene::rootObjectIds() const
{
    QVector<SceneObject::Id> roots;

    for (auto it = objects_.cbegin(); it != objects_.cend(); ++it) {
        if (it.value().parentId() == 0) {
            roots.append(it.key());
        }
    }

    return roots;
}

QVector<SceneObject::Id> Scene::allObjectIds() const
{
    return objects_.keys().toVector();
}

int Scene::addMesh(const MeshData& meshData)
{
    const int handle = nextMeshHandle_++;
    meshes_.insert(handle, meshData);
    return handle;
}

const MeshData* Scene::findMesh(int meshHandle) const
{
    auto it = meshes_.constFind(meshHandle);
    return it == meshes_.cend() ? nullptr : &it.value();
}

QVector<int> Scene::allMeshHandles() const
{
    return meshes_.keys().toVector();
}

void Scene::clear()
{
    objects_.clear();
    meshes_.clear();
    sceneBounds_.reset();
    nextId_ = 1;
    nextMeshHandle_ = 1;
}

bool Scene::isEmpty() const
{
    return objects_.isEmpty();
}

void Scene::appendScene(const Scene& other)
{
    Q_ASSERT(&other != this);

    QHash<SceneObject::Id, SceneObject::Id> objectIdMap;
    QHash<int, int> meshHandleMap;

    const QVector<SceneObject::Id> otherObjectIds = other.allObjectIds();
    for (SceneObject::Id oldId : otherObjectIds) {
        const SceneObject* sourceObject = other.findObject(oldId);
        if (sourceObject == nullptr) {
            continue;
        }

        const SceneObject::Id newId = createObject(sourceObject->name());
        objectIdMap.insert(oldId, newId);
        Q_ASSERT(objectIdMap.contains(oldId));

        SceneObject* targetObject = findObject(newId);
        if (targetObject == nullptr) {
            Q_ASSERT_X(false, "Scene::appendScene", "Mapped object id must resolve in destination scene.");
            continue;
        }

        targetObject->setLocalTransform(sourceObject->localTransform());
        targetObject->setLocalBounds(sourceObject->localBounds());
        targetObject->setWorldBounds(sourceObject->worldBounds());
    }

    const QVector<int> otherMeshHandles = other.allMeshHandles();
    for (int oldMeshHandle : otherMeshHandles) {
        const MeshData* sourceMesh = other.findMesh(oldMeshHandle);
        if (sourceMesh == nullptr) {
            continue;
        }

        const int newMeshHandle = addMesh(*sourceMesh);
        meshHandleMap.insert(oldMeshHandle, newMeshHandle);
        Q_ASSERT(meshHandleMap.contains(oldMeshHandle));
    }

    for (SceneObject::Id oldId : otherObjectIds) {
        const SceneObject* sourceObject = other.findObject(oldId);
        SceneObject* targetObject = findObject(objectIdMap.value(oldId));
        if (sourceObject == nullptr || targetObject == nullptr) {
            continue;
        }

        targetObject->clearChildren();
        targetObject->clearMeshHandles();

        const SceneObject::Id oldParentId = sourceObject->parentId();
        targetObject->setParentId(oldParentId == 0 ? 0 : objectIdMap.value(oldParentId, 0));

        for (SceneObject::Id childId : sourceObject->childIds()) {
            const SceneObject::Id mappedChildId = objectIdMap.value(childId, 0);
            Q_ASSERT(mappedChildId != 0);
            targetObject->addChildId(mappedChildId);
        }

        for (int meshHandle : sourceObject->meshHandles()) {
            const int mappedHandle = meshHandleMap.value(meshHandle, -1);
            if (mappedHandle >= 0) {
                targetObject->addMeshHandle(mappedHandle);
            } else {
                Q_ASSERT_X(false, "Scene::appendScene", "Mapped mesh handle must exist in destination scene.");
            }
        }
    }

    rebuildSceneBounds();
}

void Scene::rebuildSceneBounds()
{
    sceneBounds_.reset();

    for (auto it = objects_.cbegin(); it != objects_.cend(); ++it) {
        sceneBounds_.expandToInclude(it.value().worldBounds());
    }
}

const Bounds3D& Scene::sceneBounds() const
{
    return sceneBounds_;
}

QString Scene::debugDump() const
{
    QStringList lines;
    lines << QString("Scene objects: %1").arg(objects_.size());

    for (auto it = objects_.cbegin(); it != objects_.cend(); ++it) {
        const SceneObject& object = it.value();
        lines << QString("[%1] %2 parent=%3 mesh=%4 children=%5")
                     .arg(object.id())
                     .arg(object.name())
                     .arg(object.parentId())
                     .arg(object.meshHandles().size())
                     .arg(object.childIds().size());
    }

    return lines.join('\n');
}
