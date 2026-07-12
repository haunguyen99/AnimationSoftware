#pragma once

#include <QString>

#include "scene/Scene.h"

namespace PhoenixSceneDocument
{
struct LoadResult
{
    bool success = false;
    Scene scene;
    QString errorMessage;
};

bool saveToFile(const Scene& scene, const QString& filePath, QString* errorMessage = nullptr);
LoadResult loadFromFile(const QString& filePath);
}
