#include "io/AssimpSceneDocument.h"

#include <assimp/postprocess.h>

bool AssimpSceneDocument::loadFile(const QString& filePath)
{
    errorMessage_.clear();
    scene_ = nullptr;

    const unsigned int flags = aiProcess_Triangulate
        | aiProcess_JoinIdenticalVertices
        | aiProcess_ImproveCacheLocality
        | aiProcess_GenSmoothNormals
        | aiProcess_SortByPType
        | aiProcess_ValidateDataStructure;

    scene_ = importer_.ReadFile(filePath.toStdString(), flags);
    if (scene_ == nullptr || scene_->mRootNode == nullptr) {
        errorMessage_ = QString::fromUtf8(importer_.GetErrorString());
        if (errorMessage_.isEmpty()) {
            errorMessage_ = "Assimp failed to parse the FBX scene.";
        }
        scene_ = nullptr;
        return false;
    }

    return true;
}

QString AssimpSceneDocument::errorMessage() const
{
    return errorMessage_;
}

const aiScene* AssimpSceneDocument::scene() const
{
    return scene_;
}
