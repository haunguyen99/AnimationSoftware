#pragma once

struct EditorAnimationState
{
    int currentFrame = 0;
    int visibleStartFrame = 0;
    int visibleEndFrame = 48;
    int playbackStartFrame = 0;
    int playbackEndFrame = 24;
    int framesPerSecond = 24;
    int selectedRangeStartFrame = 0;
    int selectedRangeEndFrame = 0;
    bool autoKeyEnabled = false;
    bool playing = false;
    bool hasSelectedRange = false;
};
