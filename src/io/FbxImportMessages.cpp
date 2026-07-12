#include "io/FbxImportMessages.h"

QString missingFileImportError(const QString& filePath)
{
    return QString("Cannot import FBX.\n\nFile does not exist:\n%1").arg(filePath);
}

QString wrongExtensionImportError(const QString& fileName)
{
    return QString("Cannot import file.\n\nSelected file is not an FBX file:\n%1").arg(fileName);
}

QString unreadableFileImportError(const QString& fileName)
{
    return QString("Cannot import FBX.\n\nCannot open file for reading:\n%1").arg(fileName);
}

QString parseFailureImportError(const QString& fileName, const QString& reason)
{
    return QString("Cannot import FBX scene.\n\nFile: %1\nReason: %2").arg(fileName, reason);
}

QString formatImportSuccessMessage(const QString& fileName, int nodeCount, int meshCount, int triangleCount)
{
    return QString("%1 | nodes=%2 | meshes=%3 | tris=%4")
        .arg(fileName)
        .arg(nodeCount)
        .arg(meshCount)
        .arg(triangleCount);
}
