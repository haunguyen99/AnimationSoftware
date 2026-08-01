#pragma once

#include <QWidget>

#include <functional>
#include <QVector>

class KeyframeTimelineWidget : public QWidget
{
public:
    explicit KeyframeTimelineWidget(QWidget* parent = nullptr);

    void setFrameRange(int startFrame, int endFrame);
    void setCurrentFrame(int frame);
    void setKeyframes(const QVector<int>& keyframes);
    void setCurrentFrameKeyed(bool keyed);
    void setSelectedFrameRange(int startFrame, int endFrame, bool hasSelection);
    void setSelectionChangedCallback(std::function<void(int, int)> callback);
    void setCurrentFrameChangedCallback(std::function<void(int)> callback);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void applySelectionFromFrame(int frame);
    int positionToFrame(int x) const;
    qreal frameToPosition(int frame) const;

    int startFrame_ = 0;
    int endFrame_ = 24;
    int currentFrame_ = 0;
    int selectedRangeStartFrame_ = 0;
    int selectedRangeEndFrame_ = 0;
    int dragAnchorFrame_ = 0;
    QVector<int> keyframes_;
    bool currentFrameKeyed_ = false;
    bool hasSelectedRange_ = false;
    bool draggingSelection_ = false;
    std::function<void(int, int)> selectionChangedCallback_;
    std::function<void(int)> currentFrameChangedCallback_;
};
