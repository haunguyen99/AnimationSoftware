#pragma once

#include <cstdint>

#include "EditorInspectorController.h"
#include "EditorOutlinerController.h"
#include "ScriptCommandSystem.h"

class QPlainTextEdit;
class QTreeWidget;
class ViewportWorkspaceWidget;

namespace EditorSelectionController
{
struct Context
{
    ViewportWorkspaceWidget* viewport = nullptr;
    QTreeWidget* outlinerTree = nullptr;
    QPlainTextEdit* scriptHistoryTextEdit = nullptr;
    EditorInspectorController::InspectorWidgets inspectorWidgets;
    EditorInspectorController::InspectorActions inspectorActions;
    EditorOutlinerController::SceneAccess outlinerSceneAccess;
};

void clearSelection(Context& context, std::uint64_t& selectedObjectId, std::uint64_t markedHierarchyParentId);
bool selectObject(
    Context& context,
    std::uint64_t objectId,
    bool syncOutliner,
    std::uint64_t& selectedObjectId,
    std::uint64_t markedHierarchyParentId);
bool frameSelectedObject(const Context& context, std::uint64_t selectedObjectId);
void bindScriptCommands(
    ScriptCommandContext& scriptContext,
    Context context,
    std::uint64_t& selectedObjectId,
    std::uint64_t markedHierarchyParentId,
    const std::function<std::uint64_t(const QString&)>& findObjectIdByName);
}
