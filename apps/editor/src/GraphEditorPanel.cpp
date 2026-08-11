#include "GraphEditorPanel.h"

#include <QAction>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWheelEvent>

#include <cmath>

namespace
{
constexpr int kCanvasMarginLeft   = 44;
constexpr int kCanvasMarginRight  = 18;
constexpr int kCanvasMarginTop    = 16;
constexpr int kCanvasMarginBottom = 24;
constexpr qreal kKeyHitRadius        = 9.0;
constexpr qreal kTangentHandleLength = 48.0; // pixels

QColor gridColor()
{
    return QColor("#37404a");
}

QColor mutedTextColor()
{
    return QColor("#8f9aa6");
}
} // namespace

// ---------------------------------------------------------------------------
// GraphEditorCanvasWidget
// ---------------------------------------------------------------------------

GraphEditorCanvasWidget::GraphEditorCanvasWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(120);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
    setContextMenuPolicy(Qt::DefaultContextMenu);
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

void GraphEditorCanvasWidget::setKeyEditedCallback(std::function<void(const GraphEditorKeyEdit&)> callback)
{
    keyEditedCallback_ = std::move(callback);
}

void GraphEditorCanvasWidget::setTangentEditedCallback(std::function<void(const TangentEdit&)> callback)
{
    tangentEditedCallback_ = std::move(callback);
}

void GraphEditorCanvasWidget::setTangentModeChangedCallback(std::function<void(const TangentModeChange&)> callback)
{
    tangentModeChangedCallback_ = std::move(callback);
}

void GraphEditorCanvasWidget::setShowTangents(bool show)
{
    showTangents_ = show;
    update();
}

void GraphEditorCanvasWidget::fireKeyEditForTest(const GraphEditorKeyEdit& edit)
{
    if (keyEditedCallback_) {
        keyEditedCallback_(edit);
    }
}

void GraphEditorCanvasWidget::frameAll()
{
    hasCustomFrameView_ = false;
    hasCustomValueView_ = false;
    frameViewStart_ = viewModel_.visibleStartFrame;
    frameViewEnd_ = qMax(viewModel_.visibleStartFrame + 1, viewModel_.visibleEndFrame);
    fitValueRangeToVisibleCurves();
    update();
}

void GraphEditorCanvasWidget::frameSelected()
{
    if (selectedKeys_.isEmpty()) {
        frameAll();
        return;
    }

    int minFrame = INT_MAX;
    int maxFrame = INT_MIN;
    double minValue = 1e18;
    double maxValue = -1e18;
    bool found = false;

    for (const SelectedKey& sel : selectedKeys_) {
        for (const GraphEditorCurve& curve : viewModel_.curves) {
            if (curve.id != sel.curveId) {
                continue;
            }
            for (const GraphEditorCurvePoint& pt : curve.points) {
                if (pt.frame == sel.frame && qAbs(pt.value - sel.value) < 0.0001) {
                    minFrame = qMin(minFrame, pt.frame);
                    maxFrame = qMax(maxFrame, pt.frame);
                    minValue = qMin(minValue, pt.value);
                    maxValue = qMax(maxValue, pt.value);
                    found = true;
                }
            }
        }
    }

    if (!found) {
        frameAll();
        return;
    }

    const int framePad = qMax(2, (maxFrame - minFrame) / 4 + 2);
    frameViewStart_ = minFrame - framePad;
    frameViewEnd_ = maxFrame + framePad;

    const double valuePad = qMax(0.5, (maxValue - minValue) * 0.25 + 0.5);
    valueViewMin_ = minValue - valuePad;
    valueViewMax_ = maxValue + valuePad;
    hasCustomFrameView_ = true;
    hasCustomValueView_ = true;
    ensureViewRanges();
    update();
}

// ---------------------------------------------------------------------------
// paintEvent
// ---------------------------------------------------------------------------

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

    const QVector<const GraphEditorCurve*> visCurves = this->visibleCurves();

    if (visCurves.isEmpty()) {
        painter.setPen(mutedTextColor());
        painter.drawText(plotRect, Qt::AlignCenter, "No visible curves");
    } else {
        painter.setPen(mutedTextColor());
        painter.drawText(QRectF(4.0, plotRect.top() - 4.0, 34.0, 16.0), Qt::AlignRight | Qt::AlignVCenter, QString::number(valueViewMax_, 'f', 2));
        painter.drawText(QRectF(4.0, plotRect.bottom() - 8.0, 34.0, 16.0), Qt::AlignRight | Qt::AlignVCenter, QString::number(valueViewMin_, 'f', 2));

        for (const GraphEditorCurve* curve : visCurves) {
            if (curve->points.isEmpty()) {
                continue;
            }

            // Draw curve path — use cubicTo for non-linear modes.
            QPainterPath path;
            bool started = false;
            for (int i = 0; i < curve->points.size(); ++i) {
                const GraphEditorCurvePoint& pt = curve->points[i];
                const QPointF pos = pointToCanvasPosition(pt);
                if (!started) {
                    path.moveTo(pos);
                    started = true;
                    continue;
                }

                const GraphEditorCurvePoint& prev = curve->points[i - 1];
                const TangentMode outMode = prev.tangentMode;
                const TangentMode inMode  = pt.tangentMode;

                if (outMode == TangentMode::Stepped) {
                    // Horizontal step then vertical jump
                    path.lineTo(QPointF(pos.x(), path.currentPosition().y()));
                    path.lineTo(pos);
                } else if (outMode == TangentMode::Linear || inMode == TangentMode::Linear) {
                    path.lineTo(pos);
                } else {
                    // Bezier: compute control points using tangent angles
                    const float outA = (outMode == TangentMode::Flat) ? 0.f : prev.outAngle;
                    const float inA  = (inMode  == TangentMode::Flat) ? 0.f : pt.inAngle;

                    // Canvas-space scales
                    const qreal fw = plotRect.width()  / qMax(1, frameViewEnd_ - frameViewStart_);
                    const qreal vh = plotRect.height() / qMax(0.0001, valueViewMax_ - valueViewMin_);

                    const qreal prevCanvasX = path.currentPosition().x();
                    const qreal segW = pos.x() - prevCanvasX;

                    // Out tangent direction in canvas space
                    const qreal outSlope  = std::tan(static_cast<qreal>(outA) * M_PI / 180.0);
                    const qreal outDirX   = fw;
                    const qreal outDirY   = -vh * outSlope; // canvas y inverted
                    const qreal outLen    = std::hypot(outDirX, outDirY);
                    const qreal outScale  = (outLen > 1e-8) ? (segW * (1.0 / 3.0)) / outLen : 0.0;
                    const QPointF cp1(prevCanvasX + outDirX * outScale, path.currentPosition().y() + outDirY * outScale);

                    // In tangent direction in canvas space (arriving from right)
                    const qreal inSlope   = std::tan(static_cast<qreal>(inA) * M_PI / 180.0);
                    const qreal inDirX    = -fw;
                    const qreal inDirY    = vh * inSlope; // reversed
                    const qreal inLen     = std::hypot(inDirX, inDirY);
                    const qreal inScale   = (inLen > 1e-8) ? (segW * (1.0 / 3.0)) / inLen : 0.0;
                    const QPointF cp2(pos.x() + inDirX * inScale, pos.y() + inDirY * inScale);

                    path.cubicTo(cp1, cp2, pos);
                }
            }

            painter.setPen(QPen(curve->color, 2.0));
            painter.drawPath(path);

            // Draw key circles and tangent handles
            for (int i = 0; i < curve->points.size(); ++i) {
                const GraphEditorCurvePoint& pt = curve->points[i];
                const QPointF pos = pointToCanvasPosition(pt);
                const bool isSelected = isKeySelected(curve->id, pt.frame, pt.value);

                if (isSelected) {
                    painter.setPen(QPen(QColor("#f5f7fb"), 2.0));
                    painter.setBrush(curve->color.lighter(120));
                    painter.drawEllipse(pos, 6.0, 6.0);

                    // Draw tangent handles for selected keys
                    if (showTangents_ && pt.tangentMode != TangentMode::Stepped && pt.tangentMode != TangentMode::Linear) {
                        const QColor handleColor = curve->color.darker(140);
                        painter.setPen(QPen(handleColor, 1.0));
                        painter.setBrush(handleColor);

                        if (i < curve->points.size() - 1) {
                            const QPointF outPos = tangentHandlePosition(pos, pt.outAngle, true);
                            painter.drawLine(pos, outPos);
                            painter.drawRect(QRectF(outPos.x() - 3.5, outPos.y() - 3.5, 7.0, 7.0));
                        }
                        if (i > 0) {
                            const QPointF inPos = tangentHandlePosition(pos, pt.inAngle, false);
                            painter.drawLine(pos, inPos);
                            painter.drawRect(QRectF(inPos.x() - 3.5, inPos.y() - 3.5, 7.0, 7.0));
                        }
                    }
                } else {
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(curve->color);
                    painter.drawEllipse(pos, 3.5, 3.5);
                }
            }
        }
    }

    // Key drag preview
    if (draggingKey_) {
        painter.setPen(QPen(QColor("#f5f7fb"), 1.0, Qt::DashLine));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(dragKeyCurrent_, 6.0, 6.0);
    }

    // Marquee selection rect
    if (leftDraggingSelection_ && !marqueeRect_.isNull()) {
        painter.setPen(QPen(QColor("#83a9cf"), 1.0, Qt::DashLine));
        painter.setBrush(QColor(93, 130, 171, 38));
        painter.drawRect(marqueeRect_);
    }

    // Current frame indicator
    const qreal currentRatio = static_cast<qreal>(viewModel_.currentFrame - frameViewStart_) / frameSpan;
    const qreal currentX = plotRect.left() + (currentRatio * plotRect.width());
    painter.setPen(QPen(QColor("#f0d26c"), 1.5));
    painter.drawLine(QPointF(currentX, plotRect.top()), QPointF(currentX, plotRect.bottom()));
    painter.fillRect(QRectF(currentX - 16.0, 2.0, 32.0, 14.0), QColor("#f0d26c"));
    painter.setPen(QColor("#1f2328"));
    painter.drawText(QRectF(currentX - 16.0, 2.0, 32.0, 14.0), Qt::AlignCenter, QString::number(viewModel_.currentFrame));
}

// ---------------------------------------------------------------------------
// Mouse events
// ---------------------------------------------------------------------------

void GraphEditorCanvasWidget::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);

    if (event->button() == Qt::MiddleButton) {
        middlePanning_ = true;
        lastPanPosition_ = event->pos();
        setCursor(Qt::ClosedHandCursor);
        return;
    }

    if (event->button() != Qt::LeftButton) {
        return;
    }

    leftPressInPlot_ = plotRect().contains(event->position());
    leftDraggingSelection_ = false;
    draggingCurrentFrame_ = false;
    draggingKey_ = false;
    draggingTangent_ = false;
    selectionDragStart_ = event->pos();
    marqueeRect_ = QRectF();

    if (currentFrameHandleRect().contains(event->position())) {
        draggingCurrentFrame_ = true;
        if (currentFrameChangedCallback_) {
            currentFrameChangedCallback_(frameFromPosition(event->position().x()));
        }
        selectedKeys_.clear();
        update();
        return;
    }

    // Check tangent handle drag first (only when a key is selected)
    if (showTangents_ && !selectedKeys_.isEmpty() && tryStartTangentDrag(event->position())) {
        draggingTangent_ = true;
        leftPressInPlot_ = false;
        update();
        return;
    }

    // Check key drag
    if (trySelectKeyAt(event->position())) {
        if (!selectedKeys_.isEmpty()) {
            if (currentFrameChangedCallback_) {
                currentFrameChangedCallback_(selectedKeys_.first().frame);
            }
            // Prepare for potential drag
            dragKey_ = selectedKeys_.first();
            dragKeyOrigFrame_ = dragKey_.frame;
            dragKeyOrigValue_ = dragKey_.value;
            dragKeyCurrent_ = pointToCanvasPosition({ dragKey_.frame, dragKey_.value });
            // We enter drag mode on the first mouseMoveEvent that moves enough
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
        if (currentFrameChangedCallback_) {
            currentFrameChangedCallback_(frameFromPosition(event->position().x()));
        }
        update();
        return;
    }

    if (draggingTangent_) {
        // Update angle from current mouse pos
        const QPointF delta = event->position() - tangentDragKeyCanvasPos_;
        const float angle = angleFromCanvasDelta(delta * (tangentDrag_.isIn ? -1.0 : 1.0));
        const GraphEditorCurvePoint* pt = findCurvePoint(tangentDrag_.curveId, tangentDrag_.frame);
        if (pt != nullptr && tangentEditedCallback_) {
            // Preview: just trigger update; full emit on release
            Q_UNUSED(pt);
        }
        update();
        return;
    }

    if (!(event->buttons() & Qt::LeftButton)) {
        return;
    }

    // Check if we should enter key drag mode
    if (!draggingKey_ && !dragKey_.curveId.isEmpty()) {
        const QPoint delta = event->pos() - selectionDragStart_;
        if (delta.manhattanLength() >= 4) {
            draggingKey_ = true;
        }
    }

    if (draggingKey_) {
        const Qt::KeyboardModifiers mods = event->modifiers();
        int newFrame  = frameFromPosition(event->position().x());
        double newVal = (mods & Qt::ControlModifier) ? dragKeyOrigValue_ : valueFromPosition(event->position().y());
        if (mods & Qt::ShiftModifier) {
            newFrame = dragKeyOrigFrame_;
        }
        dragKeyCurrent_ = QPointF(
            pointToCanvasPosition({ newFrame, newVal }).x(),
            pointToCanvasPosition({ newFrame, newVal }).y());
        update();
        return;
    }

    if (!leftPressInPlot_) {
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

    if (draggingTangent_) {
        draggingTangent_ = false;
        const QPointF delta = event->position() - tangentDragKeyCanvasPos_;
        const float angle = angleFromCanvasDelta(delta * (tangentDrag_.isIn ? -1.0 : 1.0));
        if (tangentEditedCallback_) {
            tangentEditedCallback_({ tangentDrag_.curveId, tangentDrag_.frame, tangentDrag_.isIn, angle });
        }
        leftPressInPlot_ = false;
        update();
        return;
    }

    if (draggingKey_) {
        draggingKey_ = false;
        const Qt::KeyboardModifiers mods = event->modifiers();
        int newFrame = (mods & Qt::ShiftModifier) ? dragKeyOrigFrame_ : frameFromPosition(event->position().x());
        const double newVal = (mods & Qt::ControlModifier) ? dragKeyOrigValue_ : valueFromPosition(event->position().y());
        if (keyEditedCallback_ && !dragKey_.curveId.isEmpty()) {
            keyEditedCallback_({ dragKey_.curveId, dragKeyOrigFrame_, newFrame, newVal });
        }
        dragKey_ = {};
        leftPressInPlot_ = false;
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
        if (currentFrameChangedCallback_) {
            currentFrameChangedCallback_(frameFromPosition(event->position().x()));
        }
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

void GraphEditorCanvasWidget::contextMenuEvent(QContextMenuEvent* event)
{
    if (selectedKeys_.isEmpty() || !tangentModeChangedCallback_) {
        return;
    }

    QMenu menu(this);
    QAction* autoAct    = menu.addAction("Auto Tangent");
    QAction* linearAct  = menu.addAction("Linear");
    QAction* flatAct    = menu.addAction("Flat");
    QAction* steppedAct = menu.addAction("Stepped");
    QAction* brokenAct  = menu.addAction("Broken");

    QAction* chosen = menu.exec(event->globalPos());
    if (chosen == nullptr) {
        return;
    }

    TangentMode mode = TangentMode::Auto;
    if (chosen == linearAct)  mode = TangentMode::Linear;
    else if (chosen == flatAct)    mode = TangentMode::Flat;
    else if (chosen == steppedAct) mode = TangentMode::Stepped;
    else if (chosen == brokenAct)  mode = TangentMode::Broken;

    for (const SelectedKey& key : selectedKeys_) {
        tangentModeChangedCallback_({ key.curveId, key.frame, mode });
    }
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

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
    const QRectF rect = plotRect();
    if (!rect.contains(position)) {
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

QPointF GraphEditorCanvasWidget::tangentHandlePosition(const QPointF& keyPos, float angleDeg, bool isOut) const
{
    const QRectF rect = plotRect();
    const qreal frameSpan = qMax(1, frameViewEnd_ - frameViewStart_);
    const qreal valueSpan = qMax(0.0001, valueViewMax_ - valueViewMin_);
    const qreal fw = rect.width() / frameSpan;   // pixels per frame
    const qreal vh = rect.height() / valueSpan;  // pixels per value

    const qreal angleRad = static_cast<qreal>(angleDeg) * M_PI / 180.0;
    const qreal dirX =  fw * std::cos(angleRad);
    const qreal dirY = -vh * std::sin(angleRad); // canvas y is inverted
    const qreal len = std::hypot(dirX, dirY);
    if (len < 1e-8) {
        return keyPos;
    }
    const qreal scale = kTangentHandleLength / len;
    const qreal sign = isOut ? 1.0 : -1.0;
    return QPointF(keyPos.x() + sign * dirX * scale, keyPos.y() + sign * dirY * scale);
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

double GraphEditorCanvasWidget::valueFromPosition(int y) const
{
    const int plotHeight = qMax(1, height() - (kCanvasMarginTop + kCanvasMarginBottom));
    const qreal normalized = qBound(
        0.0,
        static_cast<qreal>(y - kCanvasMarginTop) / plotHeight,
        1.0);
    return valueViewMax_ - normalized * (valueViewMax_ - valueViewMin_);
}

const GraphEditorCurvePoint* GraphEditorCanvasWidget::findCurvePoint(const QString& curveId, int frame) const
{
    for (const GraphEditorCurve& curve : viewModel_.curves) {
        if (curve.id != curveId) {
            continue;
        }
        for (const GraphEditorCurvePoint& pt : curve.points) {
            if (pt.frame == frame) {
                return &pt;
            }
        }
    }
    return nullptr;
}

bool GraphEditorCanvasWidget::tryStartTangentDrag(const QPointF& position)
{
    constexpr qreal kHandleHitRadius = 7.0;

    for (const SelectedKey& sel : selectedKeys_) {
        const GraphEditorCurvePoint* pt = findCurvePoint(sel.curveId, sel.frame);
        if (pt == nullptr) {
            continue;
        }
        if (pt->tangentMode == TangentMode::Stepped || pt->tangentMode == TangentMode::Linear) {
            continue;
        }

        const QPointF keyPos = pointToCanvasPosition(*pt);

        // Check out handle (not on last key)
        for (const GraphEditorCurve& curve : viewModel_.curves) {
            if (curve.id != sel.curveId) {
                continue;
            }
            for (int i = 0; i < curve.points.size(); ++i) {
                if (curve.points[i].frame != pt->frame) {
                    continue;
                }
                if (i < curve.points.size() - 1) {
                    const QPointF outPos = tangentHandlePosition(keyPos, pt->outAngle, true);
                    if (QLineF(position, outPos).length() <= kHandleHitRadius) {
                        tangentDrag_ = { sel.curveId, sel.frame, false };
                        tangentDragKeyCanvasPos_ = keyPos;
                        return true;
                    }
                }
                if (i > 0) {
                    const QPointF inPos = tangentHandlePosition(keyPos, pt->inAngle, false);
                    if (QLineF(position, inPos).length() <= kHandleHitRadius) {
                        tangentDrag_ = { sel.curveId, sel.frame, true };
                        tangentDragKeyCanvasPos_ = keyPos;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

float GraphEditorCanvasWidget::angleFromCanvasDelta(const QPointF& delta) const
{
    const QRectF rect = plotRect();
    const qreal frameSpan = qMax(1, frameViewEnd_ - frameViewStart_);
    const qreal valueSpan = qMax(0.0001, valueViewMax_ - valueViewMin_);
    const qreal fw = rect.width() / frameSpan;
    const qreal vh = rect.height() / valueSpan;

    // Convert canvas delta to frame/value delta
    const qreal dfFrames = delta.x() / fw;
    const qreal dvValues = -delta.y() / vh; // canvas y inverted

    return static_cast<float>(std::atan2(dvValues, qMax(0.001, dfFrames)) * 180.0 / M_PI);
}

// ---------------------------------------------------------------------------
// GraphEditorPanel
// ---------------------------------------------------------------------------

GraphEditorPanel::GraphEditorPanel(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(160);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(10, 10, 10, 10);
    rootLayout->setSpacing(6);

    // Header
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

    // Toolbar (Phase 4)
    QToolBar* toolbar = new QToolBar(this);
    toolbar->setIconSize(QSize(16, 16));
    toolbar->setStyleSheet("QToolBar { border: none; background: #262d35; spacing: 2px; }"
                           "QToolButton { color: #c8d0da; padding: 2px 6px; border-radius: 3px; }"
                           "QToolButton:hover { background: #343c45; }");

    canvasWidget_ = new GraphEditorCanvasWidget(this);

    QAction* frameAllAction = toolbar->addAction("Frame All");
    QAction* frameSelAction = toolbar->addAction("Frame Selected");
    toolbar->addSeparator();
    QAction* showTangentsAction = toolbar->addAction("Tangents");
    showTangentsAction->setCheckable(true);
    showTangentsAction->setChecked(true);

    connect(frameAllAction,     &QAction::triggered, canvasWidget_, &GraphEditorCanvasWidget::frameAll);
    connect(frameSelAction,     &QAction::triggered, canvasWidget_, &GraphEditorCanvasWidget::frameSelected);
    connect(showTangentsAction, &QAction::toggled,   canvasWidget_, &GraphEditorCanvasWidget::setShowTangents);

    rootLayout->addWidget(toolbar);

    // Content area
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

void GraphEditorPanel::setKeyEditedCallback(std::function<void(const GraphEditorKeyEdit&)> callback)
{
    canvasWidget_->setKeyEditedCallback(std::move(callback));
}

void GraphEditorPanel::setTangentEditedCallback(std::function<void(const TangentEdit&)> callback)
{
    canvasWidget_->setTangentEditedCallback(std::move(callback));
}

void GraphEditorPanel::setTangentModeChangedCallback(std::function<void(const TangentModeChange&)> callback)
{
    canvasWidget_->setTangentModeChangedCallback(std::move(callback));
}

void GraphEditorPanel::fireKeyEditForTest(const GraphEditorKeyEdit& edit)
{
    canvasWidget_->fireKeyEditForTest(edit);
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

