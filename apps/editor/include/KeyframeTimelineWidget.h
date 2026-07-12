#pragma once

#include <QWidget>

#include <QVector>

class KeyframeTimelineWidget : public QWidget
{
public:
    explicit KeyframeTimelineWidget(QWidget* parent = nullptr);

    void setFrameRange(int startFrame, int endFrame);
    void setCurrentFrame(int frame);
    void setKeyframes(const QVector<int>& keyframes);
    void setCurrentFrameKeyed(bool keyed);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    qreal frameToPosition(int frame) const;

    int startFrame_ = 0;
    int endFrame_ = 24;
    int currentFrame_ = 0;
    QVector<int> keyframes_;
    bool currentFrameKeyed_ = false;
};
