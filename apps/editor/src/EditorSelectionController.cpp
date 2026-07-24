#include "EditorSelectionController.h"

#include <QPlainTextEdit>
#include <QTreeWidget>

#include "EditorSceneQueryController.h"
#include "EditorScriptLogController.h"
#include "ViewportWorkspaceWidget.h"

namespace
{
using EditorSceneQueryController::objectDisplayName;
}

namespace EditorSelectionController
{
void clearSelection(Context& context, std::uint64_t& selectedObjectId, std::uint64_t markedHierarchyParentId)
{
    selectedObjectId = 0;

    if (context.viewport != nullptr) {
        context.viewport->setSelectedObject(0);
        EditorInspectorController::clearSelectionUi(
            context.inspectorWidgets,
            !context.viewport->isSceneEmpty());
    }

    EditorOutlinerController::syncSelection(context.outlinerTree, 0);
    EditorInspectorController::updateSelectionActions(
        context.inspectorActions,
        nullptr,
        nullptr,
        markedHierarchyParentId);
}

bool selectObject(
    Context& context,
    std::uint64_t objectId,
    bool syncOutliner,
    std::uint64_t& selectedObjectId,
    std::uint64_t markedHierarchyParentId)
{
    if (context.viewport == nullptr || objectId == 0) {
        clearSelection(context, selectedObjectId, markedHierarchyParentId);
        return false;
    }

    const SceneObject* object = context.viewport->findObject(objectId);
    if (object == nullptr) {
        clearSelection(context, selectedObjectId, markedHierarchyParentId);
        return false;
    }

    selectedObjectId = objectId;
    context.viewport->setSelectedObject(objectId);

    if (syncOutliner) {
        EditorOutlinerController::syncSelection(context.outlinerTree, objectId);
    }

    const SceneObject* markedObject =
        markedHierarchyParentId == 0 ? nullptr : context.viewport->findObject(markedHierarchyParentId);
    EditorInspectorController::updateSelectionActions(
        context.inspectorActions,
        object,
        markedObject,
        markedHierarchyParentId);
    EditorInspectorController::populateSelectionUi(context.inspectorWidgets, *object);
    return true;
}

bool frameSelectedObject(const Context& context, std::uint64_t selectedObjectId)
{
    if (context.viewport == nullptr || selectedObjectId == 0) {
        return false;
    }

    context.viewport->frameObject(selectedObjectId);
    const SceneObject* object = context.viewport->findObject(selectedObjectId);
    if (object == nullptr) {
        return false;
    }

    EditorScriptLogController::appendHistoryLine(
        context.scriptHistoryTextEdit,
        QString("viewFit %1;").arg(objectDisplayName(*object)));
    EditorScriptLogController::appendHistoryLine(
        context.scriptHistoryTextEdit,
        QString("// Result: framed %1 //").arg(objectDisplayName(*object)));
    return true;
}

void bindScriptCommands(
    ScriptCommandContext& scriptContext,
    Context context,
    std::uint64_t& selectedObjectId,
    std::uint64_t markedHierarchyParentId,
    const std::function<std::uint64_t(const QString&)>& findObjectIdByName)
{
    scriptContext.clearSelection = [context, &selectedObjectId, markedHierarchyParentId]() mutable {
        clearSelection(context, selectedObjectId, markedHierarchyParentId);
    };
    scriptContext.selectObjectByName =
        [context, &selectedObjectId, markedHierarchyParentId, findObjectIdByName](const QString& objectName) mutable {
            const std::uint64_t objectId = findObjectIdByName(objectName);
            if (objectId == 0) {
                return false;
            }

            return selectObject(context, objectId, true, selectedObjectId, markedHierarchyParentId);
        };
}
}
