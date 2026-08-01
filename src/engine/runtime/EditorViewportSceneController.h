#pragma once

#include <QString>
#include <functional>

#include "scene/PrimitiveMeshFactory.h"
#include "scene/Scene.h"

namespace EditorViewportSceneController
{
struct Context
{
    Scene& scene;
    SceneObject::Id& selectedObjectId;
    bool autoKeyEnabled = false;
    std::function<void()> notifyBeforeSceneMutation;
    std::function<void()> syncSceneToRenderer;
    std::function<void()> requestRender;
    std::function<void(SceneObject::Id)> setSelectedObject;
};

void clearScene(Context context);
void replaceScene(Context context, const Scene& scene);
void appendImportedScene(Context context, const Scene& scene, bool appendToExistingScene);
void optimizeSceneStorage(Context context);

bool setObjectLocalTransform(Context context, SceneObject::Id objectId, const Transform& transform);
bool setObjectVisibility(Context context, SceneObject::Id objectId, bool visible);
void setCurrentFrame(Context context, int frame);
bool setObjectKeyframe(Context context, SceneObject::Id objectId, int frame);
bool removeObjectKeyframe(Context context, SceneObject::Id objectId, int frame);
bool setJointOrientation(Context context, SceneObject::Id objectId, const QQuaternion& orientation);
bool resetJointOrientation(Context context, SceneObject::Id objectId);
bool alignJointOrientationToChild(Context context, SceneObject::Id objectId);
bool captureBindPose(Context context, SceneObject::Id objectId, bool recursive);

SceneObject::Id createPrimitive(Context context, PrimitiveMeshFactory::Type type, const QString& name = {});
SceneObject::Id createJoint(Context context, const QString& name = {}, SceneObject::Id parentId = 0);
}
