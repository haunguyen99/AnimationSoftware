#include "KeyframeTimelineWidget.h"

#include <QPaintEvent>
#include <QPainter>
#include <QtMath>

KeyframeTimelineWidget::KeyframeTimelineWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(16);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void KeyframeTimelineWidget::setFrameRange(int startFrame, int endFrame)
{
    startFrame_ = startFrame;
    endFrame_ = qMax(startFrame, endFrame);
    update();
}

void KeyframeTimelineWidget::setCurrentFrame(int frame)
{
    currentFrame_ = frame;
    update();
}

void KeyframeTimelineWidget::setKeyframes(const QVector<int>& keyframes)
{
    keyframes_ = keyframes;
    update();
}

void KeyframeTimelineWidget::setCurrentFrameKeyed(bool keyed)
{
    currentFrameKeyed_ = keyed;
    update();
}

void KeyframeTimelineWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRectF trackRect = rect().adjusted(4, 4, -4, -4);
    painter.fillRect(trackRect, QColor(58, 58, 58));

    painter.setPen(QColor(88, 88, 88));
    painter.drawRect(trackRect);

    painter.setPen(Qt::NoPen);
    for (int frame : keyframes_) {
        if (frame < startFrame_ || frame > endFrame_) {
            continue;
        }

        const qreal x = frameToPosition(frame);
        const QRectF markerRect(x - 2.5, trackRect.top() + 1.0, 5.0, trackRect.height() - 2.0);
        painter.setBrush(frame == currentFrame_ ? QColor(255, 176, 64) : QColor(104, 208, 255));
        painter.drawRoundedRect(markerRect, 1.5, 1.5);
    }

    const qreal currentX = frameToPosition(currentFrame_);
    painter.setPen(QPen(currentFrameKeyed_ ? QColor(255, 176, 64) : QColor(220, 220, 220), 1.5));
    painter.drawLine(QPointF(currentX, trackRect.top() - 1.0), QPointF(currentX, trackRect.bottom() + 1.0));
}

qreal KeyframeTimelineWidget::frameToPosition(int frame) const
{
    const QRectF trackRect = rect().adjusted(4, 4, -4, -4);
    const int frameSpan = qMax(1, endFrame_ - startFrame_);
    const qreal t = qBound(0.0, static_cast<qreal>(frame - startFrame_) / static_cast<qreal>(frameSpan), 1.0);
    return trackRect.left() + t * trackRect.width();
}
