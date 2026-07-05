#pragma once

#include <QString>

#include "scene/Scene.h"

struct FbxImportResult
{
    bool success = false;
    QString errorMessage;
    QString infoMessage;
    Scene scene;
    int meshCount = 0;
    int nodeCount = 0;
    int triangleCount = 0;
};

