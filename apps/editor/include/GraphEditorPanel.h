#pragma once

#include <QColor>
#include <QPoint>
#include <QRectF>
#include <QString>
#include <QVector>
#include <QWidget>

#include <functional>

class QLabel;
class QListWidget;
class QMouseEvent;
class QPaintEvent;
class QWheelEvent;

struct GraphEditorCurvePoint
{
    int frame = 0;
    double value = 0.0;
};

struct GraphEditorCurve
{
    QString id;
    QString label;
    QColor color;
    QVector<GraphEditorCurvePoint> points;
};

struct GraphEditorViewModel
{
    QString objectName = "No selection";
    QString summaryText = "Select object with animation keys to inspect curves.";
    int visibleStartFrame = 0;
    int visibleEndFrame = 24;
    int currentFrame = 0;
    QVector<GraphEditorCurve> curves;
};

class GraphEditorCanvasWidget : public QWidget
{
public:
    explicit GraphEditorCanvasWidget(QWidget* parent = nullptr);

    void setViewModel(const GraphEditorViewModel& viewModel, const QVector<QString>& visibleCurveIds);
    void setCurrentFrameChangedCallback(std::function<void(int)> callback);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    struct SelectedKey
    {
        QString curveId;
        int frame = 0;
        double value = 0.0;
    };

    void ensureViewRanges();
    void fitValueRangeToVisibleCurves();
    QVector<const GraphEditorCurve*> visibleCurves() const;
    QRectF plotRect() const;
    QRectF currentFrameHandleRect() const;
    QPointF pointToCanvasPosition(const GraphEditorCurvePoint& point) const;
    void clearInvalidSelection();
    bool trySelectKeyAt(const QPointF& position);
    void selectKeysInRect(const QRectF& selectionRect);
    bool isKeySelected(const QString& curveId, int frame, double value) const;
    int frameFromPosition(int x) const;

    GraphEditorViewModel viewModel_;
    QVector<QString> visibleCurveIds_;
    std::function<void(int)> currentFrameChangedCallback_;
    int frameViewStart_ = 0;
    int frameViewEnd_ = 24;
    double valueViewMin_ = -1.0;
    double valueViewMax_ = 1.0;
    bool hasCustomFrameView_ = false;
    bool hasCustomValueView_ = false;
    bool middlePanning_ = false;
    bool draggingCurrentFrame_ = false;
    bool leftDraggingSelection_ = false;
    bool leftPressInPlot_ = false;
    QPoint lastPanPosition_;
    QPoint selectionDragStart_;
    QRectF marqueeRect_;
    QVector<SelectedKey> selectedKeys_;
};

class GraphEditorPanel : public QWidget
{
public:
    explicit GraphEditorPanel(QWidget* parent = nullptr);

    void setViewModel(const GraphEditorViewModel& viewModel);
    void setCurrentFrameChangedCallback(std::function<void(int)> callback);

private:
    void syncVisibleCurves();

    QListWidget* curveListWidget_ = nullptr;
    GraphEditorCanvasWidget* canvasWidget_ = nullptr;
    QLabel* objectNameLabel_ = nullptr;
    QLabel* summaryLabel_ = nullptr;
    GraphEditorViewModel viewModel_;
};
