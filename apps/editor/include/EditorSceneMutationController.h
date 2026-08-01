#pragma once

#include <QString>

#include <functional>

#include "core/commands/ScriptCommandSystem.h"
#include "scene/Scene.h"

namespace EditorSceneMutationController
{
struct MutationResult
{
    bool success = false;
    Scene scene;
    SceneObject::Id affectedObjectId = 0;
};

struct ScriptBindings
{
    std::function<SceneObject::Id(const QString&)> findObjectIdByName;
    std::function<QString(const QString&, SceneObject::Id)> generateUniqueObjectName;
    std::function<const SceneObject*(SceneObject::Id)> findObject;
    std::function<Scene()> sceneSnapshot;
    std::function<bool(SceneObject::Id, bool)> setObjectVisibility;
    std::function<bool(SceneObject::Id, const Transform&)> setObjectLocalTransform;
    std::function<bool(SceneObject::Id, const QQuaternion&)> setJointOrientation;
    std::function<bool(SceneObject::Id)> resetJointOrientation;
    std::function<bool(SceneObject::Id)> alignJointOrientationToChild;
    std::function<bool(SceneObject::Id, bool)> captureBindPose;
    std::function<void(const Scene&, SceneObject::Id, bool)> applySceneMutation;
    std::function<void(SceneObject::Id, bool)> applyLiveMutation;
};

MutationResult renameObject(const Scene& scene, SceneObject::Id objectId, const QString& newName);
MutationResult duplicateObject(const Scene& scene, SceneObject::Id objectId, const QString& duplicateName);
MutationResult groupObject(const Scene& scene, SceneObject::Id objectId, const QString& groupName);
MutationResult deleteObject(const Scene& scene, SceneObject::Id objectId);
MutationResult reparentObject(const Scene& scene, SceneObject::Id childId, SceneObject::Id parentId);
MutationResult bindSkin(const Scene& scene, SceneObject::Id meshId, SceneObject::Id jointId);
void bindScriptCommands(ScriptCommandContext& context, const ScriptBindings& bindings);
}
