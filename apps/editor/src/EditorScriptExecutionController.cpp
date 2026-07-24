#include "EditorScriptExecutionController.h"

namespace EditorScriptExecutionController
{
QStringList linesFromDocument(const QString& documentText)
{
    return documentText.split('\n');
}

QStringList linesFromSelection(QString selectedText)
{
    selectedText.replace(QChar(0x2029), '\n');
    return selectedText.split('\n');
}

ExecutionOutput executeLines(const QStringList& lines, const ExecutionContext& context)
{
    ExecutionOutput output;
    if (!context.createScriptCommandContext || !context.executeCommand) {
        return output;
    }

    const ScriptCommandContext scriptContext = context.createScriptCommandContext();
    for (QString commandLine : lines) {
        commandLine = commandLine.trimmed();
        if (commandLine.isEmpty()) {
            continue;
        }

        output.historyLines.append(commandLine);
        if (commandLine.startsWith("//")) {
            continue;
        }

        ScriptCommandExecution execution;
        const bool handled = context.executeCommand(commandLine, scriptContext, &execution);
        if (handled && !execution.resultLine.isEmpty()) {
            output.historyLines.append(execution.resultLine);
        }
    }

    return output;
}
}
