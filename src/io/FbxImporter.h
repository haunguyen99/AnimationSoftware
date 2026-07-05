#pragma once

#include <QString>

#include "io/FbxImportResult.h"

class FbxImporter
{
public:
    FbxImportResult importFile(const QString& filePath) const;
};

