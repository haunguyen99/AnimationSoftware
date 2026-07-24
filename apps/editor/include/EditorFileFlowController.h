#pragma once

#include <QString>

namespace EditorFileFlowController
{
struct OperationResult
{
    bool success = false;
    QString targetPath;
    QString statusMessage;
    QString commentLine;
    QString commandLine;
    QString resultLine;
};

OperationResult buildNewSceneResult();
OperationResult buildOpenSceneResult(const QString& filePath);
OperationResult buildImportSceneResult(const QString& filePath);
OperationResult buildSaveSceneResult(const QString& filePath);
OperationResult buildExportAllResult(const QString& filePath);
OperationResult buildExportSelectionResult(const QString& filePath);
OperationResult buildArchiveSceneResult(const QString& archivePath);
QString buildIncrementSavePath(const QString& currentSceneFilePath, const QString& currentDirectory);
QString buildArchivePath(const QString& currentSceneFilePath, const QString& timestamp);
}
