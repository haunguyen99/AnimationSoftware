#pragma once

#include <QWidget>

#include <functional>

#include "EditorAnimationTimelineViewModel.h"

class QSpinBox;
class QComboBox;
class QPushButton;

class RangeSliderTrackWidget : public QWidget
{
public:
    explicit RangeSliderTrackWidget(QWidget* parent = nullptr);

    void setFrames(
        int visibleStartFrame,
        int visibleEndFrame,
        int playbackStartFrame,
        int playbackEndFrame,
        int currentFrame);
    void setPlaybackRangeChangedCallback(std::function<void(int, int)> callback);
    void setVisibleRangeChangedCallback(std::function<void(int, int)> callback);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    enum class DragMode
    {
        None,
        StartHandle,
        EndHandle,
        MovePlaybackRange,
        PanVisibleRange
    };

    void emitRangeChangedIfNeeded();
    void emitVisibleRangeChangedIfNeeded();
    qreal frameToPosition(int frame) const;
    int positionToFrame(int x) const;

    int visibleStartFrame_ = 0;
    int visibleEndFrame_ = 48;
    int playbackStartFrame_ = 0;
    int playbackEndFrame_ = 24;
    int currentFrame_ = 0;
    int dragAnchorFrame_ = 0;
    int dragStartPlaybackStartFrame_ = 0;
    int dragStartPlaybackEndFrame_ = 24;
    int dragStartVisibleStartFrame_ = 0;
    int dragStartVisibleEndFrame_ = 48;
    DragMode dragMode_ = DragMode::None;
    std::function<void(int, int)> playbackRangeChangedCallback_;
    std::function<void(int, int)> visibleRangeChangedCallback_;
};

class RangeSliderPanel : public QWidget
{
public:
    explicit RangeSliderPanel(QWidget* parent = nullptr);

    void setViewModel(const EditorAnimationTimelineViewModel& viewModel);
    void setPlaybackRangeChangedCallback(std::function<void(int, int)> callback);
    void setVisibleRangeChangedCallback(std::function<void(int, int)> callback);
    void setAutoKeyChangedCallback(std::function<void(bool)> callback);
    void setFramesPerSecondChangedCallback(std::function<void(int)> callback);

private:
    bool updatingControls_ = false;
    QPushButton* autoKeyButton_ = nullptr;
    QSpinBox* startFrameSpinBox_ = nullptr;
    QSpinBox* endFrameSpinBox_ = nullptr;
    RangeSliderTrackWidget* rangeSliderTrackWidget_ = nullptr;
    QComboBox* fpsComboBox_ = nullptr;
    std::function<void(int, int)> playbackRangeChangedCallback_;
    std::function<void(int, int)> visibleRangeChangedCallback_;
    std::function<void(bool)> autoKeyChangedCallback_;
    std::function<void(int)> framesPerSecondChangedCallback_;
};
