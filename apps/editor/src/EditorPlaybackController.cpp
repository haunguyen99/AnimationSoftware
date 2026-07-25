#include "EditorPlaybackController.h"

#include <QTimer>

namespace
{
constexpr int kPlaybackIntervalMs = 1000 / 24;
}

EditorPlaybackController::EditorPlaybackController()
    : timer_(std::make_unique<QTimer>())
{
    timer_->setInterval(kPlaybackIntervalMs);
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

void EditorPlaybackController::setPlaybackRange(int startFrame, int endFrame, bool logToScript)
{
    if (!isBound()) {
        return;
    }

    apply(EditorAnimationFlowController::setPlaybackRange(context_.animationState(), startFrame, endFrame, logToScript));
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
