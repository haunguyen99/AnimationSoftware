#pragma once

struct aiScene;
struct FbxImportResult;

void buildSceneFromAssimp(const aiScene* sourceScene, FbxImportResult& result);
