#include "RangeSliderPanel.h"

#include <QAbstractSpinBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QVBoxLayout>

#include <QtMath>

namespace
{
constexpr int kHandleWidth = 6;

QRectF valueBadgeRect(const QFontMetrics& metrics, const QString& text, qreal x, qreal top, qreal minLeft, qreal maxRight)
{
    const qreal width = qMax(24, metrics.horizontalAdvance(text) + 8);
    QRectF rect(x - (width / 2.0), top, width, 18.0);
    if (rect.left() < minLeft) {
        rect.moveLeft(minLeft);
    }
    if (rect.right() > maxRight) {
        rect.moveRight(maxRight);
    }
    return rect;
}
}

RangeSliderTrackWidget::RangeSliderTrackWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(20);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void RangeSliderTrackWidget::setFrames(
    int visibleStartFrame,
    int visibleEndFrame,
    int playbackStartFrame,
    int playbackEndFrame,
    int currentFrame)
{
    visibleStartFrame_ = qMin(visibleStartFrame, visibleEndFrame);
    visibleEndFrame_ = qMax(visibleStartFrame, visibleEndFrame);
    playbackStartFrame_ = qMin(playbackStartFrame, playbackEndFrame);
    playbackEndFrame_ = qMax(playbackStartFrame, playbackEndFrame);
    playbackStartFrame_ = qBound(visibleStartFrame_, playbackStartFrame_, visibleEndFrame_);
    playbackEndFrame_ = qBound(visibleStartFrame_, playbackEndFrame_, visibleEndFrame_);
    currentFrame_ = currentFrame;
    update();
}

void RangeSliderTrackWidget::setPlaybackRangeChangedCallback(std::function<void(int, int)> callback)
{
    playbackRangeChangedCallback_ = std::move(callback);
}

void RangeSliderTrackWidget::setVisibleRangeChangedCallback(std::function<void(int, int)> callback)
{
    visibleRangeChangedCallback_ = std::move(callback);
}

void RangeSliderTrackWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    const QRectF outerRect = rect().adjusted(1, 3, -1, -3);
    const QRectF trackRect = outerRect.adjusted(1, 3, -1, -3);
    painter.fillRect(outerRect, QColor(49, 53, 58));
    painter.setPen(QColor(75, 81, 88));
    painter.drawRect(outerRect);

    painter.fillRect(trackRect, QColor(40, 43, 47));

    const qreal startX = frameToPosition(playbackStartFrame_);
    const qreal endX = frameToPosition(playbackEndFrame_);
    const QRectF selectionRect(qMin(startX, endX), trackRect.top(), qMax<qreal>(qAbs(endX - startX), kHandleWidth * 2), trackRect.height());
    painter.fillRect(selectionRect, QColor(112, 116, 121));

    const QRectF startHandleRect(selectionRect.left(), trackRect.top(), kHandleWidth, trackRect.height());
    const QRectF endHandleRect(selectionRect.right() - kHandleWidth, trackRect.top(), kHandleWidth, trackRect.height());
    painter.fillRect(startHandleRect, QColor(186, 189, 194));
    painter.fillRect(endHandleRect, QColor(186, 189, 194));

    const qreal currentX = frameToPosition(currentFrame_);
    painter.setPen(QPen(QColor(217, 175, 92), 2.0));
    painter.drawLine(QPointF(currentX, outerRect.top()), QPointF(currentX, outerRect.bottom()));

    painter.setFont(QFont("Segoe UI", 7));
    const QFontMetrics metrics(painter.font());
    const QRectF startBadge = valueBadgeRect(
        metrics,
        QString::number(playbackStartFrame_),
        startX + (kHandleWidth * 0.5),
        outerRect.top(),
        outerRect.left(),
        outerRect.right());
    const QRectF endBadge = valueBadgeRect(
        metrics,
        QString::number(playbackEndFrame_),
        endX - (kHandleWidth * 0.5),
        outerRect.top(),
        outerRect.left(),
        outerRect.right());
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(59, 63, 69));
    painter.drawRect(startBadge);
    painter.drawRect(endBadge);
    painter.setPen(QColor(241, 244, 247));
    painter.drawText(startBadge, Qt::AlignCenter, QString::number(playbackStartFrame_));
    painter.drawText(endBadge, Qt::AlignCenter, QString::number(playbackEndFrame_));
}

void RangeSliderTrackWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    const qreal startX = frameToPosition(playbackStartFrame_);
    const qreal endX = frameToPosition(playbackEndFrame_);
    const QRectF selectionRect(qMin(startX, endX), 0.0, qAbs(endX - startX), height());
    const QRectF startHandleRect(selectionRect.left() - 3.0, 0.0, kHandleWidth + 6.0, height());
    const QRectF endHandleRect(selectionRect.right() - kHandleWidth - 3.0, 0.0, kHandleWidth + 6.0, height());

    dragAnchorFrame_ = positionToFrame(qRound(event->position().x()));
    dragStartPlaybackStartFrame_ = playbackStartFrame_;
    dragStartPlaybackEndFrame_ = playbackEndFrame_;
    dragStartVisibleStartFrame_ = visibleStartFrame_;
    dragStartVisibleEndFrame_ = visibleEndFrame_;

    if (startHandleRect.contains(event->position())) {
        dragMode_ = DragMode::StartHandle;
    } else if (endHandleRect.contains(event->position())) {
        dragMode_ = DragMode::EndHandle;
    } else if (selectionRect.contains(event->position())) {
        dragMode_ = DragMode::MovePlaybackRange;
    } else {
        dragMode_ = DragMode::PanVisibleRange;
    }

    event->accept();
}

void RangeSliderTrackWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (dragMode_ == DragMode::None || !(event->buttons() & Qt::LeftButton)) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    const int frame = positionToFrame(qRound(event->position().x()));
    if (dragMode_ == DragMode::StartHandle) {
        playbackStartFrame_ = qMin(frame, playbackEndFrame_);
        playbackStartFrame_ = qBound(visibleStartFrame_, playbackStartFrame_, playbackEndFrame_);
        emitRangeChangedIfNeeded();
    } else if (dragMode_ == DragMode::EndHandle) {
        playbackEndFrame_ = qMax(frame, playbackStartFrame_);
        playbackEndFrame_ = qBound(playbackStartFrame_, playbackEndFrame_, visibleEndFrame_);
        emitRangeChangedIfNeeded();
    } else if (dragMode_ == DragMode::MovePlaybackRange) {
        const int delta = frame - dragAnchorFrame_;
        const int span = dragStartPlaybackEndFrame_ - dragStartPlaybackStartFrame_;
        int nextStart = dragStartPlaybackStartFrame_ + delta;
        int nextEnd = nextStart + span;
        if (nextStart < visibleStartFrame_) {
            nextEnd += visibleStartFrame_ - nextStart;
            nextStart = visibleStartFrame_;
        }
        if (nextEnd > visibleEndFrame_) {
            nextStart -= nextEnd - visibleEndFrame_;
            nextEnd = visibleEndFrame_;
        }
        playbackStartFrame_ = nextStart;
        playbackEndFrame_ = nextEnd;
        emitRangeChangedIfNeeded();
    } else if (dragMode_ == DragMode::PanVisibleRange) {
        const int delta = frame - dragAnchorFrame_;
        const int span = dragStartVisibleEndFrame_ - dragStartVisibleStartFrame_;
        int nextVisibleStart = dragStartVisibleStartFrame_ - delta;
        int nextVisibleEnd = nextVisibleStart + span;
        if (nextVisibleStart > playbackStartFrame_) {
            nextVisibleStart = playbackStartFrame_;
            nextVisibleEnd = nextVisibleStart + span;
        }
        if (nextVisibleEnd < playbackEndFrame_) {
            nextVisibleEnd = playbackEndFrame_;
            nextVisibleStart = nextVisibleEnd - span;
        }
        visibleStartFrame_ = nextVisibleStart;
        visibleEndFrame_ = nextVisibleEnd;
        emitVisibleRangeChangedIfNeeded();
    }
    event->accept();
}

void RangeSliderTrackWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (dragMode_ == DragMode::None || event->button() != Qt::LeftButton) {
        QWidget::mouseReleaseEvent(event);
        return;
    }

    dragMode_ = DragMode::None;
    event->accept();
}

void RangeSliderTrackWidget::emitRangeChangedIfNeeded()
{
    update();
    if (playbackRangeChangedCallback_) {
        playbackRangeChangedCallback_(playbackStartFrame_, playbackEndFrame_);
    }
}

void RangeSliderTrackWidget::emitVisibleRangeChangedIfNeeded()
{
    update();
    if (visibleRangeChangedCallback_) {
        visibleRangeChangedCallback_(visibleStartFrame_, visibleEndFrame_);
    }
}

qreal RangeSliderTrackWidget::frameToPosition(int frame) const
{
    const QRectF trackRect = rect().adjusted(3, 6, -3, -6);
    const int span = qMax(1, visibleEndFrame_ - visibleStartFrame_);
    const qreal t = qBound(0.0, static_cast<qreal>(frame - visibleStartFrame_) / static_cast<qreal>(span), 1.0);
    return trackRect.left() + t * trackRect.width();
}

int RangeSliderTrackWidget::positionToFrame(int x) const
{
    const QRectF trackRect = rect().adjusted(3, 6, -3, -6);
    const int span = qMax(1, visibleEndFrame_ - visibleStartFrame_);
    const qreal t = qBound(0.0, (static_cast<qreal>(x) - trackRect.left()) / trackRect.width(), 1.0);
    return visibleStartFrame_ + qRound(t * span);
}

RangeSliderPanel::RangeSliderPanel(QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 2, 8, 4);
    layout->setSpacing(8);

    autoKeyButton_ = new QPushButton("Auto", this);
    autoKeyButton_->setCheckable(true);
    autoKeyButton_->setFixedHeight(22);
    layout->addWidget(autoKeyButton_);

    startFrameSpinBox_ = new QSpinBox(this);
    startFrameSpinBox_->setRange(-10000, 100000);
    startFrameSpinBox_->setFixedSize(56, 22);
    startFrameSpinBox_->setButtonSymbols(QAbstractSpinBox::NoButtons);
    startFrameSpinBox_->setAlignment(Qt::AlignCenter);
    startFrameSpinBox_->setProperty("variant", "frameBox");
    layout->addWidget(startFrameSpinBox_);

    rangeSliderTrackWidget_ = new RangeSliderTrackWidget(this);
    layout->addWidget(rangeSliderTrackWidget_, 1);

    endFrameSpinBox_ = new QSpinBox(this);
    endFrameSpinBox_->setRange(-10000, 100000);
    endFrameSpinBox_->setFixedSize(56, 22);
    endFrameSpinBox_->setButtonSymbols(QAbstractSpinBox::NoButtons);
    endFrameSpinBox_->setAlignment(Qt::AlignCenter);
    endFrameSpinBox_->setProperty("variant", "frameBox");
    layout->addWidget(endFrameSpinBox_);

    fpsComboBox_ = new QComboBox(this);
    fpsComboBox_->addItem("12 fps", 12);
    fpsComboBox_->addItem("24 fps", 24);
    fpsComboBox_->addItem("30 fps", 30);
    fpsComboBox_->addItem("60 fps", 60);
    fpsComboBox_->setFixedHeight(22);
    layout->addWidget(fpsComboBox_);

    QObject::connect(autoKeyButton_, &QPushButton::toggled, this, [this](bool enabled) {
        if (!updatingControls_ && autoKeyChangedCallback_) {
            autoKeyChangedCallback_(enabled);
        }
    });

    QObject::connect(startFrameSpinBox_, qOverload<int>(&QSpinBox::valueChanged), this, [this](int startFrame) {
        if (!updatingControls_ && visibleRangeChangedCallback_) {
            visibleRangeChangedCallback_(startFrame, endFrameSpinBox_->value());
        }
    });
    QObject::connect(endFrameSpinBox_, qOverload<int>(&QSpinBox::valueChanged), this, [this](int endFrame) {
        if (!updatingControls_ && visibleRangeChangedCallback_) {
            visibleRangeChangedCallback_(startFrameSpinBox_->value(), endFrame);
        }
    });
    rangeSliderTrackWidget_->setPlaybackRangeChangedCallback([this](int startFrame, int endFrame) {
        updatingControls_ = true;
        {
            QSignalBlocker blockStart(startFrameSpinBox_);
            QSignalBlocker blockEnd(endFrameSpinBox_);
            startFrameSpinBox_->setValue(startFrame);
            endFrameSpinBox_->setValue(endFrame);
        }
        updatingControls_ = false;
        if (playbackRangeChangedCallback_) {
            playbackRangeChangedCallback_(startFrame, endFrame);
        }
    });
    rangeSliderTrackWidget_->setVisibleRangeChangedCallback([this](int startFrame, int endFrame) {
        if (!updatingControls_ && visibleRangeChangedCallback_) {
            visibleRangeChangedCallback_(startFrame, endFrame);
        }
    });
    QObject::connect(fpsComboBox_, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (!updatingControls_ && framesPerSecondChangedCallback_) {
            framesPerSecondChangedCallback_(fpsComboBox_->itemData(index).toInt());
        }
    });
}

void RangeSliderPanel::setViewModel(const EditorAnimationTimelineViewModel& viewModel)
{
    updatingControls_ = true;
    {
        QSignalBlocker blockAuto(autoKeyButton_);
        QSignalBlocker blockStart(startFrameSpinBox_);
        QSignalBlocker blockEnd(endFrameSpinBox_);
        QSignalBlocker blockFps(fpsComboBox_);
        autoKeyButton_->setChecked(viewModel.autoKeyEnabled);
        startFrameSpinBox_->setValue(viewModel.visibleStartFrame);
        endFrameSpinBox_->setValue(viewModel.visibleEndFrame);
        const int fpsIndex = fpsComboBox_->findData(viewModel.framesPerSecond);
        if (fpsIndex >= 0) {
            fpsComboBox_->setCurrentIndex(fpsIndex);
        }
    }
    updatingControls_ = false;

    rangeSliderTrackWidget_->setFrames(
        viewModel.visibleStartFrame,
        viewModel.visibleEndFrame,
        viewModel.playbackStartFrame,
        viewModel.playbackEndFrame,
        viewModel.currentFrame);
}

void RangeSliderPanel::setPlaybackRangeChangedCallback(std::function<void(int, int)> callback)
{
    playbackRangeChangedCallback_ = std::move(callback);
}

void RangeSliderPanel::setVisibleRangeChangedCallback(std::function<void(int, int)> callback)
{
    visibleRangeChangedCallback_ = std::move(callback);
}

void RangeSliderPanel::setAutoKeyChangedCallback(std::function<void(bool)> callback)
{
    autoKeyChangedCallback_ = std::move(callback);
}

void RangeSliderPanel::setFramesPerSecondChangedCallback(std::function<void(int)> callback)
{
    framesPerSecondChangedCallback_ = std::move(callback);
}
