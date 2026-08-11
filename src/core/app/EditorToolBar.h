#pragma once

class QMainWindow;
class QToolBar;

#include "EditorMenuBar.h"

// EditorToolBar — builds the main toolbar from already-created menu actions.
//
// Call build() after EditorMenuBar::build() so all Actions are populated.

namespace EditorToolBar
{
QToolBar* build(QMainWindow* mainWindow, const EditorMenuBar::Actions& actions);
}
