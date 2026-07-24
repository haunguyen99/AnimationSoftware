#pragma once

#include <QString>
#include <QStringList>

#include <functional>

#include "ScriptCommandSystem.h"

namespace EditorScriptExecutionController
{
struct ExecutionContext
{
    std::function<ScriptCommandContext()> createScriptCommandContext;
    std::function<bool(QString, const ScriptCommandContext&, ScriptCommandExecution*)> executeCommand;
};

struct ExecutionOutput
{
    QStringList historyLines;
};

QStringList linesFromDocument(const QString& documentText);
QStringList linesFromSelection(QString selectedText);
ExecutionOutput executeLines(const QStringList& lines, const ExecutionContext& context);
}
