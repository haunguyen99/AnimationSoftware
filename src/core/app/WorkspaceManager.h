#pragma once

#include <QDockWidget>
#include <QMainWindow>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

// WorkspaceManager — owns all QDockWidget instances for EditorShell.
//
// EditorShell creates panel content widgets and registers them here.
// WorkspaceManager creates the QDockWidget wrappers, applies layout presets,
// and handles persistence via QSettings.
//
// Design: management layer over QDockWidget + QMainWindow — no custom engine.

class WorkspaceManager : public QObject
{
    Q_OBJECT

public:
    // Built-in preset names
    static constexpr const char* kPresetDefault   = "Default";
    static constexpr const char* kPresetAnimation = "Animation";
    static constexpr const char* kPresetRigging   = "Rigging";

    // Panel IDs
    // Panel IDs — also used as the base for dock objectName (id + "Dock").
    // Chosen to match the legacy dock objectNames so QMainWindow::restoreState()
    // continues to find docks by name after the WorkspaceManager refactor.
    static constexpr const char* kOutliner        = "Outliner";         // → OutlinerDock
    static constexpr const char* kChannelBox      = "Inspector";        // → InspectorDock
    static constexpr const char* kPrimitivePalette = "PolygonPrimitives"; // → PolygonPrimitivesDock
    static constexpr const char* kScriptEditor    = "ScriptEditor";     // → ScriptEditorDock
    static constexpr const char* kTimeline        = "TimeSlider";       // → TimeSliderDock
    static constexpr const char* kRangeSlider     = "RangeSlider";      // → RangeSliderDock
    static constexpr const char* kCommandLine     = "CommandLine";      // → CommandLineDock
    static constexpr const char* kGraphEditor     = "GraphEditor";      // → GraphEditorDock
    static constexpr const char* kRigPanel        = "RigPanel";         // → RigPanelDock

    explicit WorkspaceManager(QMainWindow* mainWindow, QObject* parent = nullptr);

    // Register a panel — creates a QDockWidget wrapping contentWidget.
    // compactTitleBar: hides title bar when docked, shows minimal bar when floating
    //   (and re-applies bottom panel layout when un-floated).
    void registerPanel(
        const QString& id,
        const QString& displayName,
        QWidget* contentWidget,
        Qt::DockWidgetArea defaultArea,
        QDockWidget::DockWidgetFeatures features,
        Qt::DockWidgetAreas allowedAreas = Qt::AllDockWidgetAreas,
        bool compactTitleBar = false);

    // Dock access by panel ID. Returns nullptr for unknown IDs.
    QDockWidget* dock(const QString& id) const;

    // Apply initial layout after all panels are registered.
    // Should be called once from EditorShell::createDocks() after all registerPanel() calls.
    void applyInitialLayout();

    // Re-apply the default bottom-panel stacking.
    // Called when a compact-title-bar dock is un-floated (QDockWidget::topLevelChanged).
    // Also called from EditorShell::restoreBottomPanelLayout() (public for event filter).
    void restoreBottomPanelLayout();

    // Switch to a named built-in preset. Unknown names → Default.
    void applyPreset(const QString& name);

    // Reset all docks to the Default preset arrangement.
    void applyDefaultPreset();

    // Active preset name (one of kPreset* constants).
    QString activePreset() const { return activePreset_; }

    // Persistence (Step 2) — placeholders, implemented in Step 2.
    void saveSessionLayout();
    void restoreSessionLayout();

    // User-defined layouts (Step 4) — placeholders.
    QStringList userLayoutNames() const;
    void saveUserLayout(const QString& name);
    bool restoreUserLayout(const QString& name);
    void deleteUserLayout(const QString& name);

signals:
    void activePresetChanged(const QString& presetName);

private:
    struct PanelEntry
    {
        QString id;
        QString displayName;
        QDockWidget* dock = nullptr;
        Qt::DockWidgetArea defaultArea = Qt::LeftDockWidgetArea;
        bool compactTitleBar = false;
    };

    const PanelEntry* entry(const QString& id) const;
    QDockWidget* dockFor(const QString& id) const; // null-safe convenience

    void applyDefaultLayout();          // full Default preset (called by applyDefaultPreset)
    void applyAnimationLayout();        // full Animation preset
    void applyRiggingLayout();          // full Rigging preset (Default + Rig Panel tabbed on right)
    void applyDefaultBottomLayout();    // Default bottom stack (no QSignalBlocker — caller blocks)
    void applyAnimationBottomLayout();  // Animation bottom stack (larger timeline height)

    QMainWindow* mainWindow_ = nullptr;
    QMap<QString, PanelEntry> panels_;
    QVector<QString> panelOrder_; // registration order, used to rebuild layout
    QString activePreset_ = kPresetDefault;
};
