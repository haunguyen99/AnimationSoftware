#include "rendering/scene/ViewportRenderSceneAdapter.h"

#include "scene/MeshData.h"
#include "scene/Scene.h"
#include "scene/SceneObject.h"

#include <QSet>

namespace
{
void appendLine(QVector<RenderVertex>& vertices,
    const QVector3D& a,
    const QVector3D& b,
    const QVector3D& color)
{
    const QVector3D normal(0.0f, 1.0f, 0.0f);
    vertices.append({ a, normal, color });
    vertices.append({ b, normal, color });
}

void appendJointDiamond(QVector<RenderVertex>& vertices,
    const QVector3D& origin,
    const QVector3D& color,
    float size)
{
    const QVector3D top = origin + QVector3D(0.0f, size, 0.0f);
    const QVector3D bottom = origin + QVector3D(0.0f, -size, 0.0f);
    const QVector3D left = origin + QVector3D(-size, 0.0f, 0.0f);
    const QVector3D right = origin + QVector3D(size, 0.0f, 0.0f);
    const QVector3D front = origin + QVector3D(0.0f, 0.0f, size);
    const QVector3D back = origin + QVector3D(0.0f, 0.0f, -size);

    appendLine(vertices, top, left, color);
    appendLine(vertices, top, right, color);
    appendLine(vertices, top, front, color);
    appendLine(vertices, top, back, color);

    appendLine(vertices, bottom, left, color);
    appendLine(vertices, bottom, right, color);
    appendLine(vertices, bottom, front, color);
    appendLine(vertices, bottom, back, color);

    appendLine(vertices, left, front, color);
    appendLine(vertices, front, right, color);
    appendLine(vertices, right, back, color);
    appendLine(vertices, back, left, color);
}

void collectSubtreeIds(const Scene& scene, SceneObject::Id rootObjectId, QSet<SceneObject::Id>& ids)
{
    if (rootObjectId == 0 || ids.contains(rootObjectId)) {
        return;
    }

    const SceneObject* object = scene.findObject(rootObjectId);
    if (object == nullptr) {
        return;
    }

    ids.insert(rootObjectId);
    for (SceneObject::Id childId : object->childIds()) {
        collectSubtreeIds(scene, childId, ids);
    }
}

ViewportRenderSceneData buildSceneDataFiltered(
    const Scene& scene,
    const QSet<SceneObject::Id>* includeIds,
    const QSet<SceneObject::Id>* excludeIds)
{
    ViewportRenderSceneData data;

    std::uint32_t vertexOffset = 0;
    for (SceneObject::Id objectId : scene.allObjectIds()) {
        if (includeIds != nullptr && !includeIds->contains(objectId)) {
            continue;
        }
        if (excludeIds != nullptr && excludeIds->contains(objectId)) {
            continue;
        }

        const SceneObject* object = scene.findObject(objectId);
        if (object == nullptr || !object->isVisible()) {
            continue;
        }

        if (!object->meshHandles().isEmpty()) {
            for (int meshHandle : object->meshHandles()) {
                MeshData deformedMesh;
                const MeshData* mesh = scene.findMesh(meshHandle);
                if (mesh == nullptr) {
                    Q_ASSERT_X(false, "ViewportRenderSceneAdapter::buildSceneDataFiltered", "SceneObject references missing mesh handle.");
                    continue;
                }

                const bool useDeformedMesh = object->hasSkinBinding() && scene.buildDeformedMesh(objectId, meshHandle, &deformedMesh);
                const MeshData& renderMesh = useDeformedMesh ? deformedMesh : *mesh;

                Q_ASSERT(renderMesh.positions.size() == renderMesh.normals.size());
                Q_ASSERT(renderMesh.positions.size() == renderMesh.colors.size());
                Q_ASSERT((renderMesh.indices.size() % 3) == 0);

                const QMatrix4x4 worldMatrix = scene.worldTransform(objectId);
                const QMatrix3x3 normalMatrix = worldMatrix.normalMatrix();
                const int vertexCount = renderMesh.positions.size();
                for (int vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex) {
                    const QVector3D position = useDeformedMesh
                        ? renderMesh.positions[vertexIndex]
                        : worldMatrix * renderMesh.positions[vertexIndex];
                    const QVector3D sourceNormal = vertexIndex < renderMesh.normals.size()
                        ? renderMesh.normals[vertexIndex]
                        : QVector3D(0.0f, 1.0f, 0.0f);
                    const QVector3D normal = useDeformedMesh
                        ? sourceNormal.normalized()
                        : QVector3D(
                            normalMatrix(0, 0) * sourceNormal.x() + normalMatrix(0, 1) * sourceNormal.y() + normalMatrix(0, 2) * sourceNormal.z(),
                            normalMatrix(1, 0) * sourceNormal.x() + normalMatrix(1, 1) * sourceNormal.y() + normalMatrix(1, 2) * sourceNormal.z(),
                            normalMatrix(2, 0) * sourceNormal.x() + normalMatrix(2, 1) * sourceNormal.y() + normalMatrix(2, 2) * sourceNormal.z()).normalized();
                    const QVector3D color = vertexIndex < renderMesh.colors.size()
                        ? renderMesh.colors[vertexIndex]
                        : QVector3D(0.72f, 0.74f, 0.78f);

                    data.importedVertices.append({ position, normal, color });
                }

                for (std::uint32_t index : renderMesh.indices) {
                    data.importedIndices.append(vertexOffset + index);
                }

                vertexOffset += static_cast<std::uint32_t>(renderMesh.positions.size());
            }
        }

        if (!object->isJoint()) {
            continue;
        }

        const QVector3D jointColor(0.94f, 0.82f, 0.28f);
        const QVector3D boneColor(1.0f, 0.96f, 0.50f);
        const QVector3D accentColor(0.98f, 0.72f, 0.20f);
        const QVector3D position = scene.worldTransform(objectId) * QVector3D(0.0f, 0.0f, 0.0f);
        const float markerSize = 0.11f;
        appendJointDiamond(data.jointVertices, position, jointColor, markerSize);

        for (SceneObject::Id childId : object->childIds()) {
            if (includeIds != nullptr && !includeIds->contains(childId)) {
                continue;
            }
            if (excludeIds != nullptr && excludeIds->contains(childId)) {
                continue;
            }

            const SceneObject* child = scene.findObject(childId);
            if (child == nullptr || !child->isJoint() || !child->isVisible()) {
                continue;
            }

            const QVector3D childPosition = scene.worldTransform(childId) * QVector3D(0.0f, 0.0f, 0.0f);
            appendLine(data.jointVertices, position, childPosition, boneColor);

            const QVector3D boneDirection = (childPosition - position).normalized();
            const QVector3D accentStart = position + boneDirection * (markerSize * 0.75f);
            const QVector3D accentEnd = position + boneDirection * (markerSize * 2.0f);
            appendLine(data.jointVertices, accentStart, accentEnd, accentColor);
        }
    }

    return data;
}
}

namespace ViewportRenderSceneAdapter
{
ViewportRenderSceneData buildSceneData(const Scene& scene)
{
    return buildSceneDataFiltered(scene, nullptr, nullptr);
}

ViewportRenderSceneData buildSceneDataExcludingSubtree(const Scene& scene, SceneObject::Id rootObjectId)
{
    QSet<SceneObject::Id> excludedIds;
    collectSubtreeIds(scene, rootObjectId, excludedIds);
    return buildSceneDataFiltered(scene, nullptr, &excludedIds);
}

ViewportRenderSceneData buildSubtreeSceneData(const Scene& scene, SceneObject::Id rootObjectId)
{
    QSet<SceneObject::Id> includedIds;
    collectSubtreeIds(scene, rootObjectId, includedIds);
    return buildSceneDataFiltered(scene, &includedIds, nullptr);
}
}
