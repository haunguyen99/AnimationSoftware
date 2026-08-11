#pragma once

#include <QWidget>

#include <functional>

#include "EditorAnimationTimelineViewModel.h"

class QAction;
class QLabel;
class QMenu;
class QPushButton;
class QSpinBox;
class KeyframeTimelineWidget;

class AnimationTimelinePanel : public QWidget
{
public:
    explicit AnimationTimelinePanel(QWidget* parent = nullptr);

    void bindTimelineActions(
        QAction* duplicateKeyAction,
        QAction* shiftKeysLeftAction,
        QAction* shiftKeysRightAction,
        QAction* previousKeyAction,
        QAction* nextKeyAction);
    void setViewModel(const EditorAnimationTimelineViewModel& viewModel);

    void setPlaybackRangeChangedCallback(std::function<void(int, int)> callback);
    void setSelectedFrameRangeChangedCallback(std::function<void(int, int)> callback);
    void setCurrentFrameChangedCallback(std::function<void(int)> callback);
    void setJumpStartCallback(std::function<void()> callback);
    void setStepBackCallback(std::function<void()> callback);
    void setTogglePlaybackCallback(std::function<void()> callback);
    void setStepForwardCallback(std::function<void()> callback);
    void setJumpEndCallback(std::function<void()> callback);
    void setPreviousKeyCallback(std::function<void()> callback);
    void setNextKeyCallback(std::function<void()> callback);
    void setSetKeyCallback(std::function<void()> callback);
    void setDeleteKeyCallback(std::function<void()> callback);
    void setDuplicateKeyCallback(std::function<void()> callback);
    void setShiftKeysLeftCallback(std::function<void()> callback);
    void setShiftKeysRightCallback(std::function<void()> callback);
    void setAutoKeyChangedCallback(std::function<void(bool)> callback);

private:
    void syncActionState(const EditorAnimationTimelineViewModel& viewModel);
    void showTimelineContextMenu(const QPoint& position);

    QSpinBox* currentFrameSpinBox_ = nullptr;
    QPushButton* setKeyButton_ = nullptr;
    QPushButton* deleteKeyButton_ = nullptr;
    QPushButton* duplicateKeyButton_ = nullptr;
    QPushButton* shiftKeysLeftButton_ = nullptr;
    QPushButton* shiftKeysRightButton_ = nullptr;
    KeyframeTimelineWidget* keyframeTimelineWidget_ = nullptr;
    QPushButton* playPauseButton_ = nullptr;
    QPushButton* previousKeyButton_ = nullptr;
    QPushButton* nextKeyButton_ = nullptr;
    QLabel* timelineStatusLabel_ = nullptr;
    QAction* duplicateKeyAction_ = nullptr;
    QAction* shiftKeysLeftAction_ = nullptr;
    QAction* shiftKeysRightAction_ = nullptr;
    QAction* previousKeyAction_ = nullptr;
    QAction* nextKeyAction_ = nullptr;
    QMenu* timelineContextMenu_ = nullptr;
    QAction* contextSetKeyAction_ = nullptr;
    QAction* contextDeleteKeyAction_ = nullptr;
    bool updatingTimelineControls_ = false;
    std::function<void(int, int)> playbackRangeChangedCallback_;
    std::function<void(int, int)> selectedFrameRangeChangedCallback_;
    std::function<void(int)> currentFrameChangedCallback_;
    std::function<void()> jumpStartCallback_;
    std::function<void()> stepBackCallback_;
    std::function<void()> togglePlaybackCallback_;
    std::function<void()> stepForwardCallback_;
    std::function<void()> jumpEndCallback_;
    std::function<void()> previousKeyCallback_;
    std::function<void()> nextKeyCallback_;
    std::function<void()> setKeyCallback_;
    std::function<void()> deleteKeyCallback_;
    std::function<void()> duplicateKeyCallback_;
    std::function<void()> shiftKeysLeftCallback_;
    std::function<void()> shiftKeysRightCallback_;
    std::function<void(bool)> autoKeyChangedCallback_;
};
