#pragma once

#include <QString>
#include <QVector>

struct EditorAnimationTimelineViewModel
{
    int visibleStartFrame = 0;
    int visibleEndFrame = 48;
    int playbackStartFrame = 0;
    int playbackEndFrame = 24;
    int framesPerSecond = 24;
    int currentFrame = 0;
    int selectedRangeStartFrame = 0;
    int selectedRangeEndFrame = 0;
    QVector<int> keyframes;
    QString objectName = "No selection";
    QString statusText = "No selection";
    QString statusStyle = "color: #bdbdbd;";
    QString setKeyStyle;
    QString deleteKeyStyle;
    QString autoKeyStyle;
    QString playPauseText = ">";
    bool hasSelection = false;
    bool currentFrameKeyed = false;
    bool hasAnyKeys = false;
    bool hasSelectedRange = false;
    bool autoKeyEnabled = false;
    bool playing = false;
};
