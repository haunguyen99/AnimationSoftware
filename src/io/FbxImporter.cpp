#include "io/FbxImporter.h"

#include "logging/LogCategories.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <QDebug>
#include <QFileInfo>
#include <QFile>
#include <QMatrix4x4>
#include <QMatrix3x3>
#include <QQuaternion>
#include <QVector3D>

#include "scene/MeshData.h"

namespace
{
QVector3D transformNormal(const QMatrix3x3& matrix, const QVector3D& normal)
{
    return QVector3D(
               matrix(0, 0) * normal.x() + matrix(0, 1) * normal.y() + matrix(0, 2) * normal.z(),
               matrix(1, 0) * normal.x() + matrix(1, 1) * normal.y() + matrix(1, 2) * normal.z(),
               matrix(2, 0) * normal.x() + matrix(2, 1) * normal.y() + matrix(2, 2) * normal.z())
        .normalized();
}

Transform toTransform(const aiMatrix4x4& matrix)
{
    aiVector3D scale;
    aiQuaternion rotation;
    aiVector3D translation;
    matrix.Decompose(scale, rotation, translation);

    Transform transform;
    transform.translation = QVector3D(translation.x, translation.y, translation.z);
    transform.rotation = QQuaternion(rotation.w, rotation.x, rotation.y, rotation.z);
    transform.scale = QVector3D(scale.x, scale.y, scale.z);
    return transform;
}

QMatrix4x4 toMatrix4x4(const aiMatrix4x4& matrix)
{
    return QMatrix4x4(
        matrix.a1, matrix.a2, matrix.a3, matrix.a4,
        matrix.b1, matrix.b2, matrix.b3, matrix.b4,
        matrix.c1, matrix.c2, matrix.c3, matrix.c4,
        matrix.d1, matrix.d2, matrix.d3, matrix.d4);
}

Bounds3D transformBounds(const Bounds3D& bounds, const QMatrix4x4& matrix)
{
    if (!bounds.isValid()) {
        return bounds;
    }

    const QVector3D minPoint = bounds.min();
    const QVector3D maxPoint = bounds.max();
    const QVector<QVector3D> corners = {
        QVector3D(minPoint.x(), minPoint.y(), minPoint.z()),
        QVector3D(maxPoint.x(), minPoint.y(), minPoint.z()),
        QVector3D(minPoint.x(), maxPoint.y(), minPoint.z()),
        QVector3D(maxPoint.x(), maxPoint.y(), minPoint.z()),
        QVector3D(minPoint.x(), minPoint.y(), maxPoint.z()),
        QVector3D(maxPoint.x(), minPoint.y(), maxPoint.z()),
        QVector3D(minPoint.x(), maxPoint.y(), maxPoint.z()),
        QVector3D(maxPoint.x(), maxPoint.y(), maxPoint.z())
    };

    Bounds3D transformed;
    for (const QVector3D& corner : corners) {
        transformed.expandToInclude(matrix * corner);
    }

    return transformed;
}

void importNode(
    const aiScene* sourceScene,
    const aiNode* sourceNode,
    Scene& targetScene,
    SceneObject::Id parentId,
    const QMatrix4x4& parentWorldMatrix,
    FbxImportResult& result)
{
    Q_ASSERT(sourceScene != nullptr);
    Q_ASSERT(sourceNode != nullptr);

    const SceneObject::Id objectId = targetScene.createObject(QString::fromUtf8(sourceNode->mName.C_Str()));
    SceneObject* sceneObject = targetScene.findObject(objectId);
    if (sceneObject == nullptr) {
        Q_ASSERT_X(false, "FbxImporter::importNode", "Created SceneObject must be queryable immediately.");
        return;
    }

    sceneObject->setParentId(parentId);
    sceneObject->setLocalTransform(toTransform(sourceNode->mTransformation));

    const QMatrix4x4 localMatrix = toMatrix4x4(sourceNode->mTransformation);
    const QMatrix4x4 worldMatrix = parentWorldMatrix * localMatrix;
    const QMatrix3x3 normalMatrix = worldMatrix.normalMatrix();

    if (parentId != 0) {
        if (SceneObject* parentObject = targetScene.findObject(parentId)) {
            parentObject->addChildId(objectId);
        }
    }

    Bounds3D localBounds;

    if (sourceNode->mNumMeshes > 0) {
        for (unsigned int meshSlot = 0; meshSlot < sourceNode->mNumMeshes; ++meshSlot) {
            const aiMesh* sourceMesh = sourceScene->mMeshes[sourceNode->mMeshes[meshSlot]];
            if (sourceMesh == nullptr) {
                qCWarning(logFbx) << "null mesh pointer at node" << QString::fromUtf8(sourceNode->mName.C_Str());
                continue;
            }

            MeshData meshData;
            meshData.positions.reserve(static_cast<int>(sourceMesh->mNumVertices));
            meshData.normals.reserve(static_cast<int>(sourceMesh->mNumVertices));
            meshData.colors.reserve(static_cast<int>(sourceMesh->mNumVertices));
            Bounds3D localMeshBounds;

            for (unsigned int index = 0; index < sourceMesh->mNumVertices; ++index) {
                const aiVector3D& position = sourceMesh->mVertices[index];
                const QVector3D localPosition(position.x, position.y, position.z);
                const QVector3D worldPosition = worldMatrix * localPosition;
                meshData.positions.append(worldPosition);
                meshData.bounds.expandToInclude(worldPosition);
                localMeshBounds.expandToInclude(localPosition);

                if (sourceMesh->HasNormals()) {
                    const aiVector3D& normal = sourceMesh->mNormals[index];
                    meshData.normals.append(transformNormal(normalMatrix, QVector3D(normal.x, normal.y, normal.z)));
                } else {
                    meshData.normals.append(QVector3D(0.0f, 1.0f, 0.0f));
                }

                if (sourceMesh->HasVertexColors(0) && sourceMesh->mColors[0] != nullptr) {
                    const aiColor4D& color = sourceMesh->mColors[0][index];
                    meshData.colors.append(QVector3D(color.r, color.g, color.b));
                } else {
                    meshData.colors.append(QVector3D(0.72f, 0.74f, 0.78f));
                }
            }

            for (unsigned int faceIndex = 0; faceIndex < sourceMesh->mNumFaces; ++faceIndex) {
                const aiFace& face = sourceMesh->mFaces[faceIndex];
                if (face.mNumIndices != 3) {
                    continue;
                }

                meshData.indices.append(face.mIndices[0]);
                meshData.indices.append(face.mIndices[1]);
                meshData.indices.append(face.mIndices[2]);
            }

            const int meshHandle = targetScene.addMesh(meshData);
            sceneObject->addMeshHandle(meshHandle);

            result.meshCount += 1;
            result.triangleCount += meshData.indices.size() / 3;
            localBounds.expandToInclude(localMeshBounds);
        }
    }

    sceneObject->setLocalBounds(localBounds);
    if (localBounds.isValid()) {
        sceneObject->setWorldBounds(transformBounds(localBounds, worldMatrix));
    } else {
        sceneObject->setWorldBounds(localBounds);
    }

    result.nodeCount += 1;

    for (unsigned int childIndex = 0; childIndex < sourceNode->mNumChildren; ++childIndex) {
        importNode(sourceScene, sourceNode->mChildren[childIndex], targetScene, objectId, worldMatrix, result);
    }
}
}

FbxImportResult FbxImporter::importFile(const QString& filePath) const
{
    FbxImportResult result;
    const QFileInfo fileInfo(filePath);

    qCInfo(logIo) << "import request:" << filePath;

    if (!fileInfo.exists() || !fileInfo.isFile()) {
        result.errorMessage = "File does not exist.";
        qCWarning(logIo) << "import rejected: file does not exist:" << filePath;
        return result;
    }

    if (fileInfo.suffix().compare("fbx", Qt::CaseInsensitive) != 0) {
        result.errorMessage = "Selected file is not an FBX file.";
        qCWarning(logIo) << "import rejected: not an fbx:" << filePath;
        return result;
    }

    QFile testOpen(filePath);
    if (!testOpen.open(QIODevice::ReadOnly)) {
        result.errorMessage = "Cannot open FBX file for reading.";
        qCWarning(logIo) << "import rejected: cannot open file:" << filePath;
        return result;
    }
    testOpen.close();

    Assimp::Importer importer;
    const unsigned int flags = aiProcess_Triangulate
        | aiProcess_JoinIdenticalVertices
        | aiProcess_ImproveCacheLocality
        | aiProcess_GenSmoothNormals
        | aiProcess_SortByPType
        | aiProcess_ValidateDataStructure;

    const aiScene* importedScene = importer.ReadFile(filePath.toStdString(), flags);
    if (importedScene == nullptr || importedScene->mRootNode == nullptr) {
        result.errorMessage = QString::fromUtf8(importer.GetErrorString());
        if (result.errorMessage.isEmpty()) {
            result.errorMessage = "Assimp failed to parse the FBX scene.";
        }
        qCWarning(logFbx) << "parse failed:" << filePath << result.errorMessage;
        return result;
    }

    importNode(importedScene, importedScene->mRootNode, result.scene, 0, QMatrix4x4(), result);
    result.scene.rebuildSceneBounds();

    result.success = true;
    result.infoMessage = QString("%1 | nodes=%2 meshes=%3 tris=%4")
                             .arg(QFileInfo(filePath).fileName())
                             .arg(result.nodeCount)
                             .arg(result.meshCount)
                             .arg(result.triangleCount);
    qCInfo(logFbx) << "parse ok:" << result.infoMessage;

    return result;
}
