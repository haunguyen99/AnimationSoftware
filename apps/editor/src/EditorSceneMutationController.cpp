#include "EditorSceneMutationController.h"

namespace EditorSceneMutationController
{
MutationResult renameObject(const Scene& scene, SceneObject::Id objectId, const QString& newName)
{
    MutationResult result;
    result.scene = scene;
    if (!result.scene.setObjectName(objectId, newName)) {
        return result;
    }

    result.success = true;
    result.affectedObjectId = objectId;
    return result;
}

MutationResult duplicateObject(const Scene& scene, SceneObject::Id objectId, const QString& duplicateName)
{
    MutationResult result;
    result.scene = scene;

    const SceneObject* sourceObject = result.scene.findObject(objectId);
    if (sourceObject == nullptr) {
        return result;
    }

    const SceneObject::Id duplicateId = result.scene.duplicateSubtree(objectId);
    SceneObject* duplicateObject = result.scene.findObject(duplicateId);
    if (duplicateObject == nullptr) {
        return result;
    }

    duplicateObject->setName(duplicateName);
    result.scene.rebuildWorldData();

    result.success = true;
    result.affectedObjectId = duplicateId;
    return result;
}

MutationResult groupObject(const Scene& scene, SceneObject::Id objectId, const QString& groupName)
{
    MutationResult result;
    result.scene = scene;

    const SceneObject* sourceObject = result.scene.findObject(objectId);
    if (sourceObject == nullptr) {
        return result;
    }

    const SceneObject::Id sourceParentId = sourceObject->parentId();
    const SceneObject::Id groupId = result.scene.createObject(groupName);
    SceneObject* groupObject = result.scene.findObject(groupId);
    if (groupObject == nullptr) {
        return result;
    }

    groupObject->setAuthoredTransform(Transform());
    groupObject->setLocalTransform(Transform());
    groupObject->setVisible(true);
    if (!result.scene.reparentObject(groupId, sourceParentId)) {
        return result;
    }
    if (!result.scene.reparentObject(objectId, groupId)) {
        return result;
    }

    result.success = true;
    result.affectedObjectId = groupId;
    return result;
}

MutationResult deleteObject(const Scene& scene, SceneObject::Id objectId)
{
    MutationResult result;
    result.scene = scene;
    if (!result.scene.removeObject(objectId)) {
        return result;
    }

    result.success = true;
    return result;
}

MutationResult reparentObject(const Scene& scene, SceneObject::Id childId, SceneObject::Id parentId)
{
    MutationResult result;
    result.scene = scene;
    if (!result.scene.reparentObject(childId, parentId)) {
        return result;
    }

    result.success = true;
    result.affectedObjectId = childId;
    return result;
}

MutationResult bindSkin(const Scene& scene, SceneObject::Id meshId, SceneObject::Id jointId)
{
    MutationResult result;
    result.scene = scene;
    if (!result.scene.bindObjectToSkeleton(meshId, jointId)) {
        return result;
    }

    result.success = true;
    result.affectedObjectId = meshId;
    return result;
}

void bindScriptCommands(ScriptCommandContext& context, const ScriptBindings& bindings)
{
    context.renameObject = [bindings](const QString& sourceName, const QString& newName) {
        const SceneObject::Id objectId = bindings.findObjectIdByName(sourceName);
        if (objectId == 0 || newName.trimmed().isEmpty()) {
            return QString();
        }

        const QString uniqueName = bindings.generateUniqueObjectName(newName.trimmed(), objectId);
        const MutationResult mutation = renameObject(bindings.sceneSnapshot(), objectId, uniqueName);
        if (!mutation.success) {
            return QString();
        }

        bindings.applySceneMutation(mutation.scene, objectId, false);
        return uniqueName;
    };
    context.duplicateObject = [bindings](const QString& sourceName) {
        const SceneObject::Id objectId = bindings.findObjectIdByName(sourceName);
        if (objectId == 0) {
            return QString();
        }

        const SceneObject* sourceObject = bindings.findObject(objectId);
        if (sourceObject == nullptr) {
            return QString();
        }

        const QString duplicateName = bindings.generateUniqueObjectName(
            QString("%1Copy").arg(sourceObject->name()),
            0);
        const MutationResult mutation = duplicateObject(bindings.sceneSnapshot(), objectId, duplicateName);
        if (!mutation.success) {
            return QString();
        }

        bindings.applySceneMutation(mutation.scene, mutation.affectedObjectId, false);
        return duplicateName;
    };
    context.groupObject = [bindings](const QString& sourceName) {
        const SceneObject::Id objectId = bindings.findObjectIdByName(sourceName);
        if (objectId == 0) {
            return QString();
        }

        const QString groupName = bindings.generateUniqueObjectName("group", 0);
        const MutationResult mutation = groupObject(bindings.sceneSnapshot(), objectId, groupName);
        if (!mutation.success) {
            return QString();
        }

        bindings.applySceneMutation(mutation.scene, mutation.affectedObjectId, false);
        return groupName;
    };
    context.deleteObject = [bindings](const QString& sourceName) {
        const SceneObject::Id objectId = bindings.findObjectIdByName(sourceName);
        if (objectId == 0) {
            return false;
        }

        const MutationResult mutation = deleteObject(bindings.sceneSnapshot(), objectId);
        if (!mutation.success) {
            return false;
        }

        bindings.applySceneMutation(mutation.scene, 0, true);
        return true;
    };
    context.parentObject = [bindings](const QString& childName, const QString& parentName) {
        const SceneObject::Id childId = bindings.findObjectIdByName(childName);
        const SceneObject::Id parentId = bindings.findObjectIdByName(parentName);
        if (childId == 0 || parentId == 0) {
            return false;
        }

        const MutationResult mutation = reparentObject(bindings.sceneSnapshot(), childId, parentId);
        if (!mutation.success) {
            return false;
        }

        bindings.applySceneMutation(mutation.scene, childId, false);
        return true;
    };
    context.unparentObject = [bindings](const QString& childName) {
        const SceneObject::Id childId = bindings.findObjectIdByName(childName);
        if (childId == 0) {
            return false;
        }

        const MutationResult mutation = reparentObject(bindings.sceneSnapshot(), childId, 0);
        if (!mutation.success) {
            return false;
        }

        bindings.applySceneMutation(mutation.scene, childId, false);
        return true;
    };
    context.bindSkin = [bindings](const QString& meshName, const QString& jointName) {
        const SceneObject::Id meshId = bindings.findObjectIdByName(meshName);
        const SceneObject::Id jointId = bindings.findObjectIdByName(jointName);
        if (meshId == 0 || jointId == 0) {
            return false;
        }

        const MutationResult mutation = bindSkin(bindings.sceneSnapshot(), meshId, jointId);
        if (!mutation.success) {
            return false;
        }

        bindings.applySceneMutation(mutation.scene, meshId, false);
        return true;
    };
    context.setJointOrientation = [bindings](const QString& objectName, const QVector3D& eulerDegrees) {
        const SceneObject::Id objectId = bindings.findObjectIdByName(objectName);
        if (objectId == 0) {
            return false;
        }

        if (!bindings.setJointOrientation(objectId, QQuaternion::fromEulerAngles(eulerDegrees))) {
            return false;
        }

        bindings.applyLiveMutation(objectId, false);
        return true;
    };
    context.resetJointOrientation = [bindings](const QString& objectName) {
        const SceneObject::Id objectId = bindings.findObjectIdByName(objectName);
        if (objectId == 0) {
            return false;
        }

        if (!bindings.resetJointOrientation(objectId)) {
            return false;
        }

        bindings.applyLiveMutation(objectId, false);
        return true;
    };
    context.alignJointOrientationToChild = [bindings](const QString& objectName) {
        const SceneObject::Id objectId = bindings.findObjectIdByName(objectName);
        if (objectId == 0) {
            return false;
        }

        if (!bindings.alignJointOrientationToChild(objectId)) {
            return false;
        }

        bindings.applyLiveMutation(objectId, false);
        return true;
    };
    context.captureBindPose = [bindings](const QString& objectName, bool recursive) {
        const SceneObject::Id objectId = bindings.findObjectIdByName(objectName);
        if (objectId == 0) {
            return false;
        }

        if (!bindings.captureBindPose(objectId, recursive)) {
            return false;
        }

        bindings.applyLiveMutation(objectId, false);
        return true;
    };
    context.setAttribute = [bindings](const QString& objectName, const QString& attributeName, const QList<double>& values) {
        const SceneObject::Id objectId = bindings.findObjectIdByName(objectName);
        if (objectId == 0) {
            return false;
        }

        const SceneObject* object = bindings.findObject(objectId);
        if (object == nullptr) {
            return false;
        }

        if (attributeName == "visibility") {
            if (values.size() != 1) {
                return false;
            }

            if (!bindings.setObjectVisibility(objectId, values.first() != 0.0)) {
                return false;
            }

            bindings.applyLiveMutation(objectId, false);
            return true;
        }

        if (values.size() != 3) {
            return false;
        }

        if (attributeName == "jointOrient") {
            if (!bindings.setJointOrientation(
                    objectId,
                    QQuaternion::fromEulerAngles(
                        static_cast<float>(values.at(0)),
                        static_cast<float>(values.at(1)),
                        static_cast<float>(values.at(2))))) {
                return false;
            }

            bindings.applyLiveMutation(objectId, false);
            return true;
        }

        Transform transform = object->localTransform();
        const QVector3D vectorValue(
            static_cast<float>(values.at(0)),
            static_cast<float>(values.at(1)),
            static_cast<float>(values.at(2)));

        if (attributeName == "translate") {
            transform.translation = vectorValue;
        } else if (attributeName == "rotate") {
            transform.rotation = QQuaternion::fromEulerAngles(vectorValue);
        } else if (attributeName == "scale") {
            transform.scale = vectorValue;
        } else {
            return false;
        }

        if (!bindings.setObjectLocalTransform(objectId, transform)) {
            return false;
        }

        bindings.applyLiveMutation(objectId, false);
        return true;
    };
}
}
