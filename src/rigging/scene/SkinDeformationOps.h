#pragma once

#include "scene/SceneObject.h"

class MeshData;
class Scene;

namespace SkinDeformationOps
{
bool buildDeformedMesh(const Scene& scene, SceneObject::Id objectId, int meshHandle, MeshData* deformedMesh);
}
