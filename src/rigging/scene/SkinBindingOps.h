#pragma once

#include <QVector>

#include "rigging/data/ObjectRigState.h"
#include "scene/SceneObject.h"

class Scene;

namespace SkinBindingOps
{
bool bindObjectToSkeleton(Scene& scene, SceneObject::Id objectId, SceneObject::Id rootJointId);
bool setObjectSkinBinding(Scene& scene, SceneObject::Id id, const QVector<SceneObject::Id>& jointIds, const SkinWeightTable& weights);
bool clearObjectSkinBinding(Scene& scene, SceneObject::Id id);
}
