#pragma once

#include <QColor>
#include <QContextMenuEvent>
#include <QPoint>
#include <QRectF>
#include <QString>
#include <QVector>
#include <QWidget>

#include <functional>

#include "animation/data/KeyTangent.h"

class QLabel;
class QListWidget;
class QMouseEvent;
class QPaintEvent;
class QToolBar;
class QWheelEvent;

// ---------------------------------------------------------------------------
// View-model types
// ---------------------------------------------------------------------------

struct GraphEditorCurvePoint
{
    int frame = 0;
    double value = 0.0;
    // Phase 3: tangent data (populated from scene keyframe tangent)
    float inAngle       = 0.f;
    float outAngle      = 0.f;
    TangentMode tangentMode = TangentMode::Auto;
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

// ---------------------------------------------------------------------------
// Edit types (Phase 2 / 3 / 4)
// ---------------------------------------------------------------------------

struct GraphEditorKeyEdit
{
    QString curveId;
    int oldFrame  = 0;
    int newFrame  = 0;
    double newValue = 0.0;
};

struct TangentEdit
{
    QString curveId;
    int frame       = 0;
    bool isInHandle = true;
    float newAngle  = 0.f;
};

struct TangentModeChange
{
    QString curveId;
    int frame              = 0;
    TangentMode newMode    = TangentMode::Auto;
};

// ---------------------------------------------------------------------------
// Canvas widget
// ---------------------------------------------------------------------------

class GraphEditorCanvasWidget : public QWidget
{
public:
    explicit GraphEditorCanvasWidget(QWidget* parent = nullptr);

    void setViewModel(const GraphEditorViewModel& viewModel, const QVector<QString>& visibleCurveIds);
    void setCurrentFrameChangedCallback(std::function<void(int)> callback);
    void setKeyEditedCallback(std::function<void(const GraphEditorKeyEdit&)> callback);
    void setTangentEditedCallback(std::function<void(const TangentEdit&)> callback);
    void setTangentModeChangedCallback(std::function<void(const TangentModeChange&)> callback);
    void setShowTangents(bool show);

    // Test helper: fire key edit callback directly.
    void fireKeyEditForTest(const GraphEditorKeyEdit& edit);

    void frameAll();
    void frameSelected();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    struct SelectedKey
    {
        QString curveId;
        int frame = 0;
        double value = 0.0;
    };

    struct TangentHandleDrag
    {
        QString curveId;
        int frame    = 0;
        bool isIn    = true;
    };

    void ensureViewRanges();
    void fitValueRangeToVisibleCurves();
    QVector<const GraphEditorCurve*> visibleCurves() const;
    QRectF plotRect() const;
    QRectF currentFrameHandleRect() const;
    QPointF pointToCanvasPosition(const GraphEditorCurvePoint& point) const;
    QPointF tangentHandlePosition(const QPointF& keyPos, float angleDeg, bool isOut) const;
    void clearInvalidSelection();
    bool trySelectKeyAt(const QPointF& position);
    void selectKeysInRect(const QRectF& selectionRect);
    bool isKeySelected(const QString& curveId, int frame, double value) const;
    int frameFromPosition(int x) const;
    double valueFromPosition(int y) const;
    // Returns the curve point for the selected key, or nullptr.
    const GraphEditorCurvePoint* findCurvePoint(const QString& curveId, int frame) const;
    // Hit-test tangent handles for selected keys; returns false if not hit.
    bool tryStartTangentDrag(const QPointF& position);
    // Angle (degrees) from key canvas-pos to handle canvas-pos.
    float angleFromCanvasDelta(const QPointF& delta) const;

    GraphEditorViewModel viewModel_;
    QVector<QString> visibleCurveIds_;
    std::function<void(int)> currentFrameChangedCallback_;
    std::function<void(const GraphEditorKeyEdit&)> keyEditedCallback_;
    std::function<void(const TangentEdit&)> tangentEditedCallback_;
    std::function<void(const TangentModeChange&)> tangentModeChangedCallback_;
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
    bool draggingKey_ = false;
    bool draggingTangent_ = false;
    bool showTangents_ = true;
    QPoint lastPanPosition_;
    QPoint selectionDragStart_;
    QRectF marqueeRect_;
    QVector<SelectedKey> selectedKeys_;
    // Key drag state
    SelectedKey dragKey_;
    int dragKeyOrigFrame_ = 0;
    double dragKeyOrigValue_ = 0.0;
    QPointF dragKeyCurrent_;
    // Tangent drag state
    TangentHandleDrag tangentDrag_;
    QPointF tangentDragKeyCanvasPos_;
};

// ---------------------------------------------------------------------------
// Panel widget
// ---------------------------------------------------------------------------

class GraphEditorPanel : public QWidget
{
public:
    explicit GraphEditorPanel(QWidget* parent = nullptr);

    void setViewModel(const GraphEditorViewModel& viewModel);
    void setCurrentFrameChangedCallback(std::function<void(int)> callback);
    void setKeyEditedCallback(std::function<void(const GraphEditorKeyEdit&)> callback);
    void setTangentEditedCallback(std::function<void(const TangentEdit&)> callback);
    void setTangentModeChangedCallback(std::function<void(const TangentModeChange&)> callback);

    // Test helper
    void fireKeyEditForTest(const GraphEditorKeyEdit& edit);

private:
    void syncVisibleCurves();

    QListWidget* curveListWidget_ = nullptr;
    GraphEditorCanvasWidget* canvasWidget_ = nullptr;
    QLabel* objectNameLabel_ = nullptr;
    QLabel* summaryLabel_ = nullptr;
    GraphEditorViewModel viewModel_;
};
