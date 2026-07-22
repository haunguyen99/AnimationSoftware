#pragma once

#include <QString>

#include "scene/Scene.h"

namespace EditorDocumentController
{
struct OperationResult
{
    bool success = false;
    QString errorMessage;
};

struct LoadSceneResult
{
    bool success = false;
    Scene scene;
    QString errorMessage;
};

LoadSceneResult loadScene(const QString& filePath);
OperationResult saveScene(const Scene& scene, const QString& filePath);
OperationResult exportAll(const Scene& scene, const QString& filePath);
OperationResult exportSelection(const Scene& scene, SceneObject::Id objectId, const QString& filePath);
}
