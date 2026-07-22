#include "AnimationTimelinePanel.h"

#include <QAction>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>

#include "KeyframeTimelineWidget.h"

namespace
{
QPushButton* createTransportButton(const QString& label, QWidget* parent)
{
    QPushButton* button = new QPushButton(label, parent);
    button->setFixedWidth(34);
    button->setFocusPolicy(Qt::NoFocus);
    return button;
}
}

AnimationTimelinePanel::AnimationTimelinePanel(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(8, 8, 8, 8);
    rootLayout->setSpacing(8);

    QHBoxLayout* topRow = new QHBoxLayout();
    topRow->setSpacing(6);

    playbackStartSpinBox_ = new QSpinBox(this);
    playbackStartSpinBox_->setObjectName("playbackStartSpinBox");
    playbackStartSpinBox_->setRange(-10000, 100000);
    topRow->addWidget(new QLabel("Start", this));
    topRow->addWidget(playbackStartSpinBox_);

    playbackEndSpinBox_ = new QSpinBox(this);
    playbackEndSpinBox_->setObjectName("playbackEndSpinBox");
    playbackEndSpinBox_->setRange(-10000, 100000);
    topRow->addWidget(new QLabel("End", this));
    topRow->addWidget(playbackEndSpinBox_);

    autoKeyButton_ = new QPushButton("Auto Key", this);
    autoKeyButton_->setObjectName("autoKeyButton");
    autoKeyButton_->setCheckable(true);
    topRow->addWidget(autoKeyButton_);

    setKeyButton_ = new QPushButton("Key Selected", this);
    setKeyButton_->setObjectName("setKeyButton");
    topRow->addWidget(setKeyButton_);

    deleteKeyButton_ = new QPushButton("Delete Key", this);
    deleteKeyButton_->setObjectName("deleteKeyButton");
    topRow->addWidget(deleteKeyButton_);

    duplicateKeyButton_ = new QPushButton("Duplicate Key", this);
    duplicateKeyButton_->setObjectName("duplicateKeyButton");
    topRow->addWidget(duplicateKeyButton_);

    shiftKeysLeftButton_ = new QPushButton("Shift -1", this);
    shiftKeysLeftButton_->setObjectName("shiftKeysLeftButton");
    topRow->addWidget(shiftKeysLeftButton_);

    shiftKeysRightButton_ = new QPushButton("Shift +1", this);
    shiftKeysRightButton_->setObjectName("shiftKeysRightButton");
    topRow->addWidget(shiftKeysRightButton_);

    currentFrameSpinBox_ = new QSpinBox(this);
    currentFrameSpinBox_->setObjectName("currentFrameSpinBox");
    currentFrameSpinBox_->setRange(-10000, 100000);
    topRow->addWidget(new QLabel("Current", this));
    topRow->addWidget(currentFrameSpinBox_);

    rootLayout->addLayout(topRow);

    QWidget* ticksHost = new QWidget(this);
    QVBoxLayout* ticksLayout = new QVBoxLayout(ticksHost);
    ticksLayout->setContentsMargins(0, 0, 0, 0);
    ticksLayout->setSpacing(2);

    keyframeTimelineWidget_ = new KeyframeTimelineWidget(ticksHost);
    keyframeTimelineWidget_->setObjectName("keyframeTimelineWidget");
    ticksLayout->addWidget(keyframeTimelineWidget_);

    timeSlider_ = new QSlider(Qt::Horizontal, ticksHost);
    timeSlider_->setObjectName("timeSlider");
    timeSlider_->setTickPosition(QSlider::TicksBelow);
    timeSlider_->setTickInterval(1);
    timeSlider_->setPageStep(1);
    ticksLayout->addWidget(timeSlider_);

    rootLayout->addWidget(ticksHost);

    QHBoxLayout* controlsRow = new QHBoxLayout();
    controlsRow->setSpacing(6);

    QPushButton* jumpStartButton = createTransportButton("|<", this);
    jumpStartButton->setObjectName("jumpStartButton");
    previousKeyButton_ = createTransportButton("<<", this);
    previousKeyButton_->setObjectName("previousKeyButton");
    QPushButton* stepBackButton = createTransportButton("<", this);
    stepBackButton->setObjectName("stepBackButton");
    playPauseButton_ = createTransportButton(">", this);
    playPauseButton_->setObjectName("playPauseButton");
    QPushButton* stepForwardButton = createTransportButton(">", this);
    stepForwardButton->setObjectName("stepForwardButton");
    nextKeyButton_ = createTransportButton(">>", this);
    nextKeyButton_->setObjectName("nextKeyButton");
    QPushButton* jumpEndButton = createTransportButton(">|", this);
    jumpEndButton->setObjectName("jumpEndButton");

    controlsRow->addWidget(jumpStartButton);
    controlsRow->addWidget(previousKeyButton_);
    controlsRow->addWidget(stepBackButton);
    controlsRow->addWidget(playPauseButton_);
    controlsRow->addWidget(stepForwardButton);
    controlsRow->addWidget(nextKeyButton_);
    controlsRow->addWidget(jumpEndButton);
    controlsRow->addStretch(1);

    rootLayout->addLayout(controlsRow);

    timelineStatusLabel_ = new QLabel("No selection", this);
    timelineStatusLabel_->setObjectName("timelineStatusLabel");
    timelineStatusLabel_->setStyleSheet("color: #bdbdbd;");
    rootLayout->addWidget(timelineStatusLabel_);

    QObject::connect(playbackStartSpinBox_, qOverload<int>(&QSpinBox::valueChanged), this, [this](int) {
        if (!updatingTimelineControls_ && playbackRangeChangedCallback_) {
            playbackRangeChangedCallback_(playbackStartSpinBox_->value(), playbackEndSpinBox_->value());
        }
    });
    QObject::connect(playbackEndSpinBox_, qOverload<int>(&QSpinBox::valueChanged), this, [this](int) {
        if (!updatingTimelineControls_ && playbackRangeChangedCallback_) {
            playbackRangeChangedCallback_(playbackStartSpinBox_->value(), playbackEndSpinBox_->value());
        }
    });
    QObject::connect(currentFrameSpinBox_, qOverload<int>(&QSpinBox::valueChanged), this, [this](int frame) {
        if (!updatingTimelineControls_ && currentFrameChangedCallback_) {
            currentFrameChangedCallback_(frame);
        }
    });
    QObject::connect(timeSlider_, &QSlider::valueChanged, this, [this](int frame) {
        if (!updatingTimelineControls_ && currentFrameChangedCallback_) {
            currentFrameChangedCallback_(frame);
        }
    });
    QObject::connect(jumpStartButton, &QPushButton::clicked, this, [this]() {
        if (jumpStartCallback_) {
            jumpStartCallback_();
        }
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
    QObject::connect(jumpEndButton, &QPushButton::clicked, this, [this]() {
        if (jumpEndCallback_) {
            jumpEndCallback_();
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
    QObject::connect(autoKeyButton_, &QPushButton::toggled, this, [this](bool enabled) {
        if (!updatingTimelineControls_ && autoKeyChangedCallback_) {
            autoKeyChangedCallback_(enabled);
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
        QSignalBlocker blockStart(playbackStartSpinBox_);
        QSignalBlocker blockEnd(playbackEndSpinBox_);
        QSignalBlocker blockCurrent(currentFrameSpinBox_);
        QSignalBlocker blockSlider(timeSlider_);
        QSignalBlocker blockAuto(autoKeyButton_);

        playbackStartSpinBox_->setValue(viewModel.playbackStartFrame);
        playbackEndSpinBox_->setValue(viewModel.playbackEndFrame);
        currentFrameSpinBox_->setRange(viewModel.playbackStartFrame, viewModel.playbackEndFrame);
        currentFrameSpinBox_->setValue(viewModel.currentFrame);
        timeSlider_->setRange(viewModel.playbackStartFrame, viewModel.playbackEndFrame);
        timeSlider_->setValue(viewModel.currentFrame);
        autoKeyButton_->setChecked(viewModel.autoKeyEnabled);
    }
    updatingTimelineControls_ = false;

    keyframeTimelineWidget_->setFrameRange(viewModel.playbackStartFrame, viewModel.playbackEndFrame);
    keyframeTimelineWidget_->setCurrentFrame(viewModel.currentFrame);
    keyframeTimelineWidget_->setKeyframes(viewModel.keyframes);
    keyframeTimelineWidget_->setCurrentFrameKeyed(viewModel.currentFrameKeyed);

    setKeyButton_->setEnabled(viewModel.hasSelection);
    setKeyButton_->setText("Key Selected");
    setKeyButton_->setStyleSheet(viewModel.setKeyStyle);

    deleteKeyButton_->setEnabled(viewModel.hasSelection && viewModel.currentFrameKeyed);
    deleteKeyButton_->setStyleSheet(viewModel.deleteKeyStyle);
    duplicateKeyButton_->setEnabled(viewModel.hasSelection && viewModel.currentFrameKeyed);
    shiftKeysLeftButton_->setEnabled(viewModel.hasSelection && viewModel.hasAnyKeys);
    shiftKeysRightButton_->setEnabled(viewModel.hasSelection && viewModel.hasAnyKeys);
    previousKeyButton_->setEnabled(viewModel.hasSelection && viewModel.hasAnyKeys);
    nextKeyButton_->setEnabled(viewModel.hasSelection && viewModel.hasAnyKeys);
    autoKeyButton_->setStyleSheet(viewModel.autoKeyStyle);
    playPauseButton_->setText(viewModel.playPauseText);
    timelineStatusLabel_->setText(viewModel.statusText);
    timelineStatusLabel_->setStyleSheet(viewModel.statusStyle);

    syncActionState(viewModel);
}

void AnimationTimelinePanel::setPlaybackRangeChangedCallback(std::function<void(int, int)> callback) { playbackRangeChangedCallback_ = std::move(callback); }
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
