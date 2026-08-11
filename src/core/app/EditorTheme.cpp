#include "EditorTheme.h"

#include <QString>
#include <QWidget>

namespace EditorTheme
{

QString styleSheet()
{
    return
        "QMainWindow { background: #25282d; color: #d8dde6; }"
        "QWidget { color: #d8dde6; font-size: 12px; }"
        "QMenuBar { background: #2c3035; color: #e6ebf2; border-bottom: 1px solid #3a4047; }"
        "QMenuBar::item { background: transparent; padding: 7px 12px; }"
        "QMenuBar::item:selected { background: #3a4048; }"
        "QMenu { background: #2b2f34; color: #e6ebf2; border: 1px solid #434952; padding: 4px 0; }"
        "QMenu::item { padding: 6px 20px; }"
        "QMenu::separator { height: 1px; background: #3c424a; margin: 4px 8px; }"
        "QMenu::item:selected { background: #4e667a; }"
        "QToolBar { background: #2b2f34; border: none; spacing: 4px; padding: 5px; }"
        "QToolBar::separator { background: #3e444c; width: 1px; margin: 4px 6px; }"
        "QToolButton { background: #393f46; color: #dbe1ea; border: 1px solid #4c525b; border-radius: 4px; padding: 4px 8px; }"
        "QToolButton:hover { background: #444b54; }"
        "QToolButton:checked { background: #4f6b82; border-color: #6d8aa3; }"
        "QStatusBar { background: #2a2e33; color: #c6ccd6; border-top: 1px solid #3a4047; }"
        "QLabel#activePanelStatusLabel { color: #9fb8cf; font-weight: 600; padding: 0 8px; }"
        "QDockWidget { background: #262a2f; }"
        "QDockWidget::title { background: #32363d; color: #eef2f7; padding: 7px 10px; border-bottom: 1px solid #434952; }"
        "QDockWidget[activePanel=\"true\"]::title { background: #455766; color: #f4f7fb; border-bottom: 1px solid #6d8aa3; }"
        "QDockWidget::close-button, QDockWidget::float-button { background: transparent; border: none; padding: 2px; }"
        "QDockWidget::close-button:hover, QDockWidget::float-button:hover { background: #414750; border-radius: 3px; }"
        "QWidget[panelSurface=\"true\"] { border: 1px solid #2d3137; }"
        "QWidget[panelSurface=\"true\"][activePanel=\"true\"] { border: 1px solid #6d8aa3; }"
        "QTreeWidget, QListWidget, QPlainTextEdit { background: #23272c; color: #dde3ec; border: 1px solid #3f454d; }"
        "QHeaderView::section { background: #2f3338; color: #c9d0da; border: none; border-bottom: 1px solid #3f454d; padding: 6px 8px; }"
        "QAbstractItemView::item { padding: 4px 6px; }"
        "QAbstractItemView::item:selected { background: #536c82; color: #f3f6fa; }"
        "QAbstractItemView::item:hover { background: #343a42; }"
        "QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox { background: #30353b; color: #eef2f7; border: 1px solid #4a5058; border-radius: 4px; padding: 4px 6px; }"
        "QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus { border-color: #6d8aa3; }"
        "QLineEdit:disabled, QSpinBox:disabled, QDoubleSpinBox:disabled, QComboBox:disabled { background: #2a2e33; color: #7c848f; border-color: #3c4047; }"
        "QPushButton { background: #393f46; color: #e7ebf1; border: 1px solid #4c525b; border-radius: 4px; padding: 4px 10px; }"
        "QPushButton:hover { background: #444b53; }"
        "QPushButton:pressed { background: #2f3338; }"
        "QPushButton:disabled { background: #2b2f34; color: #7a818c; border-color: #3b4047; }"
        "QPushButton[variant=\"transport\"] { background: transparent; color: #cfd5dd; border: none; padding: 0px; }"
        "QPushButton[variant=\"transport\"]:hover { background: #3a4048; border-radius: 3px; color: #f4f7fb; }"
        "QPushButton[variant=\"transport\"]:pressed { background: #31363d; }"
        "QPushButton[tone=\"warning\"] { background: #8f6832; color: #f5f7fa; border-color: #ab8144; font-weight: 600; }"
        "QPushButton[tone=\"danger\"] { background: #6d3a3a; color: #f5f7fa; border-color: #8b4d4d; font-weight: 600; }"
        "QPushButton[tone=\"muted\"] { background: #4b5058; color: #f5f7fa; border-color: #5c636d; }"
        "QCheckBox { spacing: 6px; }"
        "QWidget#channelBoxPanel { background: #262a2f; }"
        "QLabel#inspectorEmptyStateLabel { color: #97a1ae; padding: 0 0 4px 0; }"
        "QLabel#channelObjectNameLabel { font-weight: 600; padding-bottom: 4px; color: #eef2f7; }"
        "QWidget#channelBoxSection { background: #2b2f34; border: 1px solid #3b4149; border-radius: 4px; }"
        "QLabel#channelSectionLabel { color: #c8ced7; font-size: 11px; font-weight: 600; padding: 0 0 2px 0; }"
        "QLabel#channelRowLabel { color: #9fa7b2; }"
        "QLabel#timelineStatusLabel[statusTone=\"muted\"] { color: #aeb6c1; }"
        "QLabel#timelineStatusLabel[statusTone=\"warning\"] { color: #d6b06e; font-weight: 600; }"
        "QLabel#timelineStatusLabel[statusTone=\"info\"] { color: #8eb3cf; }"
        "QWidget#commandLinePanel { background: #23272c; border-top: 1px solid #3a4047; }"
        "QLabel#commandLinePrefixLabel { color: #9ea7b2; font-size: 11px; font-weight: 600; letter-spacing: 0.4px; }"
        "QLabel#commandLineStatusLabel[statusTone=\"muted\"] { color: #aeb6c1; }"
        "QLabel#commandLineStatusLabel[statusTone=\"warning\"] { color: #d6b06e; font-weight: 600; }"
        "QLabel#commandLineStatusLabel[statusTone=\"info\"] { color: #8eb3cf; }"
        "QSpinBox[variant=\"frameBox\"] { background: #2f3338; color: #f0f3f8; border: 1px solid #4b515a; padding: 2px 6px; }"
        "QDoubleSpinBox[variant=\"channelBox\"] { background: #343941; color: #eef2f7; border: 1px solid #4a515a; border-radius: 3px; padding: 3px 6px; }"
        "QDoubleSpinBox[variant=\"channelBox\"]:focus { border-color: #6d8aa3; }"
        "QDoubleSpinBox[variant=\"channelBox\"]:disabled { background: #2a2e33; color: #7c848f; border-color: #3c4047; }"
        "QCheckBox[variant=\"channelVisibility\"] { color: #d8dde6; }"
        "QLabel[variant=\"channelMeta\"] { color: #9ea7b2; }"
        "QScrollBar:vertical { background: #272b30; width: 12px; margin: 0; }"
        "QScrollBar::handle:vertical { background: #4a515a; min-height: 24px; border-radius: 5px; }"
        "QScrollBar:horizontal { background: #272b30; height: 12px; margin: 0; }"
        "QScrollBar::handle:horizontal { background: #4a515a; min-width: 24px; border-radius: 5px; }"
        "QSplitter::handle { background: #2b2f34; }"
        "QSplitter::handle:hover { background: #3a4048; }";
}

void apply(QWidget* widget)
{
    if (widget != nullptr) {
        widget->setStyleSheet(styleSheet());
    }
}

} // namespace EditorTheme
