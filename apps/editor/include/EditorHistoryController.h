#pragma once

#include <QVector>

#include <cstdint>

#include "scene/Scene.h"

struct EditorHistoryState
{
    Scene scene;
    std::uint64_t selectedObjectId = 0;
    std::uint64_t markedHierarchyParentId = 0;
    int currentFrame = 0;
};

class EditorHistoryController
{
public:
    explicit EditorHistoryController(int maxEntries = 100);

    EditorHistoryState captureState(const Scene& scene, std::uint64_t selectedObjectId, std::uint64_t markedHierarchyParentId, int currentFrame) const;
    void recordUndoState(const EditorHistoryState& state);
    bool tryTakeUndoState(const EditorHistoryState& currentState, EditorHistoryState* previousState);
    bool tryTakeRedoState(const EditorHistoryState& currentState, EditorHistoryState* nextState);
    bool canUndo() const;
    bool canRedo() const;

private:
    int maxEntries_ = 100;
    QVector<EditorHistoryState> undoStack_;
    QVector<EditorHistoryState> redoStack_;
};
