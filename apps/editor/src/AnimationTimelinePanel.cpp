#include "AnimationTimelinePanel.h"

#include <QAction>
#include <QAbstractSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStyle>
#include <QVBoxLayout>

#include "KeyframeTimelineWidget.h"

namespace
{
void refreshStyle(QWidget* widget)
{
    if (widget == nullptr) {
        return;
    }

    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();
}

QPushButton* createTransportButton(const QString& label, QWidget* parent)
{
    QPushButton* button = new QPushButton(label, parent);
    button->setFixedSize(24, 24);
    button->setFocusPolicy(Qt::NoFocus);
    button->setProperty("variant", "transport");
    return button;
}
}

AnimationTimelinePanel::AnimationTimelinePanel(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(8, 5, 8, 5);
    rootLayout->setSpacing(3);

    duplicateKeyButton_ = new QPushButton("Duplicate", this);
    duplicateKeyButton_->setObjectName("duplicateKeyButton");
    duplicateKeyButton_->hide();

    shiftKeysLeftButton_ = new QPushButton("Shift -1", this);
    shiftKeysLeftButton_->setObjectName("shiftKeysLeftButton");
    shiftKeysLeftButton_->hide();

    shiftKeysRightButton_ = new QPushButton("Shift +1", this);
    shiftKeysRightButton_->setObjectName("shiftKeysRightButton");
    shiftKeysRightButton_->hide();

    QWidget* sliderRowHost = new QWidget(this);
    QHBoxLayout* sliderRow = new QHBoxLayout(sliderRowHost);
    sliderRow->setContentsMargins(0, 0, 0, 0);
    sliderRow->setSpacing(10);

    QWidget* ticksHost = new QWidget(sliderRowHost);
    QVBoxLayout* ticksLayout = new QVBoxLayout(ticksHost);
    ticksLayout->setContentsMargins(0, 0, 0, 0);
    ticksLayout->setSpacing(1);

    keyframeTimelineWidget_ = new KeyframeTimelineWidget(ticksHost);
    keyframeTimelineWidget_->setObjectName("keyframeTimelineWidget");
    keyframeTimelineWidget_->setContextMenuPolicy(Qt::CustomContextMenu);
    ticksLayout->addWidget(keyframeTimelineWidget_);
    keyframeTimelineWidget_->setSelectionChangedCallback([this](int startFrame, int endFrame) {
        if (!updatingTimelineControls_ && selectedFrameRangeChangedCallback_) {
            selectedFrameRangeChangedCallback_(startFrame, endFrame);
        }
    });
    keyframeTimelineWidget_->setCurrentFrameChangedCallback([this](int frame) {
        if (!updatingTimelineControls_ && currentFrameChangedCallback_) {
            currentFrameChangedCallback_(frame);
        }
    });
    sliderRow->addWidget(ticksHost, 1);

    QHBoxLayout* controlsRow = new QHBoxLayout();
    controlsRow->setSpacing(2);

    currentFrameSpinBox_ = new QSpinBox(sliderRowHost);
    currentFrameSpinBox_->setObjectName("currentFrameSpinBox");
    currentFrameSpinBox_->setRange(-10000, 100000);
    currentFrameSpinBox_->setFixedSize(46, 24);
    currentFrameSpinBox_->setButtonSymbols(QAbstractSpinBox::NoButtons);
    currentFrameSpinBox_->setAlignment(Qt::AlignCenter);
    currentFrameSpinBox_->setProperty("variant", "frameBox");
    controlsRow->addWidget(currentFrameSpinBox_);

    previousKeyButton_ = createTransportButton("|<", sliderRowHost);
    previousKeyButton_->setObjectName("previousKeyButton");
    QPushButton* stepBackButton = createTransportButton("<", sliderRowHost);
    stepBackButton->setObjectName("stepBackButton");
    playPauseButton_ = createTransportButton(">", sliderRowHost);
    playPauseButton_->setObjectName("playPauseButton");
    QPushButton* stepForwardButton = createTransportButton(">", sliderRowHost);
    stepForwardButton->setObjectName("stepForwardButton");
    nextKeyButton_ = createTransportButton(">|", sliderRowHost);
    nextKeyButton_->setObjectName("nextKeyButton");

    controlsRow->addWidget(previousKeyButton_);
    controlsRow->addWidget(stepBackButton);
    controlsRow->addWidget(playPauseButton_);
    controlsRow->addWidget(stepForwardButton);
    controlsRow->addWidget(nextKeyButton_);
    sliderRow->addLayout(controlsRow);
    rootLayout->addWidget(sliderRowHost);

    // Status row: timeline status text + set/delete key quick-access buttons
    QWidget* statusRowHost = new QWidget(this);
    QHBoxLayout* statusRow = new QHBoxLayout(statusRowHost);
    statusRow->setContentsMargins(0, 1, 0, 0);
    statusRow->setSpacing(4);

    timelineStatusLabel_ = new QLabel(statusRowHost);
    timelineStatusLabel_->setObjectName("timelineStatusLabel");
    timelineStatusLabel_->setProperty("statusTone", "muted");
    statusRow->addWidget(timelineStatusLabel_, 1);

    setKeyButton_ = new QPushButton("Key Selected", statusRowHost);
    setKeyButton_->setObjectName("setKeyButton");
    setKeyButton_->setEnabled(false);
    setKeyButton_->setFixedHeight(22);
    statusRow->addWidget(setKeyButton_);

    deleteKeyButton_ = new QPushButton("Delete Key", statusRowHost);
    deleteKeyButton_->setObjectName("deleteKeyButton");
    deleteKeyButton_->setEnabled(false);
    deleteKeyButton_->setFixedHeight(22);
    statusRow->addWidget(deleteKeyButton_);

    rootLayout->addWidget(statusRowHost);

    timelineContextMenu_ = new QMenu(this);
    contextSetKeyAction_ = timelineContextMenu_->addAction("Key Selected");
    contextDeleteKeyAction_ = timelineContextMenu_->addAction("Delete Key");
    QObject::connect(currentFrameSpinBox_, qOverload<int>(&QSpinBox::valueChanged), this, [this](int frame) {
        if (!updatingTimelineControls_ && currentFrameChangedCallback_) {
            currentFrameChangedCallback_(frame);
        }
    });
    QObject::connect(keyframeTimelineWidget_, &QWidget::customContextMenuRequested, this, [this](const QPoint& position) {
        showTimelineContextMenu(position);
    });
    QObject::connect(previousKeyButton_, &QPushButton::clicked, this, [this]() {
        if (previousKeyCallback_) {
            previousKeyCallback_();
        }
    });
    QObject::connect(stepBackButton, &QPushButton::clicked, this, [this]() {
        if (stepBackCallback_) {
            stepBackCallback_();
        }
    });
    QObject::connect(playPauseButton_, &QPushButton::clicked, this, [this]() {
        if (togglePlaybackCallback_) {
            togglePlaybackCallback_();
        }
    });
    QObject::connect(stepForwardButton, &QPushButton::clicked, this, [this]() {
        if (stepForwardCallback_) {
            stepForwardCallback_();
        }
    });
    QObject::connect(nextKeyButton_, &QPushButton::clicked, this, [this]() {
        if (nextKeyCallback_) {
            nextKeyCallback_();
        }
    });
    QObject::connect(contextSetKeyAction_, &QAction::triggered, this, [this]() {
        if (setKeyCallback_) {
            setKeyCallback_();
        }
    });
    QObject::connect(contextDeleteKeyAction_, &QAction::triggered, this, [this]() {
        if (deleteKeyCallback_) {
            deleteKeyCallback_();
        }
    });
    QObject::connect(duplicateKeyButton_, &QPushButton::clicked, this, [this]() {
        if (duplicateKeyCallback_) {
            duplicateKeyCallback_();
        }
    });
    QObject::connect(shiftKeysLeftButton_, &QPushButton::clicked, this, [this]() {
        if (shiftKeysLeftCallback_) {
            shiftKeysLeftCallback_();
        }
    });
    QObject::connect(shiftKeysRightButton_, &QPushButton::clicked, this, [this]() {
        if (shiftKeysRightCallback_) {
            shiftKeysRightCallback_();
        }
    });
    QObject::connect(setKeyButton_, &QPushButton::clicked, this, [this]() {
        if (setKeyCallback_) {
            setKeyCallback_();
        }
    });
    QObject::connect(deleteKeyButton_, &QPushButton::clicked, this, [this]() {
        if (deleteKeyCallback_) {
            deleteKeyCallback_();
        }
    });
}

void AnimationTimelinePanel::bindTimelineActions(
    QAction* duplicateKeyAction,
    QAction* shiftKeysLeftAction,
    QAction* shiftKeysRightAction,
    QAction* previousKeyAction,
    QAction* nextKeyAction)
{
    duplicateKeyAction_ = duplicateKeyAction;
    shiftKeysLeftAction_ = shiftKeysLeftAction;
    shiftKeysRightAction_ = shiftKeysRightAction;
    previousKeyAction_ = previousKeyAction;
    nextKeyAction_ = nextKeyAction;
}

void AnimationTimelinePanel::setViewModel(const EditorAnimationTimelineViewModel& viewModel)
{
    updatingTimelineControls_ = true;
    {
        QSignalBlocker blockCurrent(currentFrameSpinBox_);
        currentFrameSpinBox_->setRange(viewModel.playbackStartFrame, viewModel.playbackEndFrame);
        currentFrameSpinBox_->setValue(viewModel.currentFrame);
    }
    updatingTimelineControls_ = false;

    keyframeTimelineWidget_->setFrameRange(viewModel.playbackStartFrame, viewModel.playbackEndFrame);
    keyframeTimelineWidget_->setCurrentFrame(viewModel.currentFrame);
    keyframeTimelineWidget_->setKeyframes(viewModel.keyframes);
    keyframeTimelineWidget_->setCurrentFrameKeyed(viewModel.currentFrameKeyed);
    keyframeTimelineWidget_->setSelectedFrameRange(
        viewModel.selectedRangeStartFrame,
        viewModel.selectedRangeEndFrame,
        viewModel.hasSelectedRange);

    contextSetKeyAction_->setEnabled(viewModel.hasSelection);
    contextDeleteKeyAction_->setEnabled(viewModel.hasSelection && viewModel.currentFrameKeyed);
    previousKeyButton_->setEnabled(viewModel.hasSelection && viewModel.hasAnyKeys);
    nextKeyButton_->setEnabled(viewModel.hasSelection && viewModel.hasAnyKeys);
    playPauseButton_->setText(viewModel.playPauseText);

    setKeyButton_->setEnabled(viewModel.hasSelection);
    setKeyButton_->setStyleSheet(viewModel.setKeyStyle);
    deleteKeyButton_->setEnabled(viewModel.hasSelection && viewModel.currentFrameKeyed);
    deleteKeyButton_->setStyleSheet(viewModel.deleteKeyStyle);
    duplicateKeyButton_->setEnabled(viewModel.hasSelection && viewModel.currentFrameKeyed);

    timelineStatusLabel_->setText(viewModel.statusText);
    timelineStatusLabel_->setStyleSheet(viewModel.statusStyle);
    refreshStyle(timelineStatusLabel_);

    syncActionState(viewModel);
}

void AnimationTimelinePanel::setPlaybackRangeChangedCallback(std::function<void(int, int)> callback) { playbackRangeChangedCallback_ = std::move(callback); }
void AnimationTimelinePanel::setSelectedFrameRangeChangedCallback(std::function<void(int, int)> callback) { selectedFrameRangeChangedCallback_ = std::move(callback); }
void AnimationTimelinePanel::setCurrentFrameChangedCallback(std::function<void(int)> callback) { currentFrameChangedCallback_ = std::move(callback); }
void AnimationTimelinePanel::setJumpStartCallback(std::function<void()> callback) { jumpStartCallback_ = std::move(callback); }
void AnimationTimelinePanel::setStepBackCallback(std::function<void()> callback) { stepBackCallback_ = std::move(callback); }
void AnimationTimelinePanel::setTogglePlaybackCallback(std::function<void()> callback) { togglePlaybackCallback_ = std::move(callback); }
void AnimationTimelinePanel::setStepForwardCallback(std::function<void()> callback) { stepForwardCallback_ = std::move(callback); }
void AnimationTimelinePanel::setJumpEndCallback(std::function<void()> callback) { jumpEndCallback_ = std::move(callback); }
void AnimationTimelinePanel::setPreviousKeyCallback(std::function<void()> callback) { previousKeyCallback_ = std::move(callback); }
void AnimationTimelinePanel::setNextKeyCallback(std::function<void()> callback) { nextKeyCallback_ = std::move(callback); }
void AnimationTimelinePanel::setSetKeyCallback(std::function<void()> callback) { setKeyCallback_ = std::move(callback); }
void AnimationTimelinePanel::setDeleteKeyCallback(std::function<void()> callback) { deleteKeyCallback_ = std::move(callback); }
void AnimationTimelinePanel::setDuplicateKeyCallback(std::function<void()> callback) { duplicateKeyCallback_ = std::move(callback); }
void AnimationTimelinePanel::setShiftKeysLeftCallback(std::function<void()> callback) { shiftKeysLeftCallback_ = std::move(callback); }
void AnimationTimelinePanel::setShiftKeysRightCallback(std::function<void()> callback) { shiftKeysRightCallback_ = std::move(callback); }
void AnimationTimelinePanel::setAutoKeyChangedCallback(std::function<void(bool)> callback) { autoKeyChangedCallback_ = std::move(callback); }

void AnimationTimelinePanel::showTimelineContextMenu(const QPoint& position)
{
    if (timelineContextMenu_ == nullptr || keyframeTimelineWidget_ == nullptr) {
        return;
    }

    timelineContextMenu_->popup(keyframeTimelineWidget_->mapToGlobal(position));
}

void AnimationTimelinePanel::syncActionState(const EditorAnimationTimelineViewModel& viewModel)
{
    if (duplicateKeyAction_ != nullptr) {
        duplicateKeyAction_->setEnabled(viewModel.hasSelection && viewModel.currentFrameKeyed);
    }
    if (shiftKeysLeftAction_ != nullptr) {
        shiftKeysLeftAction_->setEnabled(viewModel.hasSelection && viewModel.hasAnyKeys);
    }
    if (shiftKeysRightAction_ != nullptr) {
        shiftKeysRightAction_->setEnabled(viewModel.hasSelection && viewModel.hasAnyKeys);
    }
    if (previousKeyAction_ != nullptr) {
        previousKeyAction_->setEnabled(viewModel.hasSelection && viewModel.hasAnyKeys);
    }
    if (nextKeyAction_ != nullptr) {
        nextKeyAction_->setEnabled(viewModel.hasSelection && viewModel.hasAnyKeys);
    }
}
