#include "engine/playback/EditorPlaybackController.h"

#include "EditorAnimationController.h"

#include <QTimer>

namespace
{
int playbackIntervalMs(int framesPerSecond)
{
    return qMax(1, 1000 / qMax(1, framesPerSecond));
}
}

EditorPlaybackController::EditorPlaybackController()
    : timer_(std::make_unique<QTimer>())
{
    timer_->setInterval(playbackIntervalMs(24));
    QObject::connect(timer_.get(), &QTimer::timeout, [this]() {
        advancePlayback();
    });
}

EditorPlaybackController::~EditorPlaybackController() = default;

void EditorPlaybackController::bind(Context context)
{
    context_ = std::move(context);
}

void EditorPlaybackController::sync(const EditorAnimationState& state)
{
    updateTimerInterval(state);
    if (state.playing) {
        timer_->start();
    } else {
        timer_->stop();
    }
}

void EditorPlaybackController::setCurrentFrame(int frame, bool logToScript)
{
    if (!isBound()) {
        return;
    }

    apply(EditorAnimationFlowController::setCurrentFrame(context_.animationState(), frame, logToScript));
}

void EditorPlaybackController::setVisibleFrameRange(int startFrame, int endFrame, bool logToScript)
{
    if (!isBound()) {
        return;
    }

    apply(EditorAnimationFlowController::setVisibleFrameRange(context_.animationState(), startFrame, endFrame, logToScript));
}

void EditorPlaybackController::setPlaybackRange(int startFrame, int endFrame, bool logToScript)
{
    if (!isBound()) {
        return;
    }

    apply(EditorAnimationFlowController::setPlaybackRange(context_.animationState(), startFrame, endFrame, logToScript));
}

void EditorPlaybackController::setFramesPerSecond(int framesPerSecond, bool logToScript)
{
    if (!isBound()) {
        return;
    }

    EditorAnimationFlowController::OperationResult result;
    result.success = true;
    result.state = EditorAnimationController::setFramesPerSecond(context_.animationState(), framesPerSecond);
    result.statusMessage = QString("Playback fps: %1").arg(result.state.framesPerSecond);
    if (logToScript) {
        result.commentLine = QString("playback fps %1").arg(result.state.framesPerSecond);
    }
    apply(std::move(result));
}

void EditorPlaybackController::stepFrame(int delta)
{
    if (!isBound()) {
        return;
    }

    apply(EditorAnimationFlowController::stepFrame(context_.animationState(), delta));
}

void EditorPlaybackController::togglePlayback()
{
    if (!isBound()) {
        return;
    }

    apply(EditorAnimationFlowController::togglePlayback(context_.animationState()));
}

void EditorPlaybackController::jumpToStart(bool logToScript)
{
    if (!isBound()) {
        return;
    }

    setCurrentFrame(context_.animationState().playbackStartFrame, logToScript);
}

void EditorPlaybackController::jumpToEnd(bool logToScript)
{
    if (!isBound()) {
        return;
    }

    setCurrentFrame(context_.animationState().playbackEndFrame, logToScript);
}

bool EditorPlaybackController::isBound() const
{
    return context_.animationState && context_.applyAnimationFlowResult;
}

void EditorPlaybackController::updateTimerInterval(const EditorAnimationState& state)
{
    timer_->setInterval(playbackIntervalMs(state.framesPerSecond));
}

void EditorPlaybackController::apply(EditorAnimationFlowController::OperationResult result)
{
    if (result.success) {
        sync(result.state);
    }

    context_.applyAnimationFlowResult(result);
}

void EditorPlaybackController::advancePlayback()
{
    if (!isBound()) {
        return;
    }

    apply(EditorAnimationFlowController::advancePlayback(context_.animationState()));
}
