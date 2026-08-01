#include "rigging/scene/SceneRiggingController.h"

#include "rigging/scene/JointRiggingOps.h"
#include "rigging/scene/SkinBindingOps.h"
#include "rigging/scene/SkinDeformationOps.h"

namespace SceneRiggingController
{
SceneObject::Id createJoint(Scene& scene, const QString& name, SceneObject::Id parentId)
{
    return JointRiggingOps::createJoint(scene, name, parentId);
}

bool setJointOrientation(Scene& scene, SceneObject::Id id, const QQuaternion& orientation)
{
    return JointRiggingOps::setJointOrientation(scene, id, orientation);
}

bool resetJointOrientation(Scene& scene, SceneObject::Id id)
{
    return JointRiggingOps::resetJointOrientation(scene, id);
}

bool alignJointOrientationToChild(Scene& scene, SceneObject::Id id)
{
    return JointRiggingOps::alignJointOrientationToChild(scene, id);
}

bool captureBindPose(Scene& scene, SceneObject::Id id, bool recursive)
{
    return JointRiggingOps::captureBindPose(scene, id, recursive);
}

bool bindObjectToSkeleton(Scene& scene, SceneObject::Id objectId, SceneObject::Id rootJointId)
{
    return SkinBindingOps::bindObjectToSkeleton(scene, objectId, rootJointId);
}

bool setObjectSkinBinding(Scene& scene, SceneObject::Id id, const QVector<SceneObject::Id>& jointIds, const SkinWeightTable& weights)
{
    return SkinBindingOps::setObjectSkinBinding(scene, id, jointIds, weights);
}

bool clearObjectSkinBinding(Scene& scene, SceneObject::Id id)
{
    return SkinBindingOps::clearObjectSkinBinding(scene, id);
}

bool buildDeformedMesh(const Scene& scene, SceneObject::Id objectId, int meshHandle, MeshData* deformedMesh)
{
    return SkinDeformationOps::buildDeformedMesh(scene, objectId, meshHandle, deformedMesh);
}

QMatrix4x4 bindPoseWorldTransform(const Scene& scene, SceneObject::Id id)
{
    return JointRiggingOps::bindPoseWorldTransform(scene, id);
}
}
