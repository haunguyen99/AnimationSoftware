#include "rigging/scene/SkinBindingOps.h"

#include <QHash>

#include <algorithm>
#include <cmath>
#include <limits>

#include "rigging/scene/JointRiggingOps.h"
#include "scene/MeshData.h"
#include "scene/Scene.h"

namespace
{
constexpr float kSkinWeightEpsilon = 0.0001f;

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

QVector<SceneObject::Id> collectJointSubtree(const Scene& scene, SceneObject::Id rootJointId)
{
    QVector<SceneObject::Id> joints;
    const SceneObject* rootJoint = scene.findObject(rootJointId);
    if (rootJoint == nullptr || !rootJoint->isJoint()) {
        return joints;
    }

    joints.append(rootJointId);
    for (SceneObject::Id childId : rootJoint->childIds()) {
        const SceneObject* child = scene.findObject(childId);
        if (child == nullptr || !child->isJoint()) {
            continue;
        }

        joints += collectJointSubtree(scene, childId);
    }

    return joints;
}
}

namespace SkinBindingOps
{
bool bindObjectToSkeleton(Scene& scene, SceneObject::Id objectId, SceneObject::Id rootJointId)
{
    SceneObject* object = scene.findObject(objectId);
    const SceneObject* rootJoint = scene.findObject(rootJointId);
    if (object == nullptr || rootJoint == nullptr || !rootJoint->isJoint() || object->meshHandles().isEmpty()) {
        return false;
    }

    const QVector<SceneObject::Id> jointIds = collectJointSubtree(scene, rootJointId);
    if (jointIds.isEmpty()) {
        return false;
    }

    JointRiggingOps::captureBindPose(scene, rootJointId, true);
    object->setSkinBindLocalTransform(object->localTransform());

    QVector<QVector3D> jointWorldPositions;
    jointWorldPositions.reserve(jointIds.size());
    for (SceneObject::Id jointId : jointIds) {
        jointWorldPositions.append(scene.worldTransform(jointId) * QVector3D(0.0f, 0.0f, 0.0f));
    }

    const QMatrix4x4 objectWorld = scene.worldTransform(objectId);
    SkinWeightTable weights;
    for (int meshHandle : object->meshHandles()) {
        const MeshData* mesh = scene.findMesh(meshHandle);
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

    return setObjectSkinBinding(scene, objectId, jointIds, weights);
}

bool setObjectSkinBinding(Scene& scene, SceneObject::Id id, const QVector<SceneObject::Id>& jointIds, const SkinWeightTable& weights)
{
    SceneObject* object = scene.findObject(id);
    if (object == nullptr || object->meshHandles().isEmpty()) {
        return false;
    }

    if (jointIds.isEmpty() || weights.isEmpty()) {
        return false;
    }

    int vertexCount = 0;
    for (int meshHandle : object->meshHandles()) {
        const MeshData* mesh = scene.findMesh(meshHandle);
        if (mesh == nullptr) {
            return false;
        }
        vertexCount += mesh->positions.size();
    }

    if (weights.size() != vertexCount) {
        return false;
    }

    for (SceneObject::Id jointId : jointIds) {
        const SceneObject* joint = scene.findObject(jointId);
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

bool clearObjectSkinBinding(Scene& scene, SceneObject::Id id)
{
    SceneObject* object = scene.findObject(id);
    if (object == nullptr) {
        return false;
    }

    object->clearSkinBinding();
    return true;
}
}
