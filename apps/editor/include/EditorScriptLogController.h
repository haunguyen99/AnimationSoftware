#pragma once

#include <QString>

#include "scene/PrimitiveMeshFactory.h"
#include "scene/Scene.h"

class QPlainTextEdit;

namespace EditorScriptLogController
{
void appendHistoryLine(QPlainTextEdit* historyTextEdit, const QString& line);
void appendComment(QPlainTextEdit* historyTextEdit, const QString& line);
void clearHistory(QPlainTextEdit* historyTextEdit);
void logPrimitiveCreation(QPlainTextEdit* historyTextEdit, PrimitiveMeshFactory::Type type, const QString& objectName);
void logSelection(QPlainTextEdit* historyTextEdit, const SceneObject* object);
void logSelectionCleared(QPlainTextEdit* historyTextEdit);
void logTransformChange(QPlainTextEdit* historyTextEdit, const SceneObject* object, const Transform& transform);
void logVisibilityChange(QPlainTextEdit* historyTextEdit, const SceneObject* object, bool visible);
}
