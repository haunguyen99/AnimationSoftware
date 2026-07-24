#pragma once

#include <QString>

#include "ViewportWorkspaceWidget.h"

namespace EditorSceneQueryController
{
QString objectDisplayName(const SceneObject& object);
SceneObject::Id findObjectIdByName(const ViewportWorkspaceWidget& viewport, const QString& objectName);
QString generateUniqueScriptName(const ViewportWorkspaceWidget& viewport, const QString& prefix);
QString generateUniqueObjectName(
    const ViewportWorkspaceWidget& viewport,
    const QString& baseName,
    SceneObject::Id ignoreObjectId = 0);
}
