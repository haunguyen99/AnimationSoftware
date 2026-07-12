#pragma once

#include <QString>

struct FbxImportResult;

bool validateFbxImportPath(const QString& filePath, FbxImportResult& result);
