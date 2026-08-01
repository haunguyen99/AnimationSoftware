#include "core/app/EditorShellContexts.h"

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>
#include <QWidget>

#include "ViewportWorkspaceWidget.h"

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
    QWidget* jointToolsWidget)
{
    EditorInspectorController::InspectorWidgets widgets;
    widgets.inspectorEmptyStateLabel = inspectorEmptyStateLabel;
    widgets.channelObjectNameLabel = channelObjectNameLabel;
    widgets.translateXSpinBox = translateXSpinBox;
    widgets.translateYSpinBox = translateYSpinBox;
    widgets.translateZSpinBox = translateZSpinBox;
    widgets.rotateXSpinBox = rotateXSpinBox;
    widgets.rotateYSpinBox = rotateYSpinBox;
    widgets.rotateZSpinBox = rotateZSpinBox;
    widgets.scaleXSpinBox = scaleXSpinBox;
    widgets.scaleYSpinBox = scaleYSpinBox;
    widgets.scaleZSpinBox = scaleZSpinBox;
    widgets.jointOrientXSpinBox = jointOrientXSpinBox;
    widgets.jointOrientYSpinBox = jointOrientYSpinBox;
    widgets.jointOrientZSpinBox = jointOrientZSpinBox;
    widgets.bindPoseStatusLabel = bindPoseStatusLabel;
    widgets.skinBindingStatusLabel = skinBindingStatusLabel;
    widgets.visibilityCheckBox = visibilityCheckBox;
    widgets.inspectorDetailsWidget = inspectorDetailsWidget;
    widgets.jointToolsWidget = jointToolsWidget;
    return widgets;
}

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
    QPushButton* captureBindPoseRecursiveButton)
{
    EditorInspectorController::InspectorActions actions;
    actions.markHierarchyParentAction = markHierarchyParentAction;
    actions.parentToMarkedParentAction = parentToMarkedParentAction;
    actions.unparentSelectedAction = unparentSelectedAction;
    actions.bindSkinAction = bindSkinAction;
    actions.resetJointOrientationAction = resetJointOrientationAction;
    actions.alignJointOrientationAction = alignJointOrientationAction;
    actions.captureBindPoseAction = captureBindPoseAction;
    actions.captureBindPoseRecursiveAction = captureBindPoseRecursiveAction;
    actions.frameSelectedAction = frameSelectedAction;
    actions.frameSelectedButton = frameSelectedButton;
    actions.resetJointOrientationButton = resetJointOrientationButton;
    actions.alignJointOrientationButton = alignJointOrientationButton;
    actions.captureBindPoseButton = captureBindPoseButton;
    actions.captureBindPoseRecursiveButton = captureBindPoseRecursiveButton;
    return actions;
}

EditorChannelBoxController::Context buildChannelBoxContext(ViewportWorkspaceWidget* viewport)
{
    EditorChannelBoxController::Context context;
    context.findObject = [viewport](std::uint64_t objectId) {
        return viewport->findObject(objectId);
    };
    context.setObjectLocalTransform = [viewport](std::uint64_t objectId, const Transform& transform) {
        return viewport->setObjectLocalTransform(objectId, transform);
    };
    context.setJointOrientation = [viewport](std::uint64_t objectId, const QQuaternion& orientation) {
        return viewport->setJointOrientation(objectId, orientation);
    };
    context.setObjectVisibility = [viewport](std::uint64_t objectId, bool visible) {
        return viewport->setObjectVisibility(objectId, visible);
    };
    return context;
}

EditorCreationController::Context buildCreationContext(
    std::function<QString(const QString&)> generateUniqueScriptName,
    std::function<QString(const QString&, std::uint64_t)> generateUniqueObjectName)
{
    EditorCreationController::Context context;
    context.generateUniqueScriptName = std::move(generateUniqueScriptName);
    context.generateUniqueObjectName = std::move(generateUniqueObjectName);
    return context;
}

EditorOutlinerController::SceneAccess buildOutlinerSceneAccess(ViewportWorkspaceWidget* viewport)
{
    EditorOutlinerController::SceneAccess sceneAccess;
    sceneAccess.rootObjectIds = [viewport]() {
        return viewport->rootObjectIds();
    };
    sceneAccess.findObject = [viewport](SceneObject::Id objectId) {
        return viewport->findObject(objectId);
    };
    return sceneAccess;
}

EditorRiggingController::Context buildRiggingContext(ViewportWorkspaceWidget* viewport)
{
    EditorRiggingController::Context context;
    context.findObject = [viewport](std::uint64_t objectId) {
        return viewport->findObject(objectId);
    };
    context.sceneSnapshot = [viewport]() {
        return viewport->sceneSnapshot();
    };
    context.resetJointOrientation = [viewport](std::uint64_t objectId) {
        return viewport->resetJointOrientation(objectId);
    };
    context.alignJointOrientationToChild = [viewport](std::uint64_t objectId) {
        return viewport->alignJointOrientationToChild(objectId);
    };
    context.captureBindPose = [viewport](std::uint64_t objectId, bool recursive) {
        return viewport->captureBindPose(objectId, recursive);
    };
    return context;
}

EditorSelectionController::Context buildSelectionContext(
    ViewportWorkspaceWidget* viewport,
    QTreeWidget* outlinerTree,
    QPlainTextEdit* scriptHistoryTextEdit,
    const EditorInspectorController::InspectorWidgets& inspectorWidgets,
    const EditorInspectorController::InspectorActions& inspectorActions,
    const EditorOutlinerController::SceneAccess& outlinerSceneAccess)
{
    EditorSelectionController::Context context;
    context.viewport = viewport;
    context.outlinerTree = outlinerTree;
    context.scriptHistoryTextEdit = scriptHistoryTextEdit;
    context.inspectorWidgets = inspectorWidgets;
    context.inspectorActions = inspectorActions;
    context.outlinerSceneAccess = outlinerSceneAccess;
    return context;
}

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
    QAction* localAxisAction)
{
    EditorViewportUiController::ActionSet actions;
    actions.translateAction = translateAction;
    actions.rotateAction = rotateAction;
    actions.scaleAction = scaleAction;
    actions.perspectiveCameraAction = perspectiveCameraAction;
    actions.frontCameraAction = frontCameraAction;
    actions.backCameraAction = backCameraAction;
    actions.leftCameraAction = leftCameraAction;
    actions.rightCameraAction = rightCameraAction;
    actions.topCameraAction = topCameraAction;
    actions.bottomCameraAction = bottomCameraAction;
    actions.worldAxisAction = worldAxisAction;
    actions.localAxisAction = localAxisAction;
    return actions;
}

EditorAnimationFlowController::Context buildAnimationFlowContext(ViewportWorkspaceWidget* viewport)
{
    EditorAnimationFlowController::Context context;
    context.findObject = [viewport](std::uint64_t objectId) {
        return viewport->findObject(objectId);
    };
    context.sceneSnapshot = [viewport]() {
        return viewport->sceneSnapshot();
    };
    return context;
}
}
