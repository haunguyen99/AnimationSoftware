#include "io/FbxImportValidation.h"

#include <QFile>
#include <QFileInfo>

#include "io/FbxImportMessages.h"
#include "io/FbxImportResult.h"

bool validateFbxImportPath(const QString& filePath, FbxImportResult& result)
{
    const QFileInfo fileInfo(filePath);

    if (!fileInfo.exists() || !fileInfo.isFile()) {
        result.errorMessage = missingFileImportError(filePath);
        return false;
    }

    if (fileInfo.suffix().compare("fbx", Qt::CaseInsensitive) != 0) {
        result.errorMessage = wrongExtensionImportError(fileInfo.fileName());
        return false;
    }

    QFile testOpen(filePath);
    if (!testOpen.open(QIODevice::ReadOnly)) {
        result.errorMessage = unreadableFileImportError(fileInfo.fileName());
        return false;
    }

    return true;
}
