#pragma once

#include <QString>

#include <functional>

#include "ScriptCommandSystem.h"
#include "scene/PrimitiveMeshFactory.h"
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
    std::function<QString(const QString&)> generateUniqueScriptName;
    std::function<QString(PrimitiveMeshFactory::Type)> primitiveScriptPrefix;
    std::function<SceneObject::Id()> selectedObjectId;
    std::function<const SceneObject*(SceneObject::Id)> findObject;
    std::function<Scene()> sceneSnapshot;
    std::function<SceneObject::Id(PrimitiveMeshFactory::Type, const QString&)> createPrimitive;
    std::function<SceneObject::Id(const QString&, SceneObject::Id)> createJoint;
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
