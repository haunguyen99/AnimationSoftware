#include "engine/runtime/EditorViewportSceneController.h"

namespace
{
void refreshScene(EditorViewportSceneController::Context& context)
{
    if (context.syncSceneToRenderer) {
        context.syncSceneToRenderer();
    }
    if (context.requestRender) {
        context.requestRender();
    }
}

void notifyBeforeSceneMutation(EditorViewportSceneController::Context& context)
{
    if (context.notifyBeforeSceneMutation) {
        context.notifyBeforeSceneMutation();
    }
}

bool applyMutation(EditorViewportSceneController::Context& context, const std::function<bool()>& mutation)
{
    notifyBeforeSceneMutation(context);
    if (!mutation()) {
        return false;
    }

    refreshScene(context);
    return true;
}

void selectObject(EditorViewportSceneController::Context& context, SceneObject::Id objectId)
{
    if (context.setSelectedObject) {
        context.setSelectedObject(objectId);
        return;
    }

    context.selectedObjectId = context.scene.contains(objectId) ? objectId : 0;
}
}

namespace EditorViewportSceneController
{
void clearScene(Context context)
{
    notifyBeforeSceneMutation(context);
    context.scene.clear();
    context.selectedObjectId = 0;
    refreshScene(context);
}

void replaceScene(Context context, const Scene& scene)
{
    context.scene = scene;
    context.selectedObjectId = 0;
    refreshScene(context);
}

void appendImportedScene(Context context, const Scene& scene, bool appendToExistingScene)
{
    notifyBeforeSceneMutation(context);
    if (appendToExistingScene) {
        context.scene.appendScene(scene);
    } else {
        context.scene = scene;
    }
    refreshScene(context);
}

void optimizeSceneStorage(Context context)
{
    notifyBeforeSceneMutation(context);
    context.scene.optimizeStorage();
    refreshScene(context);
}

bool setObjectLocalTransform(Context context, SceneObject::Id objectId, const Transform& transform)
{
    return applyMutation(context, [&]() {
        return context.scene.setLocalTransform(objectId, transform, context.autoKeyEnabled);
    });
}

bool setObjectVisibility(Context context, SceneObject::Id objectId, bool visible)
{
    return applyMutation(context, [&]() {
        return context.scene.setObjectVisible(objectId, visible);
    });
}

void setCurrentFrame(Context context, int frame)
{
    context.scene.setCurrentFrame(frame);
    refreshScene(context);
}

bool setObjectKeyframe(Context context, SceneObject::Id objectId, int frame)
{
    return applyMutation(context, [&]() {
        return context.scene.setObjectKeyframe(objectId, frame);
    });
}

bool removeObjectKeyframe(Context context, SceneObject::Id objectId, int frame)
{
    return applyMutation(context, [&]() {
        return context.scene.removeObjectKeyframe(objectId, frame);
    });
}

bool setJointOrientation(Context context, SceneObject::Id objectId, const QQuaternion& orientation)
{
    return applyMutation(context, [&]() {
        return context.scene.setJointOrientation(objectId, orientation);
    });
}

bool resetJointOrientation(Context context, SceneObject::Id objectId)
{
    return applyMutation(context, [&]() {
        return context.scene.resetJointOrientation(objectId);
    });
}

bool alignJointOrientationToChild(Context context, SceneObject::Id objectId)
{
    return applyMutation(context, [&]() {
        return context.scene.alignJointOrientationToChild(objectId);
    });
}

bool captureBindPose(Context context, SceneObject::Id objectId, bool recursive)
{
    return applyMutation(context, [&]() {
        return context.scene.captureBindPose(objectId, recursive);
    });
}

SceneObject::Id createPrimitive(Context context, PrimitiveMeshFactory::Type type, const QString& name)
{
    if (!PrimitiveMeshFactory::isImplemented(type)) {
        return 0;
    }

    const MeshData meshData = PrimitiveMeshFactory::createMesh(type);
    if (meshData.positions.isEmpty() || meshData.indices.isEmpty() || !meshData.bounds.isValid()) {
        return 0;
    }

    notifyBeforeSceneMutation(context);
    const SceneObject::Id objectId = context.scene.createObject(
        name.isEmpty() ? PrimitiveMeshFactory::displayName(type) : name);
    SceneObject* object = context.scene.findObject(objectId);
    if (object == nullptr) {
        return 0;
    }

    const int meshHandle = context.scene.addMesh(meshData);
    object->addMeshHandle(meshHandle);
    object->setLocalBounds(meshData.bounds);
    context.scene.rebuildWorldData();

    refreshScene(context);
    selectObject(context, objectId);
    if (context.requestRender) {
        context.requestRender();
    }
    return objectId;
}

SceneObject::Id createJoint(Context context, const QString& name, SceneObject::Id parentId)
{
    notifyBeforeSceneMutation(context);
    const SceneObject::Id objectId = context.scene.createJoint(name, parentId);
    if (objectId == 0) {
        return 0;
    }

    refreshScene(context);
    selectObject(context, objectId);
    if (context.requestRender) {
        context.requestRender();
    }
    return objectId;
}
}
