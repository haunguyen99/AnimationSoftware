#pragma once

#include <functional>
#include <memory>

#include "animation/editor/EditorAnimationFlowController.h"

class QTimer;

class EditorPlaybackController
{
public:
    struct Context
    {
        std::function<EditorAnimationState()> animationState;
        std::function<void(const EditorAnimationFlowController::OperationResult&)> applyAnimationFlowResult;
    };

    EditorPlaybackController();
    ~EditorPlaybackController();

    void bind(Context context);
    void sync(const EditorAnimationState& state);

    void setCurrentFrame(int frame, bool logToScript = true);
    void setVisibleFrameRange(int startFrame, int endFrame, bool logToScript = true);
    void setPlaybackRange(int startFrame, int endFrame, bool logToScript = true);
    void setFramesPerSecond(int framesPerSecond, bool logToScript = true);
    void stepFrame(int delta);
    void togglePlayback();
    void jumpToStart(bool logToScript = true);
    void jumpToEnd(bool logToScript = true);

private:
    bool isBound() const;
    void updateTimerInterval(const EditorAnimationState& state);
    void apply(EditorAnimationFlowController::OperationResult result);
    void advancePlayback();

    std::unique_ptr<QTimer> timer_;
    Context context_;
};
