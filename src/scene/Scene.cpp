#include "scene/Scene.h"

#include <QDebug>
#include <QStringList>

#include "scene/SceneMath.h"

namespace
{
Transform interpolateTransform(const Transform& a, const Transform& b, float t)
{
    Transform result;
    result.translation = a.translation * (1.0f - t) + b.translation * t;
    result.rotation = QQuaternion::slerp(a.rotation, b.rotation, t);
    result.scale = a.scale * (1.0f - t) + b.scale * t;
    return result;
}
}

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
    currentFrame_ = 0;
}

bool Scene::isEmpty() const
{
    return objects_.isEmpty();
}

int Scene::currentFrame() const
{
    return currentFrame_;
}

void Scene::setCurrentFrame(int frame)
{
    currentFrame_ = frame;
    rebuildWorldData();
}

bool Scene::setObjectName(SceneObject::Id id, const QString& name)
{
    SceneObject* object = findObject(id);
    if (object == nullptr) {
        return false;
    }

    object->setName(name);
    return true;
}

bool Scene::setLocalTransform(SceneObject::Id id, const Transform& transform, bool autoKeyEnabled)
{
    SceneObject* object = findObject(id);
    if (object == nullptr) {
        return false;
    }

    if (object->hasAnimation()) {
        object->setTransformKeyframe(currentFrame_, transform);
    } else if (autoKeyEnabled && currentFrame_ != 0) {
        object->setTransformKeyframe(0, object->authoredTransform());
        object->setTransformKeyframe(currentFrame_, transform);
    } else {
        object->setAuthoredTransform(transform);
    }

    object->setLocalTransform(transform);
    rebuildWorldData();
    return true;
}

bool Scene::setObjectKeyframe(SceneObject::Id id, int frame)
{
    SceneObject* object = findObject(id);
    if (object == nullptr) {
        return false;
    }

    object->setTransformKeyframe(frame, object->localTransform());
    currentFrame_ = frame;
    rebuildWorldData();
    return true;
}

bool Scene::removeObjectKeyframe(SceneObject::Id id, int frame)
{
    SceneObject* object = findObject(id);
    if (object == nullptr) {
        return false;
    }

    if (!object->removeTransformKeyframe(frame)) {
        return false;
    }

    rebuildWorldData();
    return true;
}

bool Scene::setObjectVisible(SceneObject::Id id, bool visible)
{
    SceneObject* object = findObject(id);
    if (object == nullptr) {
        return false;
    }

    object->setVisible(visible);
    return true;
}

bool Scene::reparentObject(SceneObject::Id id, SceneObject::Id newParentId)
{
    SceneObject* object = findObject(id);
    if (object == nullptr) {
        return false;
    }

    if (newParentId != 0 && !contains(newParentId)) {
        return false;
    }

    if (id == newParentId) {
        return false;
    }

    SceneObject::Id ancestorId = newParentId;
    while (ancestorId != 0) {
        if (ancestorId == id) {
            return false;
        }

        const SceneObject* ancestor = findObject(ancestorId);
        ancestorId = ancestor == nullptr ? 0 : ancestor->parentId();
    }

    if (object->parentId() != 0) {
        if (SceneObject* oldParent = findObject(object->parentId())) {
            oldParent->removeChildId(id);
        }
    }

    object->setParentId(newParentId);
    if (newParentId != 0) {
        SceneObject* newParent = findObject(newParentId);
        if (newParent == nullptr) {
            return false;
        }

        if (!newParent->childIds().contains(id)) {
            newParent->addChildId(id);
        }
    }

    rebuildWorldData();
    return true;
}

bool Scene::removeObject(SceneObject::Id id)
{
    if (!contains(id)) {
        return false;
    }

    const SceneObject* object = findObject(id);
    if (object != nullptr && object->parentId() != 0) {
        if (SceneObject* parent = findObject(object->parentId())) {
            parent->removeChildId(id);
        }
    }

    removeObjectRecursive(id);
    rebuildWorldData();
    return true;
}

SceneObject::Id Scene::duplicateSubtree(SceneObject::Id id, SceneObject::Id newParentId)
{
    if (!contains(id)) {
        return 0;
    }

    if (newParentId != 0 && !contains(newParentId)) {
        return 0;
    }

    const Scene snapshot = *this;
    const SceneObject* sourceObject = snapshot.findObject(id);
    if (sourceObject == nullptr) {
        return 0;
    }

    const SceneObject::Id parentId = newParentId == 0 ? sourceObject->parentId() : newParentId;
    const SceneObject::Id duplicateId = duplicateSubtreeRecursive(snapshot, id, parentId);
    rebuildWorldData();
    return duplicateId;
}

void Scene::optimizeStorage()
{
    QHash<int, int> meshHandleMap;
    QHash<int, MeshData> optimizedMeshes;
    int nextHandle = 1;

    QVector<int> usedHandles;
    for (auto it = objects_.cbegin(); it != objects_.cend(); ++it) {
        for (int meshHandle : it.value().meshHandles()) {
            if (!meshHandleMap.contains(meshHandle)) {
                const MeshData* mesh = findMesh(meshHandle);
                if (mesh == nullptr) {
                    continue;
                }

                const int newHandle = nextHandle++;
                meshHandleMap.insert(meshHandle, newHandle);
                optimizedMeshes.insert(newHandle, *mesh);
            }
        }
    }

    for (auto it = objects_.begin(); it != objects_.end(); ++it) {
        const QVector<int> oldHandles = it.value().meshHandles();
        it.value().clearMeshHandles();
        for (int oldHandle : oldHandles) {
            const int newHandle = meshHandleMap.value(oldHandle, -1);
            if (newHandle >= 0) {
                it.value().addMeshHandle(newHandle);
            }
        }
    }

    meshes_ = optimizedMeshes;
    nextMeshHandle_ = nextHandle;
}

SceneObject::Id Scene::duplicateSubtreeRecursive(const Scene& sourceScene, SceneObject::Id sourceId, SceneObject::Id newParentId)
{
    const SceneObject* sourceObject = sourceScene.findObject(sourceId);
    if (sourceObject == nullptr) {
        return 0;
    }

    const SceneObject::Id newId = createObject(sourceObject->name());
    SceneObject* targetObject = findObject(newId);
    if (targetObject == nullptr) {
        return 0;
    }

    targetObject->setParentId(newParentId);
    targetObject->setLocalTransform(sourceObject->localTransform());
    targetObject->setAuthoredTransform(sourceObject->authoredTransform());
    targetObject->setTransformKeyframes(sourceObject->transformKeyframes());
    targetObject->setLocalBounds(sourceObject->localBounds());
    targetObject->setWorldBounds(sourceObject->worldBounds());
    targetObject->setVisible(sourceObject->isVisible());

    if (newParentId != 0) {
        if (SceneObject* parentObject = findObject(newParentId)) {
            parentObject->addChildId(newId);
        }
    }

    for (int meshHandle : sourceObject->meshHandles()) {
        const MeshData* mesh = sourceScene.findMesh(meshHandle);
        if (mesh == nullptr) {
            continue;
        }

        targetObject->addMeshHandle(addMesh(*mesh));
    }

    for (SceneObject::Id childId : sourceObject->childIds()) {
        duplicateSubtreeRecursive(sourceScene, childId, newId);
    }

    return newId;
}

void Scene::removeObjectRecursive(SceneObject::Id id)
{
    const SceneObject* object = findObject(id);
    if (object == nullptr) {
        return;
    }

    const QVector<SceneObject::Id> childIds = object->childIds();
    for (SceneObject::Id childId : childIds) {
        removeObjectRecursive(childId);
    }

    objects_.remove(id);
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
        targetObject->setAuthoredTransform(sourceObject->authoredTransform());
        targetObject->setTransformKeyframes(sourceObject->transformKeyframes());
        targetObject->setLocalBounds(sourceObject->localBounds());
        targetObject->setWorldBounds(sourceObject->worldBounds());
        targetObject->setVisible(sourceObject->isVisible());
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

QMatrix4x4 Scene::worldTransform(SceneObject::Id id) const
{
    const SceneObject* object = findObject(id);
    if (object == nullptr) {
        return QMatrix4x4();
    }

    QVector<SceneObject::Id> chain;
    chain.reserve(16);

    const SceneObject* current = object;
    while (current != nullptr) {
        chain.append(current->id());
        current = current->parentId() == 0 ? nullptr : findObject(current->parentId());
    }

    QMatrix4x4 worldMatrix;
    for (auto it = chain.crbegin(); it != chain.crend(); ++it) {
        if (const SceneObject* chainObject = findObject(*it)) {
            worldMatrix *= SceneMath::composeMatrix(chainObject->localTransform());
        }
    }

    return worldMatrix;
}

void Scene::rebuildSceneBounds()
{
    sceneBounds_.reset();

    for (auto it = objects_.cbegin(); it != objects_.cend(); ++it) {
        sceneBounds_.expandToInclude(it.value().worldBounds());
    }
}

void Scene::rebuildWorldData()
{
    const QVector<SceneObject::Id> rootIds = rootObjectIds();
    for (SceneObject::Id rootId : rootIds) {
        rebuildWorldDataForObject(rootId, QMatrix4x4());
    }

    rebuildSceneBounds();
}

const Bounds3D& Scene::sceneBounds() const
{
    return sceneBounds_;
}

void Scene::rebuildWorldDataForObject(SceneObject::Id objectId, const QMatrix4x4& parentWorldMatrix)
{
    SceneObject* object = findObject(objectId);
    if (object == nullptr) {
        return;
    }

    const Transform evaluatedTransform = evaluateObjectTransformAtFrame(*object, currentFrame_);
    object->setLocalTransform(evaluatedTransform);
    const QMatrix4x4 worldMatrix = parentWorldMatrix * SceneMath::composeMatrix(evaluatedTransform);
    object->setWorldBounds(SceneMath::transformBounds(object->localBounds(), worldMatrix));

    for (SceneObject::Id childId : object->childIds()) {
        rebuildWorldDataForObject(childId, worldMatrix);
    }
}

Transform Scene::evaluateObjectTransformAtFrame(const SceneObject& object, int frame) const
{
    const TransformKeyframeTrack& keyframes = object.transformKeyframes();
    if (keyframes.isEmpty()) {
        return object.authoredTransform();
    }

    if (frame <= keyframes.first().frame) {
        return keyframes.first().transform;
    }

    if (frame >= keyframes.last().frame) {
        return keyframes.last().transform;
    }

    for (int index = 0; index < keyframes.size() - 1; ++index) {
        const TransformKeyframe& a = keyframes.at(index);
        const TransformKeyframe& b = keyframes.at(index + 1);
        if (frame < a.frame || frame > b.frame) {
            continue;
        }

        if (a.frame == b.frame) {
            return b.transform;
        }

        const float t = static_cast<float>(frame - a.frame) / static_cast<float>(b.frame - a.frame);
        return interpolateTransform(a.transform, b.transform, t);
    }

    return object.authoredTransform();
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
