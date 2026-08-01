#include "core/settings/EditorPreferences.h"

#include <QSettings>

namespace
{
constexpr auto kSettingsOrganization = "ProjectPhoenix";
constexpr auto kSettingsApplication = "PhoenixEditor";
constexpr auto kMainWindowGeometryKey = "mainWindow/geometry";
constexpr auto kMainWindowStateKey = "mainWindow/state";
constexpr auto kAnimationAutoKeyEnabledKey = "animation/autoKeyEnabled";
}

namespace EditorPreferences
{
EditorPreferencesState read()
{
    QSettings settings(kSettingsOrganization, kSettingsApplication);
    EditorPreferencesState state;
    state.geometry = settings.value(kMainWindowGeometryKey).toByteArray();
    state.windowState = settings.value(kMainWindowStateKey).toByteArray();
    state.autoKeyEnabled = settings.value(kAnimationAutoKeyEnabledKey, false).toBool();
    return state;
}

void write(const EditorPreferencesState& state)
{
    QSettings settings(kSettingsOrganization, kSettingsApplication);
    settings.setValue(kMainWindowGeometryKey, state.geometry);
    settings.setValue(kMainWindowStateKey, state.windowState);
    settings.setValue(kAnimationAutoKeyEnabledKey, state.autoKeyEnabled);
}
}
