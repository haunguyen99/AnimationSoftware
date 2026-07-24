#include "io/PhoenixSceneDocument.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>

namespace
{
QString objectKindToString(SceneObject::Kind kind)
{
    return kind == SceneObject::Kind::Joint ? "joint" : "transform";
}

SceneObject::Kind objectKindFromString(const QString& kind)
{
    return kind.compare("joint", Qt::CaseInsensitive) == 0
        ? SceneObject::Kind::Joint
        : SceneObject::Kind::Transform;
}

QJsonArray vector3ToJson(const QVector3D& value)
{
    return QJsonArray { value.x(), value.y(), value.z() };
}

QVector3D vector3FromJson(const QJsonArray& array)
{
    if (array.size() != 3) {
        return {};
    }

    return QVector3D(
        static_cast<float>(array.at(0).toDouble()),
        static_cast<float>(array.at(1).toDouble()),
        static_cast<float>(array.at(2).toDouble()));
}

QJsonObject meshToJson(const MeshData& mesh)
{
    QJsonObject object;
    QJsonArray positions;
    QJsonArray normals;
    QJsonArray colors;
    QJsonArray indices;

    for (const QVector3D& value : mesh.positions) {
        positions.append(vector3ToJson(value));
    }
    for (const QVector3D& value : mesh.normals) {
        normals.append(vector3ToJson(value));
    }
    for (const QVector3D& value : mesh.colors) {
        colors.append(vector3ToJson(value));
    }
    for (std::uint32_t index : mesh.indices) {
        indices.append(static_cast<int>(index));
    }

    object.insert("positions", positions);
    object.insert("normals", normals);
    object.insert("colors", colors);
    object.insert("indices", indices);
    object.insert("boundsMin", vector3ToJson(mesh.bounds.min()));
    object.insert("boundsMax", vector3ToJson(mesh.bounds.max()));
    return object;
}

MeshData meshFromJson(const QJsonObject& object)
{
    MeshData mesh;

    const QJsonArray positions = object.value("positions").toArray();
    const QJsonArray normals = object.value("normals").toArray();
    const QJsonArray colors = object.value("colors").toArray();
    const QJsonArray indices = object.value("indices").toArray();

    for (const QJsonValue& value : positions) {
        mesh.positions.append(vector3FromJson(value.toArray()));
    }
    for (const QJsonValue& value : normals) {
        mesh.normals.append(vector3FromJson(value.toArray()));
    }
    for (const QJsonValue& value : colors) {
        mesh.colors.append(vector3FromJson(value.toArray()));
    }
    for (const QJsonValue& value : indices) {
        mesh.indices.append(static_cast<std::uint32_t>(value.toInt()));
    }

    mesh.bounds = Bounds3D::fromMinMax(
        vector3FromJson(object.value("boundsMin").toArray()),
        vector3FromJson(object.value("boundsMax").toArray()));

    return mesh;
}

QJsonObject transformToJson(const Transform& transform)
{
    QJsonObject object;
    object.insert("translation", vector3ToJson(transform.translation));
    object.insert("rotation", QJsonArray { transform.rotation.scalar(), transform.rotation.x(), transform.rotation.y(), transform.rotation.z() });
    object.insert("scale", vector3ToJson(transform.scale));
    return object;
}

Transform transformFromJson(const QJsonObject& object)
{
    Transform transform;
    transform.translation = vector3FromJson(object.value("translation").toArray());

    const QJsonArray rotation = object.value("rotation").toArray();
    if (rotation.size() == 4) {
        transform.rotation = QQuaternion(
            static_cast<float>(rotation.at(0).toDouble()),
            static_cast<float>(rotation.at(1).toDouble()),
            static_cast<float>(rotation.at(2).toDouble()),
            static_cast<float>(rotation.at(3).toDouble()));
    }

    transform.scale = vector3FromJson(object.value("scale").toArray());
    return transform;
}

QJsonArray keyframesToJson(const TransformKeyframeTrack& keyframes)
{
    QJsonArray array;
    for (const TransformKeyframe& keyframe : keyframes) {
        QJsonObject keyframeObject;
        keyframeObject.insert("frame", keyframe.frame);
        keyframeObject.insert("transform", transformToJson(keyframe.transform));
        array.append(keyframeObject);
    }
    return array;
}

TransformKeyframeTrack keyframesFromJson(const QJsonArray& array)
{
    TransformKeyframeTrack keyframes;
    keyframes.reserve(array.size());

    for (const QJsonValue& value : array) {
        const QJsonObject keyframeObject = value.toObject();
        TransformKeyframe keyframe;
        keyframe.frame = keyframeObject.value("frame").toInt();
        keyframe.transform = transformFromJson(keyframeObject.value("transform").toObject());
        keyframes.append(keyframe);
    }

    return keyframes;
}

QJsonArray skinJointIdsToJson(const QVector<SceneObject::Id>& jointIds)
{
    QJsonArray array;
    for (SceneObject::Id jointId : jointIds) {
        array.append(static_cast<qint64>(jointId));
    }
    return array;
}

QVector<SceneObject::Id> skinJointIdsFromJson(const QJsonArray& array)
{
    QVector<SceneObject::Id> jointIds;
    jointIds.reserve(array.size());
    for (const QJsonValue& value : array) {
        jointIds.append(static_cast<SceneObject::Id>(value.toInteger()));
    }
    return jointIds;
}

QJsonArray skinWeightsToJson(const SkinWeightTable& table)
{
    QJsonArray vertices;
    for (const VertexSkinWeights& vertexWeights : table) {
        QJsonArray weights;
        for (const SkinWeight& weight : vertexWeights) {
            QJsonObject weightObject;
            weightObject.insert("jointId", static_cast<qint64>(weight.jointId));
            weightObject.insert("weight", weight.weight);
            weights.append(weightObject);
        }
        vertices.append(weights);
    }
    return vertices;
}

SkinWeightTable skinWeightsFromJson(const QJsonArray& array)
{
    SkinWeightTable table;
    table.reserve(array.size());
    for (const QJsonValue& vertexValue : array) {
        VertexSkinWeights vertexWeights;
        const QJsonArray weights = vertexValue.toArray();
        vertexWeights.reserve(weights.size());
        for (const QJsonValue& weightValue : weights) {
            const QJsonObject weightObject = weightValue.toObject();
            SkinWeight weight;
            weight.jointId = static_cast<SceneObject::Id>(weightObject.value("jointId").toInteger());
            weight.weight = static_cast<float>(weightObject.value("weight").toDouble());
            vertexWeights.append(weight);
        }
        table.append(vertexWeights);
    }
    return table;
}
}

namespace PhoenixSceneDocument
{
bool saveToFile(const Scene& scene, const QString& filePath, QString* errorMessage)
{
    QJsonObject root;
    root.insert("format", "phoenix-scene");
    root.insert("version", 5);

    QJsonArray meshes;
    const QVector<int> meshHandles = scene.allMeshHandles();
    for (int meshHandle : meshHandles) {
        const MeshData* mesh = scene.findMesh(meshHandle);
        if (mesh == nullptr) {
            continue;
        }

        QJsonObject meshObject = meshToJson(*mesh);
        meshObject.insert("handle", meshHandle);
        meshes.append(meshObject);
    }

    QVector<SceneObject::Id> objectIds = scene.allObjectIds();
    std::sort(objectIds.begin(), objectIds.end());

    QJsonArray objects;
    for (SceneObject::Id objectId : objectIds) {
        const SceneObject* object = scene.findObject(objectId);
        if (object == nullptr) {
            continue;
        }

        QJsonObject objectJson;
        objectJson.insert("id", static_cast<qint64>(object->id()));
        objectJson.insert("name", object->name());
        objectJson.insert("kind", objectKindToString(object->kind()));
        objectJson.insert("parentId", static_cast<qint64>(object->parentId()));
        objectJson.insert("visible", object->isVisible());
        objectJson.insert("transform", transformToJson(object->localTransform()));
        objectJson.insert("authoredTransform", transformToJson(object->authoredTransform()));
        objectJson.insert("jointOrientation", QJsonArray {
            object->jointOrientation().scalar(),
            object->jointOrientation().x(),
            object->jointOrientation().y(),
            object->jointOrientation().z()
        });
        objectJson.insert("hasBindPose", object->hasBindPose());
        objectJson.insert("bindPoseLocalTransform", transformToJson(object->bindPoseLocalTransform()));
        objectJson.insert("hasSkinBinding", object->hasSkinBinding());
        objectJson.insert("skinBindLocalTransform", transformToJson(object->skinBindLocalTransform()));
        objectJson.insert("skinJointIds", skinJointIdsToJson(object->skinJointIds()));
        objectJson.insert("skinWeights", skinWeightsToJson(object->skinWeights()));
        objectJson.insert("transformKeyframes", keyframesToJson(object->transformKeyframes()));
        objectJson.insert("localBoundsMin", vector3ToJson(object->localBounds().min()));
        objectJson.insert("localBoundsMax", vector3ToJson(object->localBounds().max()));

        QJsonArray meshRefs;
        for (int meshHandle : object->meshHandles()) {
            meshRefs.append(meshHandle);
        }
        objectJson.insert("meshHandles", meshRefs);
        objects.append(objectJson);
    }

    root.insert("meshes", meshes);
    root.insert("objects", objects);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage != nullptr) {
            *errorMessage = file.errorString();
        }
        return false;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

LoadResult loadFromFile(const QString& filePath)
{
    LoadResult result;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        result.errorMessage = file.errorString();
        return result;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!document.isObject()) {
        result.errorMessage = "Invalid Phoenix scene document.";
        return result;
    }

    const QJsonObject root = document.object();
    if (root.value("format").toString() != "phoenix-scene") {
        result.errorMessage = "Unsupported scene format.";
        return result;
    }

    const int version = root.value("version").toInt(1);

    QHash<int, int> meshHandleMap;
    const QJsonArray meshes = root.value("meshes").toArray();
    for (const QJsonValue& meshValue : meshes) {
        const QJsonObject meshObject = meshValue.toObject();
        const int oldHandle = meshObject.value("handle").toInt();
        const int newHandle = result.scene.addMesh(meshFromJson(meshObject));
        meshHandleMap.insert(oldHandle, newHandle);
    }

    struct PendingObject
    {
        SceneObject::Id oldId = 0;
        SceneObject::Id newId = 0;
        SceneObject::Id oldParentId = 0;
        QVector<int> oldMeshHandles;
    };

    QVector<PendingObject> pendingObjects;
    QHash<SceneObject::Id, SceneObject::Id> objectIdMap;
    const QJsonArray objects = root.value("objects").toArray();
    pendingObjects.reserve(objects.size());

    for (const QJsonValue& objectValue : objects) {
        const QJsonObject objectJson = objectValue.toObject();
        const SceneObject::Id oldId = static_cast<SceneObject::Id>(objectJson.value("id").toInteger());
        const SceneObject::Kind kind = objectKindFromString(objectJson.value("kind").toString());
        const SceneObject::Id newId = result.scene.createObject(objectJson.value("name").toString(), kind);
        objectIdMap.insert(oldId, newId);

        SceneObject* object = result.scene.findObject(newId);
        if (object == nullptr) {
            continue;
        }

        object->setVisible(objectJson.value("visible").toBool(true));
        const Transform localTransform = transformFromJson(objectJson.value("transform").toObject());
        const QJsonObject authoredTransformObject = objectJson.value("authoredTransform").toObject();
        const Transform authoredTransform = authoredTransformObject.isEmpty()
            ? localTransform
            : transformFromJson(authoredTransformObject);
        object->setAuthoredTransform(authoredTransform);
        object->setLocalTransform(localTransform);
        if (version >= 3) {
            const QJsonArray jointOrientationArray = objectJson.value("jointOrientation").toArray();
            if (jointOrientationArray.size() == 4) {
                object->setJointOrientation(QQuaternion(
                    static_cast<float>(jointOrientationArray.at(0).toDouble()),
                    static_cast<float>(jointOrientationArray.at(1).toDouble()),
                    static_cast<float>(jointOrientationArray.at(2).toDouble()),
                    static_cast<float>(jointOrientationArray.at(3).toDouble())));
            }
            object->setHasBindPose(objectJson.value("hasBindPose").toBool(false));
            object->setBindPoseLocalTransform(transformFromJson(objectJson.value("bindPoseLocalTransform").toObject()));
        } else if (object->isJoint()) {
            object->setHasBindPose(true);
            object->setBindPoseLocalTransform(localTransform);
        }
        if (version >= 4) {
            object->setHasSkinBinding(objectJson.value("hasSkinBinding").toBool(false));
            if (version >= 5) {
                object->setSkinBindLocalTransform(transformFromJson(objectJson.value("skinBindLocalTransform").toObject()));
            } else {
                object->setSkinBindLocalTransform(localTransform);
            }
            object->setSkinJointIds(skinJointIdsFromJson(objectJson.value("skinJointIds").toArray()));
            object->setSkinWeights(skinWeightsFromJson(objectJson.value("skinWeights").toArray()));
        } else {
            object->clearSkinBinding();
        }
        object->setTransformKeyframes(keyframesFromJson(objectJson.value("transformKeyframes").toArray()));
        object->setLocalBounds(Bounds3D::fromMinMax(
            vector3FromJson(objectJson.value("localBoundsMin").toArray()),
            vector3FromJson(objectJson.value("localBoundsMax").toArray())));

        PendingObject pending;
        pending.oldId = oldId;
        pending.newId = newId;
        pending.oldParentId = static_cast<SceneObject::Id>(objectJson.value("parentId").toInteger());
        const QJsonArray meshHandles = objectJson.value("meshHandles").toArray();
        for (const QJsonValue& meshHandleValue : meshHandles) {
            pending.oldMeshHandles.append(meshHandleValue.toInt());
        }
        pendingObjects.append(pending);
    }

    for (const PendingObject& pending : pendingObjects) {
        SceneObject* object = result.scene.findObject(pending.newId);
        if (object == nullptr) {
            continue;
        }

        object->setParentId(pending.oldParentId == 0 ? 0 : objectIdMap.value(pending.oldParentId, 0));
        for (int oldMeshHandle : pending.oldMeshHandles) {
            const int newHandle = meshHandleMap.value(oldMeshHandle, -1);
            if (newHandle >= 0) {
                object->addMeshHandle(newHandle);
            }
        }
    }

    for (const PendingObject& pending : pendingObjects) {
        if (pending.oldParentId == 0) {
            continue;
        }

        SceneObject* parent = result.scene.findObject(objectIdMap.value(pending.oldParentId, 0));
        if (parent != nullptr) {
            parent->addChildId(pending.newId);
        }
    }

    result.scene.rebuildWorldData();
    result.success = true;
    return result;
}
}
