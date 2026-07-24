#include "EditorSceneQueryController.h"

namespace EditorSceneQueryController
{
QString objectDisplayName(const SceneObject& object)
{
    return object.name().isEmpty() ? QString("Object_%1").arg(object.id()) : object.name();
}

SceneObject::Id findObjectIdByName(const ViewportWorkspaceWidget& viewport, const QString& objectName)
{
    for (SceneObject::Id objectId : viewport.allObjectIds()) {
        const SceneObject* object = viewport.findObject(objectId);
        if (object != nullptr && objectDisplayName(*object) == objectName) {
            return objectId;
        }
    }

    return 0;
}

QString generateUniqueScriptName(const ViewportWorkspaceWidget& viewport, const QString& prefix)
{
    int suffix = 1;
    while (findObjectIdByName(viewport, QString("%1%2").arg(prefix).arg(suffix)) != 0) {
        ++suffix;
    }
    return QString("%1%2").arg(prefix).arg(suffix);
}

QString generateUniqueObjectName(
    const ViewportWorkspaceWidget& viewport,
    const QString& baseName,
    SceneObject::Id ignoreObjectId)
{
    const auto nameInUse = [&viewport, ignoreObjectId](const QString& candidate) {
        for (SceneObject::Id objectId : viewport.allObjectIds()) {
            if (objectId == ignoreObjectId) {
                continue;
            }

            const SceneObject* object = viewport.findObject(objectId);
            if (object != nullptr && objectDisplayName(*object) == candidate) {
                return true;
            }
        }

        return false;
    };

    const QString trimmedBaseName = baseName.trimmed().isEmpty() ? QString("object") : baseName.trimmed();
    if (!nameInUse(trimmedBaseName)) {
        return trimmedBaseName;
    }

    int suffix = 1;
    while (nameInUse(QString("%1%2").arg(trimmedBaseName).arg(suffix))) {
        ++suffix;
    }

    return QString("%1%2").arg(trimmedBaseName).arg(suffix);
}
}
