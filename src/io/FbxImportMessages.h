#pragma once

#include <QString>

QString missingFileImportError(const QString& filePath);
QString wrongExtensionImportError(const QString& fileName);
QString unreadableFileImportError(const QString& fileName);
QString parseFailureImportError(const QString& fileName, const QString& reason);
QString formatImportSuccessMessage(const QString& fileName, int nodeCount, int meshCount, int triangleCount);
