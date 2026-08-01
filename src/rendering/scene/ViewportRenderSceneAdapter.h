#pragma once

#include <QVector>

#include "rendering/RenderTypes.h"
#include "scene/SceneObject.h"

class Scene;

struct ViewportRenderSceneData
{
    QVector<RenderVertex> importedVertices;
    QVector<std::uint32_t> importedIndices;
    QVector<RenderVertex> jointVertices;
};

namespace ViewportRenderSceneAdapter
{
ViewportRenderSceneData buildSceneData(const Scene& scene);
ViewportRenderSceneData buildSceneDataExcludingSubtree(const Scene& scene, SceneObject::Id rootObjectId);
ViewportRenderSceneData buildSubtreeSceneData(const Scene& scene, SceneObject::Id rootObjectId);
}
