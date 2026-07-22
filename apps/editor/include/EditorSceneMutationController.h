#pragma once

#include <QString>

#include "scene/Scene.h"

namespace EditorSceneMutationController
{
struct MutationResult
{
    bool success = false;
    Scene scene;
    SceneObject::Id affectedObjectId = 0;
};

MutationResult renameObject(const Scene& scene, SceneObject::Id objectId, const QString& newName);
MutationResult duplicateObject(const Scene& scene, SceneObject::Id objectId, const QString& duplicateName);
MutationResult groupObject(const Scene& scene, SceneObject::Id objectId, const QString& groupName);
MutationResult deleteObject(const Scene& scene, SceneObject::Id objectId);
MutationResult reparentObject(const Scene& scene, SceneObject::Id childId, SceneObject::Id parentId);
MutationResult bindSkin(const Scene& scene, SceneObject::Id meshId, SceneObject::Id jointId);
}
