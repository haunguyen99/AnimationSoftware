#include "rigging/scene/SkinDeformationOps.h"

#include "rigging/scene/JointRiggingOps.h"
#include "scene/MeshData.h"
#include "scene/Scene.h"

namespace SkinDeformationOps
{
bool buildDeformedMesh(const Scene& scene, SceneObject::Id objectId, int meshHandle, MeshData* deformedMesh)
{
    if (deformedMesh == nullptr) {
        return false;
    }

    const SceneObject* object = scene.findObject(objectId);
    const MeshData* mesh = scene.findMesh(meshHandle);
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
        const MeshData* objectMesh = scene.findMesh(objectMeshHandle);
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

    const QMatrix4x4 objectBindWorld = JointRiggingOps::bindPoseWorldTransform(scene, objectId);
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
            const QMatrix4x4 jointWorld = scene.worldTransform(weight.jointId);
            const QMatrix4x4 inverseBindJointWorld = JointRiggingOps::bindPoseWorldTransform(scene, weight.jointId).inverted();
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
}
