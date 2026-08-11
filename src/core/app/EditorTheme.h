#pragma once

class QWidget;
class QString;

// EditorTheme — dark editor stylesheet.
// Call apply() once on the QMainWindow at startup.

namespace EditorTheme
{
void apply(QWidget* widget);
QString styleSheet();
}
