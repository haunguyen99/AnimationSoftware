#include "EditorFileFlowController.h"

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>

namespace
{
QString fileNameForMessage(const QString& filePath)
{
    return QFileInfo(filePath).fileName();
}
}

namespace EditorFileFlowController
{
OperationResult buildNewSceneResult()
{
    OperationResult result;
    result.success = true;
    result.statusMessage = "New scene created";
    result.commandLine = "file -f -new;";
    result.resultLine = "// Result: new scene //";
    return result;
}

OperationResult buildOpenSceneResult(const QString& filePath)
{
    OperationResult result;
    result.success = true;
    result.targetPath = filePath;
    result.statusMessage = QString("Opened %1").arg(fileNameForMessage(filePath));
    result.commandLine = QString("file -o \"%1\";").arg(QDir::toNativeSeparators(filePath));
    result.resultLine = QString("// Result: opened %1 //").arg(fileNameForMessage(filePath));
    return result;
}

OperationResult buildImportSceneResult(const QString& filePath)
{
    OperationResult result;
    result.success = true;
    result.targetPath = filePath;
    result.statusMessage = QString("Imported %1").arg(fileNameForMessage(filePath));
    result.commandLine = QString("file -import \"%1\";").arg(QDir::toNativeSeparators(filePath));
    result.resultLine = QString("// Result: imported %1 //").arg(fileNameForMessage(filePath));
    return result;
}

OperationResult buildSaveSceneResult(const QString& filePath)
{
    OperationResult result;
    result.success = true;
    result.targetPath = filePath;
    result.statusMessage = QString("Saved %1").arg(fileNameForMessage(filePath));
    result.commandLine = QString("file -save \"%1\";").arg(QDir::toNativeSeparators(filePath));
    result.resultLine = QString("// Result: saved %1 //").arg(fileNameForMessage(filePath));
    return result;
}

OperationResult buildExportAllResult(const QString& filePath)
{
    OperationResult result;
    result.success = true;
    result.targetPath = filePath;
    result.statusMessage = QString("Exported %1").arg(fileNameForMessage(filePath));
    result.commandLine = QString("file -exportAll \"%1\";").arg(QDir::toNativeSeparators(filePath));
    result.resultLine = QString("// Result: exported %1 //").arg(fileNameForMessage(filePath));
    return result;
}

OperationResult buildExportSelectionResult(const QString& filePath)
{
    OperationResult result;
    result.success = true;
    result.targetPath = filePath;
    result.statusMessage = QString("Exported selection to %1").arg(fileNameForMessage(filePath));
    result.commandLine = QString("file -exportSelected \"%1\";").arg(QDir::toNativeSeparators(filePath));
    result.resultLine = QString("// Result: exported selection to %1 //").arg(fileNameForMessage(filePath));
    return result;
}

OperationResult buildArchiveSceneResult(const QString& archivePath)
{
    OperationResult result;
    result.success = true;
    result.targetPath = archivePath;
    result.statusMessage = QString("Archived to %1").arg(fileNameForMessage(archivePath));
    result.commentLine = QString("Archived scene to %1").arg(QDir::toNativeSeparators(archivePath));
    return result;
}

QString buildIncrementSavePath(const QString& currentSceneFilePath, const QString& currentDirectory)
{
    QFileInfo info(currentSceneFilePath);
    if (currentSceneFilePath.isEmpty()) {
        return QDir(currentDirectory).filePath("scene_001.phoenixscene");
    }

    const QString baseName = info.completeBaseName();
    QRegularExpression suffixPattern("^(.*?)(?:_(\\d+))?$");
    const QRegularExpressionMatch match = suffixPattern.match(baseName);
    const QString stem = match.hasMatch() ? match.captured(1) : baseName;
    const int version = match.hasMatch() && !match.captured(2).isEmpty() ? match.captured(2).toInt() + 1 : 1;
    const QString nextName = QString("%1_%2.%3")
                                 .arg(stem)
                                 .arg(version, 3, 10, QChar('0'))
                                 .arg(info.suffix().isEmpty() ? "phoenixscene" : info.suffix());
    return info.dir().filePath(nextName);
}

QString buildArchivePath(const QString& currentSceneFilePath, const QString& timestamp)
{
    QFileInfo info(currentSceneFilePath);
    const QString archiveDirPath = info.dir().filePath("archive");
    const QString archiveName = QString("%1_%2.%3")
                                    .arg(info.completeBaseName())
                                    .arg(timestamp)
                                    .arg(info.suffix());
    return QDir(archiveDirPath).filePath(archiveName);
}
}
