#include "GraphEditorPanel.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QWheelEvent>

#include <cmath>

namespace
{
constexpr int kCanvasMarginLeft = 44;
constexpr int kCanvasMarginRight = 18;
constexpr int kCanvasMarginTop = 16;
constexpr int kCanvasMarginBottom = 24;
constexpr qreal kKeyHitRadius = 9.0;

QColor gridColor()
{
    return QColor("#37404a");
}

QColor mutedTextColor()
{
    return QColor("#8f9aa6");
}
}

GraphEditorCanvasWidget::GraphEditorCanvasWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(120);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
}

void GraphEditorCanvasWidget::setViewModel(const GraphEditorViewModel& viewModel, const QVector<QString>& visibleCurveIds)
{
    const bool objectChanged = viewModel_.objectName != viewModel.objectName;
    viewModel_ = viewModel;
    visibleCurveIds_ = visibleCurveIds;
    if (!hasCustomFrameView_ || objectChanged) {
        frameViewStart_ = viewModel_.visibleStartFrame;
        frameViewEnd_ = qMax(viewModel_.visibleStartFrame + 1, viewModel_.visibleEndFrame);
        hasCustomFrameView_ = false;
    }
    if (!hasCustomValueView_ || objectChanged) {
        fitValueRangeToVisibleCurves();
        hasCustomValueView_ = false;
    } else {
        ensureViewRanges();
    }
    if (objectChanged) {
        selectedKeys_.clear();
    } else {
        clearInvalidSelection();
    }
    update();
}

void GraphEditorCanvasWidget::setCurrentFrameChangedCallback(std::function<void(int)> callback)
{
    currentFrameChangedCallback_ = std::move(callback);
}

void GraphEditorCanvasWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    ensureViewRanges();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), QColor("#20252b"));

    const QRectF plotRect = this->plotRect();

    painter.setPen(QPen(QColor("#43505d"), 1.0));
    painter.drawRect(plotRect);

    const int frameSpan = qMax(1, frameViewEnd_ - frameViewStart_);
    for (int i = 0; i <= 6; ++i) {
        const qreal y = plotRect.top() + (plotRect.height() * static_cast<qreal>(i) / 6.0);
        painter.setPen(QPen(gridColor(), 1.0));
        painter.drawLine(QPointF(plotRect.left(), y), QPointF(plotRect.right(), y));
    }

    const int horizontalTicks = qMin(frameSpan, 12);
    for (int i = 0; i <= horizontalTicks; ++i) {
        const qreal x = plotRect.left() + (plotRect.width() * static_cast<qreal>(i) / qMax(1, horizontalTicks));
        painter.setPen(QPen(gridColor(), 1.0));
        painter.drawLine(QPointF(x, plotRect.top()), QPointF(x, plotRect.bottom()));
        const int frame = frameViewStart_ + qRound(frameSpan * static_cast<qreal>(i) / qMax(1, horizontalTicks));
        painter.setPen(mutedTextColor());
        painter.drawText(QRectF(x - 18.0, plotRect.bottom() + 4.0, 36.0, 16.0), Qt::AlignCenter, QString::number(frame));
    }

    const QVector<const GraphEditorCurve*> visibleCurves = this->visibleCurves();

    if (visibleCurves.isEmpty()) {
        painter.setPen(mutedTextColor());
        painter.drawText(plotRect, Qt::AlignCenter, "No visible curves");
    } else {
        painter.setPen(mutedTextColor());
        painter.drawText(QRectF(4.0, plotRect.top() - 4.0, 34.0, 16.0), Qt::AlignRight | Qt::AlignVCenter, QString::number(valueViewMax_, 'f', 2));
        painter.drawText(QRectF(4.0, plotRect.bottom() - 8.0, 34.0, 16.0), Qt::AlignRight | Qt::AlignVCenter, QString::number(valueViewMin_, 'f', 2));

        for (const GraphEditorCurve* curve : visibleCurves) {
            if (curve->points.isEmpty()) {
                continue;
            }

            QPainterPath path;
            bool started = false;
            for (const GraphEditorCurvePoint& point : curve->points) {
                const QPointF pos = pointToCanvasPosition(point);
                if (!started) {
                    path.moveTo(pos);
                    started = true;
                } else {
                    path.lineTo(pos);
                }
            }

            painter.setPen(QPen(curve->color, 2.0));
            painter.drawPath(path);

            painter.setBrush(curve->color);
            for (const GraphEditorCurvePoint& point : curve->points) {
                const QPointF pos = pointToCanvasPosition(point);
                const bool isSelected = isKeySelected(curve->id, point.frame, point.value);
                if (isSelected) {
                    painter.setPen(QPen(QColor("#f5f7fb"), 2.0));
                    painter.setBrush(curve->color.lighter(120));
                    painter.drawEllipse(pos, 6.0, 6.0);
                } else {
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(curve->color);
                    painter.drawEllipse(pos, 3.5, 3.5);
                }
            }
        }
    }

    if (leftDraggingSelection_ && !marqueeRect_.isNull()) {
        painter.setPen(QPen(QColor("#83a9cf"), 1.0, Qt::DashLine));
        painter.setBrush(QColor(93, 130, 171, 38));
        painter.drawRect(marqueeRect_);
    }

    const qreal currentRatio = static_cast<qreal>(viewModel_.currentFrame - frameViewStart_) / frameSpan;
    const qreal currentX = plotRect.left() + (currentRatio * plotRect.width());
    painter.setPen(QPen(QColor("#f0d26c"), 1.5));
    painter.drawLine(QPointF(currentX, plotRect.top()), QPointF(currentX, plotRect.bottom()));
    painter.fillRect(QRectF(currentX - 16.0, 2.0, 32.0, 14.0), QColor("#f0d26c"));
    painter.setPen(QColor("#1f2328"));
    painter.drawText(QRectF(currentX - 16.0, 2.0, 32.0, 14.0), Qt::AlignCenter, QString::number(viewModel_.currentFrame));
}

void GraphEditorCanvasWidget::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);

    if (event->button() == Qt::MiddleButton) {
        middlePanning_ = true;
        lastPanPosition_ = event->pos();
        setCursor(Qt::ClosedHandCursor);
        return;
    }

    if (event->button() != Qt::LeftButton || !currentFrameChangedCallback_) {
        return;
    }

    leftPressInPlot_ = plotRect().contains(event->position());
    leftDraggingSelection_ = false;
    draggingCurrentFrame_ = false;
    selectionDragStart_ = event->pos();
    marqueeRect_ = QRectF();

    if (currentFrameHandleRect().contains(event->position())) {
        draggingCurrentFrame_ = true;
        currentFrameChangedCallback_(frameFromPosition(event->position().x()));
        selectedKeys_.clear();
        update();
        return;
    }

    if (trySelectKeyAt(event->position())) {
        if (!selectedKeys_.isEmpty()) {
            currentFrameChangedCallback_(selectedKeys_.first().frame);
        }
        leftPressInPlot_ = false;
        update();
        return;
    }
}

void GraphEditorCanvasWidget::mouseMoveEvent(QMouseEvent* event)
{
    QWidget::mouseMoveEvent(event);

    if (middlePanning_) {
        ensureViewRanges();
        const QPoint delta = event->pos() - lastPanPosition_;
        lastPanPosition_ = event->pos();

        const int plotWidth = qMax(1, width() - (kCanvasMarginLeft + kCanvasMarginRight));
        const int frameSpan = qMax(1, frameViewEnd_ - frameViewStart_);
        const qreal framePerPixel = static_cast<qreal>(frameSpan) / plotWidth;
        const int frameDelta = qRound(delta.x() * framePerPixel);
        frameViewStart_ -= frameDelta;
        frameViewEnd_ -= frameDelta;
        hasCustomFrameView_ = true;

        const int plotHeight = qMax(1, height() - (kCanvasMarginTop + kCanvasMarginBottom));
        const double valueSpan = qMax(0.0001, valueViewMax_ - valueViewMin_);
        const double valuePerPixel = valueSpan / plotHeight;
        const double valueDelta = delta.y() * valuePerPixel;
        valueViewMin_ += valueDelta;
        valueViewMax_ += valueDelta;
        hasCustomValueView_ = true;

        ensureViewRanges();
        update();
        return;
    }

    if (draggingCurrentFrame_) {
        currentFrameChangedCallback_(frameFromPosition(event->position().x()));
        update();
        return;
    }

    if (!(event->buttons() & Qt::LeftButton) || !leftPressInPlot_) {
        return;
    }

    if (!leftDraggingSelection_) {
        const QPoint delta = event->pos() - selectionDragStart_;
        if (delta.manhattanLength() < 4) {
            return;
        }
        leftDraggingSelection_ = true;
    }

    marqueeRect_ = QRectF(selectionDragStart_, event->pos()).normalized();
    update();
}

void GraphEditorCanvasWidget::mouseReleaseEvent(QMouseEvent* event)
{
    QWidget::mouseReleaseEvent(event);

    if (event->button() == Qt::MiddleButton) {
        middlePanning_ = false;
        unsetCursor();
        return;
    }

    if (event->button() != Qt::LeftButton) {
        return;
    }

    if (draggingCurrentFrame_) {
        draggingCurrentFrame_ = false;
        leftPressInPlot_ = false;
        marqueeRect_ = QRectF();
        update();
        return;
    }

    if (leftDraggingSelection_) {
        selectKeysInRect(marqueeRect_);
        leftDraggingSelection_ = false;
        leftPressInPlot_ = false;
        marqueeRect_ = QRectF();
        update();
        return;
    }

    if (leftPressInPlot_) {
        currentFrameChangedCallback_(frameFromPosition(event->position().x()));
        selectedKeys_.clear();
        leftPressInPlot_ = false;
        marqueeRect_ = QRectF();
        update();
    }
}

void GraphEditorCanvasWidget::wheelEvent(QWheelEvent* event)
{
    ensureViewRanges();

    const QPoint angleDelta = event->angleDelta();
    if (angleDelta.y() == 0) {
        event->ignore();
        return;
    }

    const bool zoomValueAxis = event->modifiers().testFlag(Qt::ShiftModifier);
    const qreal strength = event->modifiers().testFlag(Qt::AltModifier) ? 1.45 : 1.20;
    const qreal zoomFactor = angleDelta.y() > 0 ? (1.0 / strength) : strength;

    if (zoomValueAxis) {
        const int plotHeight = qMax(1, height() - (kCanvasMarginTop + kCanvasMarginBottom));
        const qreal yRatio = qBound(
            0.0,
            static_cast<qreal>(event->position().y() - kCanvasMarginTop) / plotHeight,
            1.0);
        const double anchorValue = valueViewMax_ - (valueViewMax_ - valueViewMin_) * yRatio;
        const double newMin = anchorValue - ((anchorValue - valueViewMin_) * zoomFactor);
        const double newMax = anchorValue + ((valueViewMax_ - anchorValue) * zoomFactor);
        valueViewMin_ = newMin;
        valueViewMax_ = newMax;
        hasCustomValueView_ = true;
    } else {
        const int plotWidth = qMax(1, width() - (kCanvasMarginLeft + kCanvasMarginRight));
        const qreal xRatio = qBound(
            0.0,
            static_cast<qreal>(event->position().x() - kCanvasMarginLeft) / plotWidth,
            1.0);
        const double anchorFrame = frameViewStart_ + ((frameViewEnd_ - frameViewStart_) * xRatio);
        const double newStart = anchorFrame - ((anchorFrame - frameViewStart_) * zoomFactor);
        const double newEnd = anchorFrame + ((frameViewEnd_ - anchorFrame) * zoomFactor);
        frameViewStart_ = static_cast<int>(std::floor(newStart));
        frameViewEnd_ = static_cast<int>(std::ceil(newEnd));
        hasCustomFrameView_ = true;
    }

    ensureViewRanges();
    update();
    event->accept();
}

QVector<const GraphEditorCurve*> GraphEditorCanvasWidget::visibleCurves() const
{
    QVector<const GraphEditorCurve*> curves;
    for (const GraphEditorCurve& curve : viewModel_.curves) {
        if (visibleCurveIds_.contains(curve.id)) {
            curves.append(&curve);
        }
    }
    return curves;
}

void GraphEditorCanvasWidget::clearInvalidSelection()
{
    if (selectedKeys_.isEmpty()) {
        return;
    }

    QVector<SelectedKey> validKeys;
    for (const SelectedKey& key : selectedKeys_) {
        for (const GraphEditorCurve& curve : viewModel_.curves) {
            if (curve.id != key.curveId || !visibleCurveIds_.contains(curve.id)) {
                continue;
            }

            for (const GraphEditorCurvePoint& point : curve.points) {
                if (point.frame == key.frame && qAbs(point.value - key.value) < 0.0001) {
                    validKeys.append(key);
                    goto nextKey;
                }
            }
        }
nextKey:
        continue;
    }

    selectedKeys_ = validKeys;
}

bool GraphEditorCanvasWidget::trySelectKeyAt(const QPointF& position)
{
    const QRectF plotRect = this->plotRect();
    if (!plotRect.contains(position)) {
        return false;
    }

    qreal bestDistance = kKeyHitRadius;
    SelectedKey bestKey;
    for (const GraphEditorCurve* curve : visibleCurves()) {
        for (const GraphEditorCurvePoint& point : curve->points) {
            const qreal distance = QLineF(position, pointToCanvasPosition(point)).length();
            if (distance <= bestDistance) {
                bestDistance = distance;
                bestKey.curveId = curve->id;
                bestKey.frame = point.frame;
                bestKey.value = point.value;
            }
        }
    }

    if (bestKey.curveId.isEmpty()) {
        return false;
    }

    selectedKeys_ = { bestKey };
    return true;
}

void GraphEditorCanvasWidget::selectKeysInRect(const QRectF& selectionRect)
{
    selectedKeys_.clear();
    if (selectionRect.isNull() || selectionRect.width() < 2.0 || selectionRect.height() < 2.0) {
        return;
    }

    for (const GraphEditorCurve* curve : visibleCurves()) {
        for (const GraphEditorCurvePoint& point : curve->points) {
            if (selectionRect.contains(pointToCanvasPosition(point))) {
                selectedKeys_.append({ curve->id, point.frame, point.value });
            }
        }
    }
}

bool GraphEditorCanvasWidget::isKeySelected(const QString& curveId, int frame, double value) const
{
    for (const SelectedKey& key : selectedKeys_) {
        if (key.curveId == curveId && key.frame == frame && qAbs(key.value - value) < 0.0001) {
            return true;
        }
    }
    return false;
}

QRectF GraphEditorCanvasWidget::plotRect() const
{
    return QRectF(
        kCanvasMarginLeft,
        kCanvasMarginTop,
        qMax(40, width() - (kCanvasMarginLeft + kCanvasMarginRight)),
        qMax(40, height() - (kCanvasMarginTop + kCanvasMarginBottom)));
}

QRectF GraphEditorCanvasWidget::currentFrameHandleRect() const
{
    const QRectF rect = plotRect();
    const int frameSpan = qMax(1, frameViewEnd_ - frameViewStart_);
    const qreal currentRatio = static_cast<qreal>(viewModel_.currentFrame - frameViewStart_) / frameSpan;
    const qreal currentX = rect.left() + (currentRatio * rect.width());
    return QRectF(currentX - 10.0, 0.0, 20.0, rect.bottom());
}

QPointF GraphEditorCanvasWidget::pointToCanvasPosition(const GraphEditorCurvePoint& point) const
{
    const QRectF rect = plotRect();
    const int frameSpan = qMax(1, frameViewEnd_ - frameViewStart_);
    const qreal xRatio = static_cast<qreal>(point.frame - frameViewStart_) / frameSpan;
    const qreal yRatio = (point.value - valueViewMin_) / qMax(0.0001, valueViewMax_ - valueViewMin_);
    return QPointF(
        rect.left() + (xRatio * rect.width()),
        rect.bottom() - (yRatio * rect.height()));
}

void GraphEditorCanvasWidget::ensureViewRanges()
{
    if (frameViewEnd_ <= frameViewStart_) {
        frameViewEnd_ = frameViewStart_ + 1;
    }

    if (qFuzzyCompare(valueViewMin_ + 1.0, valueViewMax_ + 1.0) || valueViewMax_ <= valueViewMin_) {
        valueViewMin_ -= 1.0;
        valueViewMax_ += 1.0;
    }
}

void GraphEditorCanvasWidget::fitValueRangeToVisibleCurves()
{
    const QVector<const GraphEditorCurve*> curves = visibleCurves();
    double minValue = 0.0;
    double maxValue = 0.0;
    bool hasValue = false;

    for (const GraphEditorCurve* curve : curves) {
        for (const GraphEditorCurvePoint& point : curve->points) {
            minValue = hasValue ? qMin(minValue, point.value) : point.value;
            maxValue = hasValue ? qMax(maxValue, point.value) : point.value;
            hasValue = true;
        }
    }

    if (!hasValue) {
        valueViewMin_ = -1.0;
        valueViewMax_ = 1.0;
    } else if (qFuzzyCompare(minValue + 1.0, maxValue + 1.0)) {
        valueViewMin_ = minValue - 1.0;
        valueViewMax_ = maxValue + 1.0;
    } else {
        const double padding = qMax(0.1, (maxValue - minValue) * 0.12);
        valueViewMin_ = minValue - padding;
        valueViewMax_ = maxValue + padding;
    }

    ensureViewRanges();
}

int GraphEditorCanvasWidget::frameFromPosition(int x) const
{
    const int plotWidth = qMax(1, width() - (kCanvasMarginLeft + kCanvasMarginRight));
    const qreal normalized = qBound(
        0.0,
        static_cast<qreal>(x - kCanvasMarginLeft) / plotWidth,
        1.0);
    const int frameSpan = qMax(1, frameViewEnd_ - frameViewStart_);
    return frameViewStart_ + qRound(frameSpan * normalized);
}

GraphEditorPanel::GraphEditorPanel(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(160);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(10, 10, 10, 10);
    rootLayout->setSpacing(8);

    objectNameLabel_ = new QLabel(this);
    objectNameLabel_->setStyleSheet("color: #eef2f7; font-weight: 600;");
    rootLayout->addWidget(objectNameLabel_);

    summaryLabel_ = new QLabel(this);
    summaryLabel_->setStyleSheet("color: #8f9aa6;");
    rootLayout->addWidget(summaryLabel_);

    QFrame* headerDivider = new QFrame(this);
    headerDivider->setFrameShape(QFrame::HLine);
    headerDivider->setStyleSheet("color: #343c45; background: #343c45; min-height: 1px; max-height: 1px;");
    rootLayout->addWidget(headerDivider);

    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(10);
    rootLayout->addLayout(contentLayout, 1);

    curveListWidget_ = new QListWidget(this);
    curveListWidget_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    curveListWidget_->setMinimumWidth(140);
    curveListWidget_->setMaximumWidth(180);
    curveListWidget_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    contentLayout->addWidget(curveListWidget_);

    canvasWidget_ = new GraphEditorCanvasWidget(this);
    canvasWidget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    contentLayout->addWidget(canvasWidget_, 1);

    connect(curveListWidget_, &QListWidget::itemSelectionChanged, this, [this]() {
        syncVisibleCurves();
    });
}

void GraphEditorPanel::setViewModel(const GraphEditorViewModel& viewModel)
{
    viewModel_ = viewModel;
    objectNameLabel_->setText(viewModel_.objectName);
    summaryLabel_->setText(viewModel_.summaryText);

    {
        const QSignalBlocker blocker(curveListWidget_);
        curveListWidget_->clear();
        for (const GraphEditorCurve& curve : viewModel_.curves) {
            QListWidgetItem* item = new QListWidgetItem(curve.label, curveListWidget_);
            item->setData(Qt::UserRole, curve.id);
            item->setForeground(curve.color);
            item->setSelected(true);
        }
    }

    syncVisibleCurves();
}

void GraphEditorPanel::setCurrentFrameChangedCallback(std::function<void(int)> callback)
{
    canvasWidget_->setCurrentFrameChangedCallback(std::move(callback));
}

void GraphEditorPanel::syncVisibleCurves()
{
    QVector<QString> visibleCurveIds;
    const QList<QListWidgetItem*> selectedItems = curveListWidget_->selectedItems();
    for (QListWidgetItem* item : selectedItems) {
        visibleCurveIds.append(item->data(Qt::UserRole).toString());
    }

    if (visibleCurveIds.isEmpty()) {
        for (int i = 0; i < curveListWidget_->count(); ++i) {
            visibleCurveIds.append(curveListWidget_->item(i)->data(Qt::UserRole).toString());
        }
    }

    canvasWidget_->setViewModel(viewModel_, visibleCurveIds);
}
