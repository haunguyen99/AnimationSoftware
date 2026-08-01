#include "KeyframeTimelineWidget.h"

#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QtMath>

KeyframeTimelineWidget::KeyframeTimelineWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(42);
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

void KeyframeTimelineWidget::setSelectedFrameRange(int startFrame, int endFrame, bool hasSelection)
{
    hasSelectedRange_ = hasSelection;
    selectedRangeStartFrame_ = qMin(startFrame, endFrame);
    selectedRangeEndFrame_ = qMax(startFrame, endFrame);
    update();
}

void KeyframeTimelineWidget::setSelectionChangedCallback(std::function<void(int, int)> callback)
{
    selectionChangedCallback_ = std::move(callback);
}

void KeyframeTimelineWidget::setCurrentFrameChangedCallback(std::function<void(int)> callback)
{
    currentFrameChangedCallback_ = std::move(callback);
}

void KeyframeTimelineWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    const QRectF fullRect = rect().adjusted(2, 2, -2, -2);
    const QRectF rulerRect(fullRect.left(), fullRect.top(), fullRect.width(), 18.0);
    const QRectF trackRect(fullRect.left(), fullRect.top() + 18.0, fullRect.width(), fullRect.height() - 18.0);

    painter.fillRect(fullRect, QColor(44, 47, 52));
    painter.fillRect(rulerRect, QColor(54, 58, 64));
    painter.fillRect(trackRect, QColor(35, 38, 42));

    painter.setPen(QColor(84, 90, 98));
    painter.drawRect(fullRect);

    const int frameSpan = qMax(1, endFrame_ - startFrame_);
    const int labelStep = frameSpan <= 24 ? 1 : (frameSpan <= 48 ? 2 : (frameSpan <= 96 ? 4 : 8));
    painter.setFont(QFont("Segoe UI", 7));
    for (int frame = startFrame_; frame <= endFrame_; ++frame) {
        const qreal x = frameToPosition(frame);
        const bool majorTick = ((frame - startFrame_) % labelStep) == 0;
        painter.setPen(majorTick ? QColor(150, 156, 165) : QColor(88, 94, 101));
        painter.drawLine(
            QPointF(x, rulerRect.bottom() - (majorTick ? 11.0 : 6.0)),
            QPointF(x, trackRect.bottom() - (majorTick ? 2.0 : 5.0)));
        if (majorTick) {
            painter.setPen(QColor(217, 175, 92));
            painter.drawText(
                QRectF(x + 3.0, rulerRect.top(), 28.0, rulerRect.height() - 3.0),
                Qt::AlignLeft | Qt::AlignVCenter,
                QString::number(frame));
        }
    }

    if (hasSelectedRange_) {
        const qreal selectionStartX = frameToPosition(selectedRangeStartFrame_);
        const qreal selectionEndX = frameToPosition(selectedRangeEndFrame_);
        const QRectF selectionRect(
            qMin(selectionStartX, selectionEndX),
            trackRect.top() + 1.0,
            qAbs(selectionEndX - selectionStartX) + 1.0,
            trackRect.height() - 2.0);
        painter.fillRect(selectionRect, QColor(98, 118, 139, 82));
    }

    painter.setPen(Qt::NoPen);
    for (int frame : keyframes_) {
        if (frame < startFrame_ || frame > endFrame_) {
            continue;
        }

        const qreal x = frameToPosition(frame);
        const QRectF markerRect(x - 2.0, rulerRect.bottom() - 6.0, 4.0, trackRect.bottom() - rulerRect.bottom() - 1.0);
        painter.setBrush(frame == currentFrame_ ? QColor(255, 210, 120) : QColor(255, 82, 82));
        painter.drawRect(markerRect);
    }

    const qreal currentX = frameToPosition(currentFrame_);
    painter.setPen(QPen(currentFrameKeyed_ ? QColor(255, 192, 92) : QColor(232, 236, 240), 2.0));
    painter.drawLine(QPointF(currentX, rulerRect.top()), QPointF(currentX, trackRect.bottom() + 1.0));

    const QString currentFrameText = QString::number(currentFrame_);
    const QFontMetrics metrics(painter.font());
    const int badgeWidth = qMax(20, metrics.horizontalAdvance(currentFrameText) + 10);
    QRectF badgeRect(currentX - (badgeWidth / 2.0), rulerRect.bottom() - 1.0, badgeWidth, 14.0);
    if (badgeRect.left() < fullRect.left()) {
        badgeRect.moveLeft(fullRect.left());
    }
    if (badgeRect.right() > fullRect.right()) {
        badgeRect.moveRight(fullRect.right());
    }
    painter.setPen(Qt::NoPen);
    painter.setBrush(currentFrameKeyed_ ? QColor(152, 106, 44) : QColor(70, 75, 82));
    painter.drawRect(badgeRect);
    painter.setPen(QColor(244, 246, 249));
    painter.drawText(badgeRect, Qt::AlignCenter, currentFrameText);
}

void KeyframeTimelineWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    draggingSelection_ = true;
    dragAnchorFrame_ = positionToFrame(event->position().x());
    applySelectionFromFrame(dragAnchorFrame_);
    if (currentFrameChangedCallback_) {
        currentFrameChangedCallback_(dragAnchorFrame_);
    }
    event->accept();
}

void KeyframeTimelineWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (!draggingSelection_ || !(event->buttons() & Qt::LeftButton)) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    const int frame = positionToFrame(event->position().x());
    applySelectionFromFrame(frame);
    if (currentFrameChangedCallback_) {
        currentFrameChangedCallback_(frame);
    }
    event->accept();
}

void KeyframeTimelineWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (!draggingSelection_ || event->button() != Qt::LeftButton) {
        QWidget::mouseReleaseEvent(event);
        return;
    }

    draggingSelection_ = false;
    const int frame = positionToFrame(event->position().x());
    applySelectionFromFrame(frame);
    if (currentFrameChangedCallback_) {
        currentFrameChangedCallback_(frame);
    }
    event->accept();
}

void KeyframeTimelineWidget::applySelectionFromFrame(int frame)
{
    hasSelectedRange_ = true;
    selectedRangeStartFrame_ = qMin(dragAnchorFrame_, frame);
    selectedRangeEndFrame_ = qMax(dragAnchorFrame_, frame);
    update();

    if (selectionChangedCallback_) {
        selectionChangedCallback_(selectedRangeStartFrame_, selectedRangeEndFrame_);
    }
}

int KeyframeTimelineWidget::positionToFrame(int x) const
{
    const QRectF trackRect = rect().adjusted(4, 4, -4, -4);
    if (trackRect.width() <= 0.0) {
        return startFrame_;
    }

    const qreal t = qBound(0.0, (static_cast<qreal>(x) - trackRect.left()) / trackRect.width(), 1.0);
    const int frameSpan = qMax(1, endFrame_ - startFrame_);
    return startFrame_ + qRound(t * frameSpan);
}

qreal KeyframeTimelineWidget::frameToPosition(int frame) const
{
    const QRectF trackRect = rect().adjusted(4, 4, -4, -4);
    const int frameSpan = qMax(1, endFrame_ - startFrame_);
    const qreal t = qBound(0.0, static_cast<qreal>(frame - startFrame_) / static_cast<qreal>(frameSpan), 1.0);
    return trackRect.left() + t * trackRect.width();
}
