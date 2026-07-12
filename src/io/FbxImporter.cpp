#include "io/FbxImporter.h"

#include <QFileInfo>

#include "io/AssimpSceneDocument.h"
#include "io/FbxImportMessages.h"
#include "io/FbxSceneBuilder.h"
#include "io/FbxImportValidation.h"
#include "logging/LogCategories.h"

FbxImportResult FbxImporter::importFile(const QString& filePath) const
{
    FbxImportResult result;
    const QFileInfo fileInfo(filePath);

    qCInfo(logIo) << "import request:" << filePath;

    if (!validateFbxImportPath(filePath, result)) {
        qCWarning(logIo) << "import rejected:" << filePath << result.errorMessage;
        return result;
    }

    AssimpSceneDocument document;
    if (!document.loadFile(filePath)) {
        result.errorMessage = parseFailureImportError(fileInfo.fileName(), document.errorMessage());
        qCWarning(logFbx) << "parse failed:" << filePath << result.errorMessage;
        return result;
    }

    buildSceneFromAssimp(document.scene(), result);

    result.success = true;
    result.infoMessage = formatImportSuccessMessage(
        fileInfo.fileName(),
        result.nodeCount,
        result.meshCount,
        result.triangleCount);
    qCInfo(logFbx) << "parse ok:" << result.infoMessage;

    return result;
}
