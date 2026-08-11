#include "WorkspaceManager.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSettings>
#include <QSignalBlocker>
#include <QWidget>

namespace
{
constexpr auto kSettingsOrganization = "ProjectPhoenix";
constexpr auto kSettingsApplication  = "PhoenixEditor";
constexpr auto kActivePresetKey      = "workspace/activePreset";
constexpr auto kUserLayoutsGroup     = "workspace/userLayouts";
}

// ── Dock title-bar helpers (anonymous namespace) ─────────────────────────────

namespace
{

void hideDockTitleBar(QDockWidget* dock)
{
    if (dock == nullptr) {
        return;
    }
    QWidget* titleBar = new QWidget(dock);
    titleBar->setFixedHeight(0);
    titleBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    dock->setTitleBarWidget(titleBar);
}

QWidget* createFloatingDockTitleBar(QDockWidget* dock)
{
    if (dock == nullptr) {
        return nullptr;
    }
    QWidget* titleBar = new QWidget(dock);
    titleBar->setObjectName("floatingDockTitleBar");
    titleBar->setFixedHeight(26);

    QHBoxLayout* layout = new QHBoxLayout(titleBar);
    layout->setContentsMargins(8, 3, 8, 3);
    layout->setSpacing(6);

    QLabel* label = new QLabel(dock->windowTitle(), titleBar);
    label->setStyleSheet("color: #eef2f7; font-weight: 600;");
    layout->addWidget(label, 1);

    return titleBar;
}

void updateDockTitleBarForFloatingState(QDockWidget* dock)
{
    if (dock == nullptr) {
        return;
    }
    if (dock->isFloating()) {
        dock->setTitleBarWidget(createFloatingDockTitleBar(dock));
        return;
    }
    hideDockTitleBar(dock);
}

} // namespace

// ── WorkspaceManager ─────────────────────────────────────────────────────────

WorkspaceManager::WorkspaceManager(QMainWindow* mainWindow, QObject* parent)
    : QObject(parent)
    , mainWindow_(mainWindow)
{
}

void WorkspaceManager::registerPanel(
    const QString& id,
    const QString& displayName,
    QWidget* contentWidget,
    Qt::DockWidgetArea defaultArea,
    QDockWidget::DockWidgetFeatures features,
    Qt::DockWidgetAreas allowedAreas,
    bool compactTitleBar)
{
    auto* dock = new QDockWidget(displayName, mainWindow_);
    dock->setObjectName(id + "Dock");
    dock->setAllowedAreas(allowedAreas);
    dock->setFeatures(features);
    dock->setWidget(contentWidget);

    if (compactTitleBar) {
        updateDockTitleBarForFloatingState(dock);
        QObject::connect(dock, &QDockWidget::topLevelChanged, this, [this, dock](bool floating) {
            updateDockTitleBarForFloatingState(dock);
            if (!floating) {
                restoreBottomPanelLayout();
            }
        });
    }

    PanelEntry entry;
    entry.id = id;
    entry.displayName = displayName;
    entry.dock = dock;
    entry.defaultArea = defaultArea;
    entry.compactTitleBar = compactTitleBar;

    panels_.insert(id, entry);
    panelOrder_.append(id);
}

QDockWidget* WorkspaceManager::dock(const QString& id) const
{
    auto it = panels_.find(id);
    return it != panels_.end() ? it->dock : nullptr;
}

const WorkspaceManager::PanelEntry* WorkspaceManager::entry(const QString& id) const
{
    auto it = panels_.find(id);
    return it != panels_.end() ? &*it : nullptr;
}

QDockWidget* WorkspaceManager::dockFor(const QString& id) const
{
    return dock(id);
}

// ── Layout ───────────────────────────────────────────────────────────────────

void WorkspaceManager::applyInitialLayout()
{
    // Side panels
    auto* outliner   = dockFor(kOutliner);
    auto* channelBox = dockFor(kChannelBox);
    if (outliner != nullptr) {
        mainWindow_->addDockWidget(Qt::LeftDockWidgetArea, outliner);
    }
    if (channelBox != nullptr) {
        mainWindow_->addDockWidget(Qt::RightDockWidgetArea, channelBox);
    }
    if (outliner != nullptr && channelBox != nullptr) {
        mainWindow_->resizeDocks({ outliner, channelBox }, { 280, 320 }, Qt::Horizontal);
    }

    // Bottom panel stack
    applyDefaultBottomLayout();

    // Panels that start floating + hidden
    auto* primitives = dockFor(kPrimitivePalette);
    if (primitives != nullptr) {
        mainWindow_->addDockWidget(Qt::RightDockWidgetArea, primitives);
        primitives->setFloating(true);
        primitives->hide();
    }
    auto* scriptEditor = dockFor(kScriptEditor);
    if (scriptEditor != nullptr) {
        mainWindow_->addDockWidget(Qt::BottomDockWidgetArea, scriptEditor);
        scriptEditor->setFloating(true);
        scriptEditor->hide();
    }

    // Rig Panel starts hidden; shown/tabbed only in the Rigging preset.
    auto* rigPanel = dockFor(kRigPanel);
    if (rigPanel != nullptr) {
        mainWindow_->addDockWidget(Qt::RightDockWidgetArea, rigPanel);
        rigPanel->setFloating(true);
        rigPanel->hide();
    }
}

void WorkspaceManager::applyDefaultBottomLayout()
{
    // Bottom panel stacking:
    //   Timeline
    //   RangeSlider  (split below Timeline)
    //   CommandLine  (split below RangeSlider)
    //   GraphEditor  (tabbed with Timeline, hidden)

    auto* timeline    = dockFor(kTimeline);
    auto* rangeSlider = dockFor(kRangeSlider);
    auto* commandLine = dockFor(kCommandLine);
    auto* graphEditor = dockFor(kGraphEditor);

    if (timeline == nullptr) {
        return;
    }

    mainWindow_->addDockWidget(Qt::BottomDockWidgetArea, timeline);

    if (rangeSlider != nullptr) {
        mainWindow_->addDockWidget(Qt::BottomDockWidgetArea, rangeSlider);
        mainWindow_->splitDockWidget(timeline, rangeSlider, Qt::Vertical);
    }

    if (commandLine != nullptr) {
        mainWindow_->addDockWidget(Qt::BottomDockWidgetArea, commandLine);
        auto* splitAbove = (rangeSlider != nullptr) ? rangeSlider : timeline;
        mainWindow_->splitDockWidget(splitAbove, commandLine, Qt::Vertical);
    }

    if (graphEditor != nullptr) {
        mainWindow_->addDockWidget(Qt::BottomDockWidgetArea, graphEditor);
        mainWindow_->tabifyDockWidget(timeline, graphEditor);
        graphEditor->hide();
    }

    if (rangeSlider != nullptr && commandLine != nullptr) {
        mainWindow_->resizeDocks({ timeline, rangeSlider, commandLine }, { 112, 56, 34 }, Qt::Vertical);
    } else if (commandLine != nullptr) {
        mainWindow_->resizeDocks({ timeline, commandLine }, { 150, 34 }, Qt::Vertical);
    } else {
        mainWindow_->resizeDocks({ timeline }, { 150 }, Qt::Vertical);
    }

    timeline->show();
    if (rangeSlider != nullptr) { rangeSlider->show(); }
    if (commandLine != nullptr) { commandLine->show(); }
    if (graphEditor != nullptr) {
        graphEditor->show();
        timeline->raise();
    }
}

void WorkspaceManager::applyDefaultLayout()
{
    auto* outliner    = dockFor(kOutliner);
    auto* channelBox  = dockFor(kChannelBox);
    auto* timeline    = dockFor(kTimeline);
    auto* rangeSlider = dockFor(kRangeSlider);
    auto* commandLine = dockFor(kCommandLine);
    auto* graphEditor = dockFor(kGraphEditor);

    if (outliner == nullptr || channelBox == nullptr || timeline == nullptr) {
        return;
    }

    // Block compact-title-bar signals while we reorganize to prevent re-entrant
    // restoreBottomPanelLayout() calls from setFloating(false) → topLevelChanged.
    const QSignalBlocker blockTime(timeline);
    const QSignalBlocker blockRange(rangeSlider); // null-safe
    const QSignalBlocker blockCmd(commandLine);   // null-safe

    outliner->setFloating(false);
    channelBox->setFloating(false);
    timeline->setFloating(false);
    if (rangeSlider != nullptr) { rangeSlider->setFloating(false); }
    if (commandLine != nullptr) { commandLine->setFloating(false); }
    if (graphEditor != nullptr) { graphEditor->setFloating(false); }

    mainWindow_->addDockWidget(Qt::LeftDockWidgetArea, outliner);
    mainWindow_->addDockWidget(Qt::RightDockWidgetArea, channelBox);
    applyDefaultBottomLayout();

    mainWindow_->resizeDocks({ outliner, channelBox }, { 280, 320 }, Qt::Horizontal);

    outliner->show();
    channelBox->show();
}

void WorkspaceManager::restoreBottomPanelLayout()
{
    auto* timeline    = dockFor(kTimeline);
    auto* rangeSlider = dockFor(kRangeSlider);
    auto* commandLine = dockFor(kCommandLine);
    auto* graphEditor = dockFor(kGraphEditor);

    if (timeline == nullptr) {
        return;
    }

    // Re-entrancy guard: setFloating(false) fires topLevelChanged synchronously
    // on the dock, which would call restoreBottomPanelLayout() again mid-execution.
    const QSignalBlocker blockTime(timeline);
    const QSignalBlocker blockRange(rangeSlider); // null-safe
    const QSignalBlocker blockCmd(commandLine);   // null-safe

    timeline->setFloating(false);
    mainWindow_->addDockWidget(Qt::BottomDockWidgetArea, timeline);

    if (rangeSlider != nullptr) {
        rangeSlider->setFloating(false);
        mainWindow_->addDockWidget(Qt::BottomDockWidgetArea, rangeSlider);
        mainWindow_->splitDockWidget(timeline, rangeSlider, Qt::Vertical);
    }

    if (commandLine != nullptr) {
        commandLine->setFloating(false);
        mainWindow_->addDockWidget(Qt::BottomDockWidgetArea, commandLine);
        auto* splitAbove = (rangeSlider != nullptr) ? rangeSlider : timeline;
        mainWindow_->splitDockWidget(splitAbove, commandLine, Qt::Vertical);
    }

    if (graphEditor != nullptr) {
        graphEditor->setFloating(false);
        mainWindow_->addDockWidget(Qt::BottomDockWidgetArea, graphEditor);
        mainWindow_->tabifyDockWidget(timeline, graphEditor);
    }

    if (rangeSlider != nullptr && commandLine != nullptr) {
        mainWindow_->resizeDocks({ timeline, rangeSlider, commandLine }, { 112, 56, 34 }, Qt::Vertical);
    } else if (commandLine != nullptr) {
        mainWindow_->resizeDocks({ timeline, commandLine }, { 150, 34 }, Qt::Vertical);
    } else {
        mainWindow_->resizeDocks({ timeline }, { 150 }, Qt::Vertical);
    }

    timeline->show();
    if (rangeSlider != nullptr) { rangeSlider->show(); }
    if (commandLine != nullptr) { commandLine->show(); }
    if (graphEditor != nullptr) {
        graphEditor->show();
        timeline->raise();
    }
}

void WorkspaceManager::applyPreset(const QString& name)
{
    if (name == kPresetAnimation) {
        applyAnimationLayout();
        activePreset_ = kPresetAnimation;
    } else if (name == kPresetRigging) {
        applyRiggingLayout();
        activePreset_ = kPresetRigging;
    } else {
        applyDefaultLayout();
        activePreset_ = kPresetDefault;
    }
    emit activePresetChanged(activePreset_);
}

// ── Rigging preset ────────────────────────────────────────────────────────────
//
// Default layout plus the Rig Panel tabbed with the Channel Box on the right.
// The Rig Panel is raised (frontmost tab) so joint tools are visible on launch.

void WorkspaceManager::applyRiggingLayout()
{
    // Start from Default, then add/reveal the Rig Panel.
    applyDefaultLayout();

    auto* channelBox = dockFor(kChannelBox);
    auto* rigPanel   = dockFor(kRigPanel);

    if (rigPanel == nullptr) {
        return; // Rig Panel not registered yet — skip silently.
    }

    rigPanel->setFloating(false);

    if (channelBox != nullptr) {
        // Tab Rig Panel with Channel Box; raise it so it's the visible tab.
        mainWindow_->tabifyDockWidget(channelBox, rigPanel);
    } else {
        mainWindow_->addDockWidget(Qt::RightDockWidgetArea, rigPanel);
    }

    rigPanel->show();
    rigPanel->raise();
}

void WorkspaceManager::applyDefaultPreset()
{
    applyDefaultLayout();
    activePreset_ = kPresetDefault;
    emit activePresetChanged(activePreset_);
}

// ── Animation preset ─────────────────────────────────────────────────────────
//
// Same panel arrangement as Default, but the bottom area is taller so the
// timeline widget has more room for keyframe editing.

void WorkspaceManager::applyAnimationBottomLayout()
{
    auto* timeline    = dockFor(kTimeline);
    auto* rangeSlider = dockFor(kRangeSlider);
    auto* commandLine = dockFor(kCommandLine);
    auto* graphEditor = dockFor(kGraphEditor);

    if (timeline == nullptr) {
        return;
    }

    mainWindow_->addDockWidget(Qt::BottomDockWidgetArea, timeline);

    if (rangeSlider != nullptr) {
        mainWindow_->addDockWidget(Qt::BottomDockWidgetArea, rangeSlider);
        mainWindow_->splitDockWidget(timeline, rangeSlider, Qt::Vertical);
    }

    if (commandLine != nullptr) {
        mainWindow_->addDockWidget(Qt::BottomDockWidgetArea, commandLine);
        auto* splitAbove = (rangeSlider != nullptr) ? rangeSlider : timeline;
        mainWindow_->splitDockWidget(splitAbove, commandLine, Qt::Vertical);
    }

    if (graphEditor != nullptr) {
        mainWindow_->addDockWidget(Qt::BottomDockWidgetArea, graphEditor);
        mainWindow_->tabifyDockWidget(timeline, graphEditor);
        graphEditor->hide();
    }

    // Animation preset: taller timeline (180 vs 112 in Default)
    if (rangeSlider != nullptr && commandLine != nullptr) {
        mainWindow_->resizeDocks({ timeline, rangeSlider, commandLine }, { 180, 56, 34 }, Qt::Vertical);
    } else if (commandLine != nullptr) {
        mainWindow_->resizeDocks({ timeline, commandLine }, { 220, 34 }, Qt::Vertical);
    } else {
        mainWindow_->resizeDocks({ timeline }, { 220 }, Qt::Vertical);
    }

    timeline->show();
    if (rangeSlider != nullptr) { rangeSlider->show(); }
    if (commandLine != nullptr) { commandLine->show(); }
    if (graphEditor != nullptr) {
        graphEditor->show();
        timeline->raise();
    }
}

void WorkspaceManager::applyAnimationLayout()
{
    auto* outliner    = dockFor(kOutliner);
    auto* channelBox  = dockFor(kChannelBox);
    auto* timeline    = dockFor(kTimeline);
    auto* rangeSlider = dockFor(kRangeSlider);
    auto* commandLine = dockFor(kCommandLine);
    auto* graphEditor = dockFor(kGraphEditor);

    if (outliner == nullptr || channelBox == nullptr || timeline == nullptr) {
        return;
    }

    const QSignalBlocker blockTime(timeline);
    const QSignalBlocker blockRange(rangeSlider);
    const QSignalBlocker blockCmd(commandLine);

    outliner->setFloating(false);
    channelBox->setFloating(false);
    timeline->setFloating(false);
    if (rangeSlider != nullptr) { rangeSlider->setFloating(false); }
    if (commandLine != nullptr) { commandLine->setFloating(false); }
    if (graphEditor != nullptr) { graphEditor->setFloating(false); }

    mainWindow_->addDockWidget(Qt::LeftDockWidgetArea, outliner);
    mainWindow_->addDockWidget(Qt::RightDockWidgetArea, channelBox);
    applyAnimationBottomLayout();

    mainWindow_->resizeDocks({ outliner, channelBox }, { 220, 260 }, Qt::Horizontal);

    outliner->show();
    channelBox->show();
}

// ── Persistence ───────────────────────────────────────────────────────────────
//
// QMainWindow::saveState() / restoreState() handle the full dock geometry.
// EditorPreferences (EditorShell::savePreferences / loadPreferences) already
// calls those.  WorkspaceManager adds only the active-preset name on top.

void WorkspaceManager::saveSessionLayout()
{
    QSettings settings(kSettingsOrganization, kSettingsApplication);
    settings.setValue(kActivePresetKey, activePreset_);
}

void WorkspaceManager::restoreSessionLayout()
{
    QSettings settings(kSettingsOrganization, kSettingsApplication);
    const QString saved = settings.value(kActivePresetKey, kPresetDefault).toString();
    // Only record the name — the actual dock geometry is already restored by
    // QMainWindow::restoreState() in EditorShell::loadPreferences().
    activePreset_ = saved;
}

// ── User layouts (Step 4) ─────────────────────────────────────────────────────

QStringList WorkspaceManager::userLayoutNames() const
{
    QSettings settings(kSettingsOrganization, kSettingsApplication);
    settings.beginGroup(kUserLayoutsGroup);
    const QStringList names = settings.childGroups();
    settings.endGroup();
    return names;
}

void WorkspaceManager::saveUserLayout(const QString& name)
{
    if (name.isEmpty() || mainWindow_ == nullptr) {
        return;
    }
    QSettings settings(kSettingsOrganization, kSettingsApplication);
    settings.beginGroup(kUserLayoutsGroup);
    settings.beginGroup(name);
    settings.setValue("state", mainWindow_->saveState());
    settings.setValue("preset", activePreset_);
    settings.endGroup();
    settings.endGroup();
}

bool WorkspaceManager::restoreUserLayout(const QString& name)
{
    if (name.isEmpty() || mainWindow_ == nullptr) {
        return false;
    }
    QSettings settings(kSettingsOrganization, kSettingsApplication);
    settings.beginGroup(kUserLayoutsGroup);
    settings.beginGroup(name);
    const QByteArray state = settings.value("state").toByteArray();
    const QString preset   = settings.value("preset", kPresetDefault).toString();
    settings.endGroup();
    settings.endGroup();

    if (state.isEmpty()) {
        return false;
    }

    mainWindow_->restoreState(state);
    activePreset_ = preset;
    emit activePresetChanged(activePreset_);
    return true;
}

void WorkspaceManager::deleteUserLayout(const QString& name)
{
    if (name.isEmpty()) {
        return;
    }
    QSettings settings(kSettingsOrganization, kSettingsApplication);
    settings.beginGroup(kUserLayoutsGroup);
    settings.remove(name);
    settings.endGroup();
}
