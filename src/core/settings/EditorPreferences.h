#pragma once

#include <QByteArray>

struct EditorPreferencesState
{
    QByteArray geometry;
    QByteArray windowState;
    bool autoKeyEnabled = false;
};

namespace EditorPreferences
{
EditorPreferencesState read();
void write(const EditorPreferencesState& state);
}
