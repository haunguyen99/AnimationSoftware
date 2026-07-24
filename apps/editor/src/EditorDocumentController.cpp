#include "EditorDocumentController.h"

#include "io/PhoenixSceneDocument.h"

namespace EditorDocumentController
{
namespace
{
Scene buildExportSceneForObject(const Scene& sourceScene, SceneObject::Id objectId);
SceneObject::Id copyObjectSubtreeToScene(const Scene& sourceScene, SceneObject::Id sourceId, Scene& targetScene, SceneObject::Id targetParentId);

Scene buildExportSceneForObject(const Scene& sourceScene, SceneObject::Id objectId)
{
    Scene exportScene;
    copyObjectSubtreeToScene(sourceScene, objectId, exportScene, 0);
    exportScene.rebuildWorldData();
    return exportScene;
}

SceneObject::Id copyObjectSubtreeToScene(const Scene& sourceScene, SceneObject::Id sourceId, Scene& targetScene, SceneObject::Id targetParentId)
{
    const SceneObject* sourceObject = sourceScene.findObject(sourceId);
    if (sourceObject == nullptr) {
        return 0;
    }

    const SceneObject::Id newId = targetScene.createObject(sourceObject->name(), sourceObject->kind());
    SceneObject* targetObject = targetScene.findObject(newId);
    if (targetObject == nullptr) {
        return 0;
    }

    targetObject->setParentId(targetParentId);
    targetObject->setVisible(sourceObject->isVisible());
    targetObject->setAuthoredTransform(sourceObject->authoredTransform());
    targetObject->setLocalTransform(sourceObject->localTransform());
    targetObject->setJointOrientation(sourceObject->jointOrientation());
    targetObject->setBindPoseLocalTransform(sourceObject->bindPoseLocalTransform());
    targetObject->setHasBindPose(sourceObject->hasBindPose());
    targetObject->setHasSkinBinding(sourceObject->hasSkinBinding());
    targetObject->setSkinJointIds(sourceObject->skinJointIds());
    targetObject->setSkinWeights(sourceObject->skinWeights());
    targetObject->setTransformKeyframes(sourceObject->transformKeyframes());
    targetObject->setLocalBounds(sourceObject->localBounds());

    for (int meshHandle : sourceObject->meshHandles()) {
        const MeshData* mesh = sourceScene.findMesh(meshHandle);
        if (mesh == nullptr) {
            continue;
        }

        targetObject->addMeshHandle(targetScene.addMesh(*mesh));
    }

    for (SceneObject::Id childId : sourceObject->childIds()) {
        const SceneObject::Id newChildId = copyObjectSubtreeToScene(sourceScene, childId, targetScene, newId);
        if (newChildId != 0) {
            targetObject->addChildId(newChildId);
        }
    }

    return newId;
}
}

LoadSceneResult loadScene(const QString& filePath)
{
    const PhoenixSceneDocument::LoadResult loadResult = PhoenixSceneDocument::loadFromFile(filePath);
    LoadSceneResult result;
    result.success = loadResult.success;
    result.scene = loadResult.scene;
    result.errorMessage = loadResult.errorMessage;
    return result;
}

OperationResult saveScene(const Scene& scene, const QString& filePath)
{
    OperationResult result;
    result.success = PhoenixSceneDocument::saveToFile(scene, filePath, &result.errorMessage);
    return result;
}

OperationResult exportAll(const Scene& scene, const QString& filePath)
{
    return saveScene(scene, filePath);
}

OperationResult exportSelection(const Scene& scene, SceneObject::Id objectId, const QString& filePath)
{
    if (objectId == 0 || !scene.contains(objectId)) {
        return OperationResult { false, "Selected object is not valid for export." };
    }

    const Scene exportScene = buildExportSceneForObject(scene, objectId);
    return saveScene(exportScene, filePath);
}

void bindScriptCommands(ScriptCommandContext& context, const ScriptBindings& bindings)
{
    context.newScene = [bindings]() {
        bindings.newScene();
    };
    context.openSceneFile = [bindings](const QString& filePath) {
        return bindings.openSceneFile(filePath);
    };
    context.importSceneFile = [bindings](const QString& filePath) {
        return bindings.importSceneFile(filePath);
    };
    context.saveSceneFile = [bindings](const QString& filePath) {
        return bindings.saveSceneFile(filePath);
    };
}
}
