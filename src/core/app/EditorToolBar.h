#pragma once

class QMainWindow;
class QToolBar;
class QToolButton;

#include "EditorMenuBar.h"

// EditorToolBar — builds the main toolbar from already-created menu actions.
//
// Call build() after EditorMenuBar::build() so all Actions are populated.

namespace EditorToolBar
{

struct BuildResult
{
    QToolBar*   toolbar      = nullptr;
    QToolButton* presetButton = nullptr; // right-aligned Unity-style preset dropdown; owned by toolbar
};

BuildResult build(QMainWindow* mainWindow, const EditorMenuBar::Actions& actions);

} // namespace EditorToolBar
