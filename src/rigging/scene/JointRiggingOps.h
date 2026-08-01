#pragma once

#include <QMatrix4x4>
#include <QString>

#include "scene/SceneObject.h"

class QQuaternion;
class Scene;

namespace JointRiggingOps
{
SceneObject::Id createJoint(Scene& scene, const QString& name, SceneObject::Id parentId);
bool setJointOrientation(Scene& scene, SceneObject::Id id, const QQuaternion& orientation);
bool resetJointOrientation(Scene& scene, SceneObject::Id id);
bool alignJointOrientationToChild(Scene& scene, SceneObject::Id id);
bool captureBindPose(Scene& scene, SceneObject::Id id, bool recursive);
QMatrix4x4 bindPoseWorldTransform(const Scene& scene, SceneObject::Id id);
}
