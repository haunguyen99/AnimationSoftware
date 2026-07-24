#include "scene/Scene.h"

#include <QDebug>
#include <QStringList>

#include <limits>

#include "scene/SceneMath.h"

namespace
{
constexpr float kJointBoundsRadius = 0.2f;
constexpr float kSkinWeightEpsilon = 0.0001f;

Transform interpolateTransform(const Transform& a, const Transform& b, float t)
{
    Transform result;
    result.translation = a.translation * (1.0f - t) + b.translation * t;
    result.rotation = QQuaternion::slerp(a.rotation, b.rotation, t);
    result.scale = a.scale * (1.0f - t) + b.scale * t;
    return result;
}

Bounds3D jointBoundsAtPosition(const QVector3D& position)
{
    return Bounds3D::fromMinMax(
        position - QVector3D(kJointBoundsRadius, kJointBoundsRadius, kJointBoundsRadius),
        position + QVector3D(kJointBoundsRadius, kJointBoundsRadius, kJointBoundsRadius));
}

SceneObject::Id nearestJointId(const QVector<SceneObject::Id>& jointIds, const QVector<QVector3D>& jointWorldPositions, const QVector3D& worldPosition)
{
    if (jointIds.isEmpty() || jointIds.size() != jointWorldPositions.size()) {
        return 0;
    }

    float bestDistanceSquared = std::numeric_limits<float>::max();
    SceneObject::Id bestJointId = 0;
    for (int index = 0; index < jointIds.size(); ++index) {
        const float distanceSquared = (jointWorldPositions.at(index) - worldPosition).lengthSquared();
        if (distanceSquared < bestDistanceSquared) {
            bestDistanceSquared = distanceSquared;
            bestJointId = jointIds.at(index);
        }
    }

    return bestJointId;
}

VertexSkinWeights normalizedVertexWeights(const VertexSkinWeights& inputWeights)
{
    QHash<SceneObject::Id, float> mergedWeights;
    for (const SkinWeight& inputWeight : inputWeights) {
        if (inputWeight.jointId == 0 || inputWeight.weight <= 0.0f) {
            continue;
        }

        mergedWeights[inputWeight.jointId] += inputWeight.weight;
    }

    float totalWeight = 0.0f;
    VertexSkinWeights outputWeights;
    outputWeights.reserve(mergedWeights.size());
    for (auto it = mergedWeights.cbegin(); it != mergedWeights.cend(); ++it) {
        if (it.value() <= kSkinWeightEpsilon) {
            continue;
        }

        totalWeight += it.value();
        outputWeights.append(SkinWeight { it.key(), it.value() });
    }

    if (totalWeight <= 0.0f) {
        return {};
    }

    for (SkinWeight& weight : outputWeights) {
        weight.weight /= totalWeight;
    }

    std::sort(outputWeights.begin(), outputWeights.end(), [](const SkinWeight& lhs, const SkinWeight& rhs) {
        if (lhs.weight == rhs.weight) {
            return lhs.jointId < rhs.jointId;
        }
        return lhs.weight > rhs.weight;
    });

    return outputWeights;
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
    if (parentId != 0 && !contains(parentId)) {
        return 0;
    }

    const SceneObject::Id id = createObject(name.isEmpty() ? QString("joint_%1").arg(nextId_ - 1) : name, SceneObject::Kind::Joint);
    if (parentId != 0 && !reparentObject(id, parentId)) {
        removeObject(id);
        return 0;
    }

    SceneObject* object = findObject(id);
    if (object != nullptr) {
        object->setHasBindPose(true);
        object->setBindPoseLocalTransform(object->authoredTransform());
    }

    rebuildWorldData();
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

bool Scene::duplicateObjectKeyframe(SceneObject::Id id, int sourceFrame, int targetFrame)
{
    SceneObject* object = findObject(id);
    if (object == nullptr || !object->duplicateTransformKeyframe(sourceFrame, targetFrame)) {
        return false;
    }

    currentFrame_ = targetFrame;
    rebuildWorldData();
    return true;
}

bool Scene::offsetObjectKeyframes(SceneObject::Id id, int frameDelta)
{
    SceneObject* object = findObject(id);
    if (object == nullptr || !object->offsetAllTransformKeyframes(frameDelta)) {
        return false;
    }

    currentFrame_ += frameDelta;
    rebuildWorldData();
    return true;
}

int Scene::nextObjectKeyframe(SceneObject::Id id, int frame) const
{
    const SceneObject* object = findObject(id);
    return object == nullptr ? frame : object->nextTransformKeyframeAfter(frame);
}

int Scene::previousObjectKeyframe(SceneObject::Id id, int frame) const
{
    const SceneObject* object = findObject(id);
    return object == nullptr ? frame : object->previousTransformKeyframeBefore(frame);
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
    SceneObject* object = findObject(id);
    if (object == nullptr || !object->isJoint()) {
        return false;
    }

    object->setJointOrientation(orientation);
    rebuildWorldData();
    return true;
}

bool Scene::resetJointOrientation(SceneObject::Id id)
{
    return setJointOrientation(id, QQuaternion());
}

bool Scene::alignJointOrientationToChild(SceneObject::Id id)
{
    SceneObject* object = findObject(id);
    if (object == nullptr || !object->isJoint() || object->childIds().isEmpty()) {
        return false;
    }

    const SceneObject* child = findObject(object->childIds().first());
    if (child == nullptr) {
        return false;
    }

    const QVector3D aimVector = child->localTransform().translation.normalized();
    if (aimVector.lengthSquared() < 0.0001f) {
        object->setJointOrientation(QQuaternion());
    } else {
        object->setJointOrientation(QQuaternion::rotationTo(QVector3D(1.0f, 0.0f, 0.0f), aimVector));
    }

    rebuildWorldData();
    return true;
}

bool Scene::captureBindPose(SceneObject::Id id, bool recursive)
{
    SceneObject* object = findObject(id);
    if (object == nullptr || !object->isJoint()) {
        return false;
    }

    object->setBindPoseLocalTransform(object->localTransform());
    object->setHasBindPose(true);

    if (recursive) {
        for (SceneObject::Id childId : object->childIds()) {
            const SceneObject* child = findObject(childId);
            if (child != nullptr && child->isJoint()) {
                captureBindPose(childId, true);
            }
        }
    }

    return true;
}

bool Scene::bindObjectToSkeleton(SceneObject::Id objectId, SceneObject::Id rootJointId)
{
    SceneObject* object = findObject(objectId);
    const SceneObject* rootJoint = findObject(rootJointId);
    if (object == nullptr || rootJoint == nullptr || !rootJoint->isJoint() || object->meshHandles().isEmpty()) {
        return false;
    }

    const QVector<SceneObject::Id> jointIds = collectJointSubtree(rootJointId);
    if (jointIds.isEmpty()) {
        return false;
    }

    captureBindPose(rootJointId, true);
    object->setSkinBindLocalTransform(object->localTransform());

    QVector<QVector3D> jointWorldPositions;
    jointWorldPositions.reserve(jointIds.size());
    for (SceneObject::Id jointId : jointIds) {
        jointWorldPositions.append(worldTransform(jointId) * QVector3D(0.0f, 0.0f, 0.0f));
    }

    const QMatrix4x4 objectWorld = worldTransform(objectId);
    SkinWeightTable weights;
    for (int meshHandle : object->meshHandles()) {
        const MeshData* mesh = findMesh(meshHandle);
        if (mesh == nullptr) {
            return false;
        }

        for (const QVector3D& localPosition : mesh->positions) {
            const QVector3D worldPosition = objectWorld * localPosition;
            const SceneObject::Id jointId = nearestJointId(jointIds, jointWorldPositions, worldPosition);
            if (jointId == 0) {
                return false;
            }

            weights.append(VertexSkinWeights { SkinWeight { jointId, 1.0f } });
        }
    }

    return setObjectSkinBinding(objectId, jointIds, weights);
}

bool Scene::setObjectSkinBinding(SceneObject::Id id, const QVector<SceneObject::Id>& jointIds, const SkinWeightTable& weights)
{
    SceneObject* object = findObject(id);
    if (object == nullptr || object->meshHandles().isEmpty()) {
        return false;
    }

    if (jointIds.isEmpty() || weights.isEmpty()) {
        return false;
    }

    int vertexCount = 0;
    for (int meshHandle : object->meshHandles()) {
        const MeshData* mesh = findMesh(meshHandle);
        if (mesh == nullptr) {
            return false;
        }
        vertexCount += mesh->positions.size();
    }

    if (weights.size() != vertexCount) {
        return false;
    }

    for (SceneObject::Id jointId : jointIds) {
        const SceneObject* joint = findObject(jointId);
        if (joint == nullptr || !joint->isJoint()) {
            return false;
        }
    }

    SkinWeightTable normalizedWeights;
    normalizedWeights.reserve(weights.size());
    for (const VertexSkinWeights& vertexWeights : weights) {
        const VertexSkinWeights normalizedWeightsForVertex = normalizedVertexWeights(vertexWeights);
        if (normalizedWeightsForVertex.isEmpty()) {
            return false;
        }

        float weightSum = 0.0f;
        for (const SkinWeight& weight : normalizedWeightsForVertex) {
            if (weight.jointId == 0 || !jointIds.contains(weight.jointId) || weight.weight < 0.0f) {
                return false;
            }
            weightSum += weight.weight;
        }

        if (std::abs(weightSum - 1.0f) > 0.001f) {
            return false;
        }

        normalizedWeights.append(normalizedWeightsForVertex);
    }

    object->setHasSkinBinding(true);
    object->setSkinBindLocalTransform(object->localTransform());
    object->setSkinJointIds(jointIds);
    object->setSkinWeights(normalizedWeights);
    return true;
}

bool Scene::clearObjectSkinBinding(SceneObject::Id id)
{
    SceneObject* object = findObject(id);
    if (object == nullptr) {
        return false;
    }

    object->clearSkinBinding();
    return true;
}

bool Scene::buildDeformedMesh(SceneObject::Id objectId, int meshHandle, MeshData* deformedMesh) const
{
    if (deformedMesh == nullptr) {
        return false;
    }

    const SceneObject* object = findObject(objectId);
    const MeshData* mesh = findMesh(meshHandle);
    if (object == nullptr || mesh == nullptr) {
        return false;
    }

    *deformedMesh = *mesh;
    if (!object->hasSkinBinding()) {
        return true;
    }

    int vertexOffset = 0;
    bool foundHandle = false;
    for (int objectMeshHandle : object->meshHandles()) {
        const MeshData* objectMesh = findMesh(objectMeshHandle);
        if (objectMesh == nullptr) {
            return false;
        }

        if (objectMeshHandle == meshHandle) {
            foundHandle = true;
            break;
        }

        vertexOffset += objectMesh->positions.size();
    }

    if (!foundHandle || object->skinWeights().size() < vertexOffset + mesh->positions.size()) {
        return false;
    }

    const QMatrix4x4 objectBindWorld = bindPoseWorldTransform(objectId);
    const QMatrix3x3 objectBindNormal = objectBindWorld.normalMatrix();
    deformedMesh->positions.resize(mesh->positions.size());
    deformedMesh->normals.resize(mesh->normals.size());

    for (int vertexIndex = 0; vertexIndex < mesh->positions.size(); ++vertexIndex) {
        const VertexSkinWeights& vertexWeights = object->skinWeights().at(vertexOffset + vertexIndex);
        QVector3D skinnedPosition;
        QVector3D skinnedNormal;

        const QVector3D bindPosition = objectBindWorld * mesh->positions.at(vertexIndex);
        const QVector3D sourceNormal = vertexIndex < mesh->normals.size()
            ? mesh->normals.at(vertexIndex)
            : QVector3D(0.0f, 1.0f, 0.0f);
        const QVector3D bindNormal = QVector3D(
            objectBindNormal(0, 0) * sourceNormal.x() + objectBindNormal(0, 1) * sourceNormal.y() + objectBindNormal(0, 2) * sourceNormal.z(),
            objectBindNormal(1, 0) * sourceNormal.x() + objectBindNormal(1, 1) * sourceNormal.y() + objectBindNormal(1, 2) * sourceNormal.z(),
            objectBindNormal(2, 0) * sourceNormal.x() + objectBindNormal(2, 1) * sourceNormal.y() + objectBindNormal(2, 2) * sourceNormal.z()).normalized();

        for (const SkinWeight& weight : vertexWeights) {
            const QMatrix4x4 jointWorld = worldTransform(weight.jointId);
            const QMatrix4x4 inverseBindJointWorld = bindPoseWorldTransform(weight.jointId).inverted();
            const QMatrix4x4 skinMatrix = jointWorld * inverseBindJointWorld;
            skinnedPosition += (skinMatrix * bindPosition) * weight.weight;
            skinnedNormal += skinMatrix.mapVector(bindNormal) * weight.weight;
        }

        deformedMesh->positions[vertexIndex] = skinnedPosition;
        if (vertexIndex < deformedMesh->normals.size()) {
            deformedMesh->normals[vertexIndex] = skinnedNormal.normalized();
        }
    }

    deformedMesh->bounds.reset();
    for (const QVector3D& position : deformedMesh->positions) {
        deformedMesh->bounds.expandToInclude(position);
    }

    return true;
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
            object->setTransformKeyframe(currentFrame_, updatedTransform);
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
        const SceneObject* chainObject = findObject(*it);
        if (chainObject == nullptr) {
            continue;
        }

        Transform bindTransform = chainObject->localTransform();
        if (chainObject->isJoint() && chainObject->hasBindPose()) {
            bindTransform = chainObject->bindPoseLocalTransform();
            bindTransform.rotation = chainObject->jointOrientation() * bindTransform.rotation;
        } else if (chainObject->hasSkinBinding()) {
            bindTransform = chainObject->skinBindLocalTransform();
        } else {
            bindTransform = composeObjectLocalTransform(*chainObject, chainObject->localTransform());
        }

        worldMatrix *= SceneMath::composeMatrix(bindTransform);
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

QVector<SceneObject::Id> Scene::collectJointSubtree(SceneObject::Id rootJointId) const
{
    QVector<SceneObject::Id> joints;
    const SceneObject* rootJoint = findObject(rootJointId);
    if (rootJoint == nullptr || !rootJoint->isJoint()) {
        return joints;
    }

    joints.append(rootJointId);
    for (SceneObject::Id childId : rootJoint->childIds()) {
        const SceneObject* child = findObject(childId);
        if (child == nullptr || !child->isJoint()) {
            continue;
        }

        joints += collectJointSubtree(childId);
    }

    return joints;
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
