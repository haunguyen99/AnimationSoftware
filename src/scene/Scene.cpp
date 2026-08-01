#include "scene/Scene.h"

#include <QDebug>
#include <QStringList>

#include <limits>

#include "rigging/scene/SceneRiggingController.h"
#include "scene/SceneMath.h"

namespace
{
constexpr float kJointBoundsRadius = 0.2f;

Bounds3D jointBoundsAtPosition(const QVector3D& position)
{
    return Bounds3D::fromMinMax(
        position - QVector3D(kJointBoundsRadius, kJointBoundsRadius, kJointBoundsRadius),
        position + QVector3D(kJointBoundsRadius, kJointBoundsRadius, kJointBoundsRadius));
}

}

Scene::Scene() = default;

SceneObject::Id Scene::createObject(const QString& name, SceneObject::Kind kind)
{
    const SceneObject::Id id = nextId_++;
    SceneObject object(id);
    object.setName(name.isEmpty() ? QString("Object_%1").arg(id) : name);
    object.setKind(kind);
    object.setAuthoredTransform(Transform());
    object.setLocalTransform(Transform());
    if (kind == SceneObject::Kind::Joint) {
        object.setHasBindPose(true);
        object.setBindPoseLocalTransform(Transform());
        object.setLocalBounds(jointBoundsAtPosition(QVector3D()));
    }
    objects_.insert(id, object);
    return id;
}

SceneObject::Id Scene::createJoint(const QString& name, SceneObject::Id parentId)
{
    return SceneRiggingController::createJoint(*this, name, parentId);
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
    animationState_.clear();
}

bool Scene::isEmpty() const
{
    return objects_.isEmpty();
}

int Scene::currentFrame() const
{
    return animationState_.currentFrame();
}

void Scene::setCurrentFrame(int frame)
{
    animationState_.setCurrentFrame(frame);
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

    animationState_.setLocalTransform(*object, transform, autoKeyEnabled);
    rebuildWorldData();
    return true;
}

bool Scene::setLocalTransformInteractive(SceneObject::Id id, const Transform& transform, bool autoKeyEnabled)
{
    SceneObject* object = findObject(id);
    if (object == nullptr) {
        return false;
    }

    animationState_.setLocalTransform(*object, transform, autoKeyEnabled);

    const QMatrix4x4 parentWorldMatrix = object->parentId() == 0
        ? QMatrix4x4()
        : worldTransform(object->parentId());
    rebuildWorldDataForObject(id, parentWorldMatrix);
    rebuildSceneBounds();
    return true;
}

bool Scene::setObjectKeyframe(SceneObject::Id id, int frame)
{
    SceneObject* object = findObject(id);
    if (object == nullptr) {
        return false;
    }

    animationState_.setObjectKeyframe(*object, frame);
    rebuildWorldData();
    return true;
}

bool Scene::removeObjectKeyframe(SceneObject::Id id, int frame)
{
    SceneObject* object = findObject(id);
    if (object == nullptr) {
        return false;
    }

    if (!animationState_.removeObjectKeyframe(*object, frame)) {
        return false;
    }

    rebuildWorldData();
    return true;
}

bool Scene::removeObjectKeyframesInRange(SceneObject::Id id, int startFrame, int endFrame)
{
    SceneObject* object = findObject(id);
    if (object == nullptr) {
        return false;
    }

    if (!animationState_.removeObjectKeyframesInRange(*object, startFrame, endFrame)) {
        return false;
    }

    rebuildWorldData();
    return true;
}

bool Scene::duplicateObjectKeyframe(SceneObject::Id id, int sourceFrame, int targetFrame)
{
    SceneObject* object = findObject(id);
    if (object == nullptr || !animationState_.duplicateObjectKeyframe(*object, sourceFrame, targetFrame)) {
        return false;
    }

    rebuildWorldData();
    return true;
}

bool Scene::offsetObjectKeyframes(SceneObject::Id id, int frameDelta)
{
    SceneObject* object = findObject(id);
    if (object == nullptr || !animationState_.offsetObjectKeyframes(*object, frameDelta)) {
        return false;
    }

    rebuildWorldData();
    return true;
}

bool Scene::offsetObjectKeyframesInRange(SceneObject::Id id, int startFrame, int endFrame, int frameDelta)
{
    SceneObject* object = findObject(id);
    if (object == nullptr || !animationState_.offsetObjectKeyframesInRange(*object, startFrame, endFrame, frameDelta)) {
        return false;
    }

    rebuildWorldData();
    return true;
}

bool Scene::scaleAllObjectKeyframes(double scaleFactor)
{
    bool changed = false;
    for (SceneObject::Id objectId : allObjectIds()) {
        SceneObject* object = findObject(objectId);
        if (object != nullptr && object->scaleAllTransformKeyframes(scaleFactor)) {
            changed = true;
        }
    }

    if (!changed) {
        return false;
    }

    rebuildWorldData();
    return true;
}

int Scene::nextObjectKeyframe(SceneObject::Id id, int frame) const
{
    const SceneObject* object = findObject(id);
    return object == nullptr ? frame : animationState_.nextObjectKeyframe(*object, frame);
}

int Scene::previousObjectKeyframe(SceneObject::Id id, int frame) const
{
    const SceneObject* object = findObject(id);
    return object == nullptr ? frame : animationState_.previousObjectKeyframe(*object, frame);
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

bool Scene::setJointOrientation(SceneObject::Id id, const QQuaternion& orientation)
{
    return SceneRiggingController::setJointOrientation(*this, id, orientation);
}

bool Scene::resetJointOrientation(SceneObject::Id id)
{
    return SceneRiggingController::resetJointOrientation(*this, id);
}

bool Scene::alignJointOrientationToChild(SceneObject::Id id)
{
    return SceneRiggingController::alignJointOrientationToChild(*this, id);
}

bool Scene::captureBindPose(SceneObject::Id id, bool recursive)
{
    return SceneRiggingController::captureBindPose(*this, id, recursive);
}

bool Scene::bindObjectToSkeleton(SceneObject::Id objectId, SceneObject::Id rootJointId)
{
    return SceneRiggingController::bindObjectToSkeleton(*this, objectId, rootJointId);
}

bool Scene::setObjectSkinBinding(SceneObject::Id id, const QVector<SceneObject::Id>& jointIds, const SkinWeightTable& weights)
{
    return SceneRiggingController::setObjectSkinBinding(*this, id, jointIds, weights);
}

bool Scene::clearObjectSkinBinding(SceneObject::Id id)
{
    return SceneRiggingController::clearObjectSkinBinding(*this, id);
}

bool Scene::buildDeformedMesh(SceneObject::Id objectId, int meshHandle, MeshData* deformedMesh) const
{
    return SceneRiggingController::buildDeformedMesh(*this, objectId, meshHandle, deformedMesh);
}

bool Scene::reparentObject(SceneObject::Id id, SceneObject::Id newParentId, bool keepWorldTransform)
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

    if (isDescendantOf(newParentId, id)) {
        return false;
    }

    const QMatrix4x4 currentWorldMatrix = worldTransform(id);
    const Transform currentLocalTransform = object->localTransform();

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

    if (keepWorldTransform) {
        const QMatrix4x4 parentWorldMatrix = newParentId == 0 ? QMatrix4x4() : worldTransform(newParentId);
        const QMatrix4x4 localMatrix = parentWorldMatrix.inverted() * currentWorldMatrix;

        Transform updatedTransform = currentLocalTransform;
        updatedTransform.translation = localMatrix * QVector3D(0.0f, 0.0f, 0.0f);
        updatedTransform.rotation = QQuaternion::fromRotationMatrix(localMatrix.normalMatrix());
        updatedTransform.scale = QVector3D(
            localMatrix.mapVector(QVector3D(1.0f, 0.0f, 0.0f)).length(),
            localMatrix.mapVector(QVector3D(0.0f, 1.0f, 0.0f)).length(),
            localMatrix.mapVector(QVector3D(0.0f, 0.0f, 1.0f)).length());

        if (object->isJoint()) {
            updatedTransform.rotation = object->jointOrientation().conjugated() * updatedTransform.rotation;
        }

        object->setLocalTransform(updatedTransform);
        object->setAuthoredTransform(updatedTransform);
        if (object->hasAnimation()) {
            object->setTransformKeyframe(animationState_.currentFrame(), updatedTransform);
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

    const SceneObject::Id newId = createObject(sourceObject->name(), sourceObject->kind());
    SceneObject* targetObject = findObject(newId);
    if (targetObject == nullptr) {
        return 0;
    }

    targetObject->setParentId(newParentId);
    targetObject->setLocalTransform(sourceObject->localTransform());
    targetObject->setAuthoredTransform(sourceObject->authoredTransform());
    targetObject->setJointOrientation(sourceObject->jointOrientation());
    targetObject->setBindPoseLocalTransform(sourceObject->bindPoseLocalTransform());
    targetObject->setHasBindPose(sourceObject->hasBindPose());
    targetObject->setHasSkinBinding(sourceObject->hasSkinBinding());
    targetObject->setSkinBindLocalTransform(sourceObject->skinBindLocalTransform());
    targetObject->setSkinJointIds(sourceObject->skinJointIds());
    targetObject->setSkinWeights(sourceObject->skinWeights());
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

        const SceneObject::Id newId = createObject(sourceObject->name(), sourceObject->kind());
        objectIdMap.insert(oldId, newId);
        Q_ASSERT(objectIdMap.contains(oldId));

        SceneObject* targetObject = findObject(newId);
        if (targetObject == nullptr) {
            Q_ASSERT_X(false, "Scene::appendScene", "Mapped object id must resolve in destination scene.");
            continue;
        }

        targetObject->setLocalTransform(sourceObject->localTransform());
        targetObject->setAuthoredTransform(sourceObject->authoredTransform());
        targetObject->setJointOrientation(sourceObject->jointOrientation());
        targetObject->setBindPoseLocalTransform(sourceObject->bindPoseLocalTransform());
        targetObject->setHasBindPose(sourceObject->hasBindPose());
        targetObject->setHasSkinBinding(sourceObject->hasSkinBinding());
        targetObject->setSkinBindLocalTransform(sourceObject->skinBindLocalTransform());
        targetObject->setSkinJointIds(sourceObject->skinJointIds());
        targetObject->setSkinWeights(sourceObject->skinWeights());
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

        QVector<SceneObject::Id> remappedSkinJointIds;
        remappedSkinJointIds.reserve(sourceObject->skinJointIds().size());
        for (SceneObject::Id jointId : sourceObject->skinJointIds()) {
            const SceneObject::Id remappedJointId = objectIdMap.value(jointId, 0);
            if (remappedJointId != 0) {
                remappedSkinJointIds.append(remappedJointId);
            }
        }
        targetObject->setSkinJointIds(remappedSkinJointIds);

        SkinWeightTable remappedSkinWeights = sourceObject->skinWeights();
        for (VertexSkinWeights& vertexWeights : remappedSkinWeights) {
            for (SkinWeight& weight : vertexWeights) {
                weight.jointId = objectIdMap.value(weight.jointId, 0);
            }
        }
        targetObject->setSkinWeights(remappedSkinWeights);

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
            worldMatrix *= SceneMath::composeMatrix(composeObjectLocalTransform(*chainObject, chainObject->localTransform()));
        }
    }

    return worldMatrix;
}

QMatrix4x4 Scene::bindPoseWorldTransform(SceneObject::Id id) const
{
    return SceneRiggingController::bindPoseWorldTransform(*this, id);
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

    const Transform evaluatedTransform = animationState_.evaluateObjectTransformAtCurrentFrame(*object);
    object->setLocalTransform(evaluatedTransform);
    const Transform composedTransform = composeObjectLocalTransform(*object, evaluatedTransform);
    const QMatrix4x4 worldMatrix = parentWorldMatrix * SceneMath::composeMatrix(composedTransform);
    if (object->isJoint()) {
        object->setWorldBounds(jointBoundsAtPosition(worldMatrix * QVector3D(0.0f, 0.0f, 0.0f)));
    } else {
        object->setWorldBounds(SceneMath::transformBounds(object->localBounds(), worldMatrix));
    }

    for (SceneObject::Id childId : object->childIds()) {
        rebuildWorldDataForObject(childId, worldMatrix);
    }
}

Transform Scene::composeObjectLocalTransform(const SceneObject& object, const Transform& baseTransform) const
{
    Transform composed = baseTransform;
    if (object.isJoint()) {
        composed.rotation = object.jointOrientation() * composed.rotation;
    }
    return composed;
}

bool Scene::isDescendantOf(SceneObject::Id id, SceneObject::Id potentialAncestorId) const
{
    SceneObject::Id ancestorId = id;
    while (ancestorId != 0) {
        if (ancestorId == potentialAncestorId) {
            return true;
        }

        const SceneObject* ancestor = findObject(ancestorId);
        ancestorId = ancestor == nullptr ? 0 : ancestor->parentId();
    }

    return false;
}

QString Scene::debugDump() const
{
    QStringList lines;
    lines << QString("Scene objects: %1").arg(objects_.size());

    for (auto it = objects_.cbegin(); it != objects_.cend(); ++it) {
        const SceneObject& object = it.value();
        lines << QString("[%1] %2 kind=%3 parent=%4 mesh=%5 children=%6")
                     .arg(object.id())
                     .arg(object.name())
                     .arg(object.isJoint() ? "joint" : "transform")
                     .arg(object.parentId())
                     .arg(object.meshHandles().size())
                     .arg(object.childIds().size());
    }

    return lines.join('\n');
}
