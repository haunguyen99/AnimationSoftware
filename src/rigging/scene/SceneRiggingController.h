#pragma once

#include <QMatrix4x4>
#include <QString>
#include <QVector>

#include "rigging/data/ObjectRigState.h"
#include "scene/MeshData.h"
#include "scene/SceneObject.h"

class Scene;

namespace SceneRiggingController
{
SceneObject::Id createJoint(Scene& scene, const QString& name, SceneObject::Id parentId);
bool setJointOrientation(Scene& scene, SceneObject::Id id, const QQuaternion& orientation);
bool resetJointOrientation(Scene& scene, SceneObject::Id id);
bool alignJointOrientationToChild(Scene& scene, SceneObject::Id id);
bool captureBindPose(Scene& scene, SceneObject::Id id, bool recursive);
bool bindObjectToSkeleton(Scene& scene, SceneObject::Id objectId, SceneObject::Id rootJointId);
bool setObjectSkinBinding(Scene& scene, SceneObject::Id id, const QVector<SceneObject::Id>& jointIds, const SkinWeightTable& weights);
bool clearObjectSkinBinding(Scene& scene, SceneObject::Id id);
bool buildDeformedMesh(const Scene& scene, SceneObject::Id objectId, int meshHandle, MeshData* deformedMesh);
QMatrix4x4 bindPoseWorldTransform(const Scene& scene, SceneObject::Id id);
}
