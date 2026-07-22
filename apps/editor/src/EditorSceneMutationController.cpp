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
}
