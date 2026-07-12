#pragma once

#include <QString>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

class AssimpSceneDocument
{
public:
    bool loadFile(const QString& filePath);

    QString errorMessage() const;
    const aiScene* scene() const;

private:
    Assimp::Importer importer_;
    const aiScene* scene_ = nullptr;
    QString errorMessage_;
};
