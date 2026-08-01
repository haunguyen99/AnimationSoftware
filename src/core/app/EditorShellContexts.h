#pragma once

#include <QAction>
#include <QPlainTextEdit>

#include <cstdint>
#include <functional>

#include "animation/editor/EditorAnimationFlowController.h"
#include "EditorChannelBoxController.h"
#include "EditorCreationController.h"
#include "EditorInspectorController.h"
#include "EditorOutlinerController.h"
#include "rigging/editor/EditorRiggingController.h"
#include "EditorSelectionController.h"
#include "EditorViewportUiController.h"

class QLabel;
class QPushButton;
class QDoubleSpinBox;
class QCheckBox;
class QWidget;
class QTreeWidget;
class ViewportWorkspaceWidget;

namespace EditorShellContexts
{
EditorInspectorController::InspectorWidgets buildInspectorWidgets(
    QLabel* inspectorEmptyStateLabel,
    QLabel* channelObjectNameLabel,
    QDoubleSpinBox* translateXSpinBox,
    QDoubleSpinBox* translateYSpinBox,
    QDoubleSpinBox* translateZSpinBox,
    QDoubleSpinBox* rotateXSpinBox,
    QDoubleSpinBox* rotateYSpinBox,
    QDoubleSpinBox* rotateZSpinBox,
    QDoubleSpinBox* scaleXSpinBox,
    QDoubleSpinBox* scaleYSpinBox,
    QDoubleSpinBox* scaleZSpinBox,
    QDoubleSpinBox* jointOrientXSpinBox,
    QDoubleSpinBox* jointOrientYSpinBox,
    QDoubleSpinBox* jointOrientZSpinBox,
    QLabel* bindPoseStatusLabel,
    QLabel* skinBindingStatusLabel,
    QCheckBox* visibilityCheckBox,
    QWidget* inspectorDetailsWidget,
    QWidget* jointToolsWidget);

EditorInspectorController::InspectorActions buildInspectorActions(
    QAction* markHierarchyParentAction,
    QAction* parentToMarkedParentAction,
    QAction* unparentSelectedAction,
    QAction* bindSkinAction,
    QAction* resetJointOrientationAction,
    QAction* alignJointOrientationAction,
    QAction* captureBindPoseAction,
    QAction* captureBindPoseRecursiveAction,
    QAction* frameSelectedAction,
    QPushButton* frameSelectedButton,
    QPushButton* resetJointOrientationButton,
    QPushButton* alignJointOrientationButton,
    QPushButton* captureBindPoseButton,
    QPushButton* captureBindPoseRecursiveButton);

EditorChannelBoxController::Context buildChannelBoxContext(ViewportWorkspaceWidget* viewport);

EditorCreationController::Context buildCreationContext(
    std::function<QString(const QString&)> generateUniqueScriptName,
    std::function<QString(const QString&, std::uint64_t)> generateUniqueObjectName);

EditorOutlinerController::SceneAccess buildOutlinerSceneAccess(ViewportWorkspaceWidget* viewport);

EditorRiggingController::Context buildRiggingContext(ViewportWorkspaceWidget* viewport);

EditorSelectionController::Context buildSelectionContext(
    ViewportWorkspaceWidget* viewport,
    QTreeWidget* outlinerTree,
    QPlainTextEdit* scriptHistoryTextEdit,
    const EditorInspectorController::InspectorWidgets& inspectorWidgets,
    const EditorInspectorController::InspectorActions& inspectorActions,
    const EditorOutlinerController::SceneAccess& outlinerSceneAccess);

EditorViewportUiController::ActionSet buildViewportUiActions(
    QAction* translateAction,
    QAction* rotateAction,
    QAction* scaleAction,
    QAction* perspectiveCameraAction,
    QAction* frontCameraAction,
    QAction* backCameraAction,
    QAction* leftCameraAction,
    QAction* rightCameraAction,
    QAction* topCameraAction,
    QAction* bottomCameraAction,
    QAction* worldAxisAction,
    QAction* localAxisAction);

EditorAnimationFlowController::Context buildAnimationFlowContext(ViewportWorkspaceWidget* viewport);
}
