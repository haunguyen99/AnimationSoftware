#pragma once

#include <cstdint>
#include <functional>

#include <QVector>

#include "scene/SceneObject.h"

class QTreeWidget;

namespace EditorOutlinerController
{
struct SceneAccess
{
    std::function<QVector<SceneObject::Id>()> rootObjectIds;
    std::function<const SceneObject*(SceneObject::Id)> findObject;
};

void populateTree(QTreeWidget* outlinerTree, const SceneAccess& sceneAccess);
void syncSelection(QTreeWidget* outlinerTree, std::uint64_t objectId);
std::uint64_t selectedObjectId(const QTreeWidget* outlinerTree);
QString selectedObjectLabel(const QTreeWidget* outlinerTree);
}
