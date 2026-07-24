#include "EditorInspectorController.h"

#include <QAction>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include "EditorSceneQueryController.h"
#include "scene/SceneObject.h"

namespace
{
using EditorSceneQueryController::objectDisplayName;

QString formatVector3(const QVector3D& value)
{
    return QString("(%1, %2, %3)")
        .arg(value.x(), 0, 'f', 2)
        .arg(value.y(), 0, 'f', 2)
        .arg(value.z(), 0, 'f', 2);
}

QString formatTransform(const Transform& transform)
{
    return QString("T %1 | R (%2, %3, %4, %5) | S %6")
        .arg(formatVector3(transform.translation))
        .arg(transform.rotation.scalar(), 0, 'f', 2)
        .arg(transform.rotation.x(), 0, 'f', 2)
        .arg(transform.rotation.y(), 0, 'f', 2)
        .arg(transform.rotation.z(), 0, 'f', 2)
        .arg(formatVector3(transform.scale));
}

QString formatBindPoseStatus(const SceneObject& object)
{
    if (!object.isJoint()) {
        return "Bind pose: n/a";
    }

    return object.hasBindPose()
        ? QString("Bind pose captured: %1").arg(formatTransform(object.bindPoseLocalTransform()))
        : "Bind pose: not captured";
}

QString formatSkinBindingStatus(const SceneObject& object)
{
    if (object.meshHandles().isEmpty()) {
        return "Skin binding: n/a";
    }

    return object.hasSkinBinding()
        ? QString("Skin binding: %1 joints, %2 weighted vertices")
              .arg(object.skinJointIds().size())
              .arg(object.skinWeights().size())
        : "Skin binding: not bound";
}

void setActionEnabled(QAction* action, bool enabled)
{
    if (action != nullptr) {
        action->setEnabled(enabled);
    }
}

void setButtonEnabled(QPushButton* button, bool enabled)
{
    if (button != nullptr) {
        button->setEnabled(enabled);
    }
}
}

namespace EditorInspectorController
{
void clearSelectionUi(const InspectorWidgets& widgets, bool hasScene)
{
    if (widgets.inspectorEmptyStateLabel != nullptr) {
        widgets.inspectorEmptyStateLabel->setText(
            hasScene ? "No selection." : "No selection. Import an FBX to inspect the scene.");
    }

    if (widgets.channelObjectNameLabel != nullptr) {
        widgets.channelObjectNameLabel->setText("-");
    }

    for (QDoubleSpinBox* spinBox : { widgets.translateXSpinBox, widgets.translateYSpinBox, widgets.translateZSpinBox,
             widgets.rotateXSpinBox, widgets.rotateYSpinBox, widgets.rotateZSpinBox,
             widgets.jointOrientXSpinBox, widgets.jointOrientYSpinBox, widgets.jointOrientZSpinBox }) {
        if (spinBox != nullptr) {
            spinBox->setValue(0.0);
        }
    }

    for (QDoubleSpinBox* spinBox : { widgets.scaleXSpinBox, widgets.scaleYSpinBox, widgets.scaleZSpinBox }) {
        if (spinBox != nullptr) {
            spinBox->setValue(1.0);
        }
    }

    if (widgets.bindPoseStatusLabel != nullptr) {
        widgets.bindPoseStatusLabel->setText("Bind pose: n/a");
    }
    if (widgets.skinBindingStatusLabel != nullptr) {
        widgets.skinBindingStatusLabel->setText("Skin binding: n/a");
    }
    if (widgets.visibilityCheckBox != nullptr) {
        widgets.visibilityCheckBox->setChecked(true);
        widgets.visibilityCheckBox->setText("on");
    }

    setInspectorEnabled(widgets, false);
}

void populateSelectionUi(const InspectorWidgets& widgets, const SceneObject& object)
{
    const Transform& transform = object.localTransform();
    const QVector3D eulerDegrees = transform.rotation.toEulerAngles();
    const QVector3D jointOrientEuler = object.jointOrientation().toEulerAngles();
    const bool isJoint = object.isJoint();

    if (widgets.inspectorEmptyStateLabel != nullptr) {
        widgets.inspectorEmptyStateLabel->setText("Selected object channel box.");
    }
    if (widgets.channelObjectNameLabel != nullptr) {
        widgets.channelObjectNameLabel->setText(objectDisplayName(object));
    }
    if (widgets.translateXSpinBox != nullptr) {
        widgets.translateXSpinBox->setValue(transform.translation.x());
    }
    if (widgets.translateYSpinBox != nullptr) {
        widgets.translateYSpinBox->setValue(transform.translation.y());
    }
    if (widgets.translateZSpinBox != nullptr) {
        widgets.translateZSpinBox->setValue(transform.translation.z());
    }
    if (widgets.rotateXSpinBox != nullptr) {
        widgets.rotateXSpinBox->setValue(eulerDegrees.x());
    }
    if (widgets.rotateYSpinBox != nullptr) {
        widgets.rotateYSpinBox->setValue(eulerDegrees.y());
    }
    if (widgets.rotateZSpinBox != nullptr) {
        widgets.rotateZSpinBox->setValue(eulerDegrees.z());
    }
    if (widgets.scaleXSpinBox != nullptr) {
        widgets.scaleXSpinBox->setValue(transform.scale.x());
    }
    if (widgets.scaleYSpinBox != nullptr) {
        widgets.scaleYSpinBox->setValue(transform.scale.y());
    }
    if (widgets.scaleZSpinBox != nullptr) {
        widgets.scaleZSpinBox->setValue(transform.scale.z());
    }
    if (widgets.jointOrientXSpinBox != nullptr) {
        widgets.jointOrientXSpinBox->setValue(isJoint ? jointOrientEuler.x() : 0.0);
    }
    if (widgets.jointOrientYSpinBox != nullptr) {
        widgets.jointOrientYSpinBox->setValue(isJoint ? jointOrientEuler.y() : 0.0);
    }
    if (widgets.jointOrientZSpinBox != nullptr) {
        widgets.jointOrientZSpinBox->setValue(isJoint ? jointOrientEuler.z() : 0.0);
    }
    if (widgets.bindPoseStatusLabel != nullptr) {
        widgets.bindPoseStatusLabel->setText(formatBindPoseStatus(object));
    }
    if (widgets.skinBindingStatusLabel != nullptr) {
        widgets.skinBindingStatusLabel->setText(formatSkinBindingStatus(object));
    }
    if (widgets.visibilityCheckBox != nullptr) {
        widgets.visibilityCheckBox->setChecked(object.isVisible());
        widgets.visibilityCheckBox->setText(object.isVisible() ? "on" : "off");
    }
    if (widgets.jointToolsWidget != nullptr) {
        widgets.jointToolsWidget->setEnabled(isJoint);
    }

    setInspectorEnabled(widgets, true);
}

void setInspectorEnabled(const InspectorWidgets& widgets, bool enabled)
{
    if (widgets.inspectorDetailsWidget != nullptr) {
        widgets.inspectorDetailsWidget->setEnabled(enabled);
    }
}

void updateSelectionActions(
    const InspectorActions& actions,
    const SceneObject* selectedObject,
    const SceneObject* markedHierarchyParentObject,
    std::uint64_t markedHierarchyParentId)
{
    if (selectedObject == nullptr) {
        setActionEnabled(actions.markHierarchyParentAction, false);
        setActionEnabled(actions.parentToMarkedParentAction, false);
        setActionEnabled(actions.unparentSelectedAction, false);
        setActionEnabled(actions.bindSkinAction, false);
        setActionEnabled(actions.resetJointOrientationAction, false);
        setActionEnabled(actions.alignJointOrientationAction, false);
        setActionEnabled(actions.captureBindPoseAction, false);
        setActionEnabled(actions.captureBindPoseRecursiveAction, false);
        setActionEnabled(actions.frameSelectedAction, false);
        setButtonEnabled(actions.frameSelectedButton, false);
        setButtonEnabled(actions.resetJointOrientationButton, false);
        setButtonEnabled(actions.alignJointOrientationButton, false);
        setButtonEnabled(actions.captureBindPoseButton, false);
        setButtonEnabled(actions.captureBindPoseRecursiveButton, false);
        return;
    }

    const bool canFrame = selectedObject->isVisible() && selectedObject->worldBounds().isValid();
    const bool isJoint = selectedObject->isJoint();
    const bool hasChildJointTarget = isJoint && !selectedObject->childIds().isEmpty();
    const bool canBindSkin = !selectedObject->meshHandles().isEmpty()
        && markedHierarchyParentObject != nullptr
        && markedHierarchyParentObject->isJoint()
        && markedHierarchyParentId != selectedObject->id();

    setActionEnabled(actions.markHierarchyParentAction, true);
    setActionEnabled(actions.parentToMarkedParentAction, markedHierarchyParentId != 0 && markedHierarchyParentId != selectedObject->id());
    setActionEnabled(actions.unparentSelectedAction, selectedObject->parentId() != 0);
    setActionEnabled(actions.bindSkinAction, canBindSkin);
    setActionEnabled(actions.resetJointOrientationAction, isJoint);
    setActionEnabled(actions.alignJointOrientationAction, hasChildJointTarget);
    setActionEnabled(actions.captureBindPoseAction, isJoint);
    setActionEnabled(actions.captureBindPoseRecursiveAction, isJoint);
    setActionEnabled(actions.frameSelectedAction, canFrame);
    setButtonEnabled(actions.frameSelectedButton, canFrame);
    setButtonEnabled(actions.resetJointOrientationButton, isJoint);
    setButtonEnabled(actions.alignJointOrientationButton, hasChildJointTarget);
    setButtonEnabled(actions.captureBindPoseButton, isJoint);
    setButtonEnabled(actions.captureBindPoseRecursiveButton, isJoint);
}
}
