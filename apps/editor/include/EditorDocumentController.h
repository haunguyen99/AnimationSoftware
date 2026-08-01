#pragma once

#include <QString>

#include <functional>

#include "core/commands/ScriptCommandSystem.h"
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

struct ScriptBindings
{
    std::function<void()> newScene;
    std::function<bool(const QString&)> openSceneFile;
    std::function<bool(const QString&)> importSceneFile;
    std::function<bool(const QString&)> saveSceneFile;
};

LoadSceneResult loadScene(const QString& filePath);
OperationResult saveScene(const Scene& scene, const QString& filePath);
OperationResult exportAll(const Scene& scene, const QString& filePath);
OperationResult exportSelection(const Scene& scene, SceneObject::Id objectId, const QString& filePath);
void bindScriptCommands(ScriptCommandContext& context, const ScriptBindings& bindings);
}
