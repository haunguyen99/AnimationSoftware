#include "core/history/EditorHistoryController.h"

EditorHistoryController::EditorHistoryController(int maxEntries)
    : maxEntries_(maxEntries)
{
}

EditorHistoryState EditorHistoryController::captureState(const Scene& scene, std::uint64_t selectedObjectId, std::uint64_t markedHierarchyParentId, int currentFrame) const
{
    EditorHistoryState state;
    state.scene = scene;
    state.selectedObjectId = selectedObjectId;
    state.markedHierarchyParentId = markedHierarchyParentId;
    state.currentFrame = currentFrame;
    return state;
}

void EditorHistoryController::recordUndoState(const EditorHistoryState& state)
{
    undoStack_.append(state);
    if (undoStack_.size() > maxEntries_) {
        undoStack_.removeFirst();
    }

    redoStack_.clear();
}

bool EditorHistoryController::tryTakeUndoState(const EditorHistoryState& currentState, EditorHistoryState* previousState)
{
    if (previousState == nullptr || undoStack_.isEmpty()) {
        return false;
    }

    redoStack_.append(currentState);
    *previousState = undoStack_.takeLast();
    return true;
}

bool EditorHistoryController::tryTakeRedoState(const EditorHistoryState& currentState, EditorHistoryState* nextState)
{
    if (nextState == nullptr || redoStack_.isEmpty()) {
        return false;
    }

    undoStack_.append(currentState);
    *nextState = redoStack_.takeLast();
    return true;
}

bool EditorHistoryController::canUndo() const
{
    return !undoStack_.isEmpty();
}

bool EditorHistoryController::canRedo() const
{
    return !redoStack_.isEmpty();
}
