#pragma once

#include <cstdint>

class QAction;
class QCheckBox;
class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QWidget;
class SceneObject;

namespace EditorInspectorController
{
struct InspectorWidgets
{
    QLabel* inspectorEmptyStateLabel = nullptr;
    QLabel* channelObjectNameLabel = nullptr;
    QDoubleSpinBox* translateXSpinBox = nullptr;
    QDoubleSpinBox* translateYSpinBox = nullptr;
    QDoubleSpinBox* translateZSpinBox = nullptr;
    QDoubleSpinBox* rotateXSpinBox = nullptr;
    QDoubleSpinBox* rotateYSpinBox = nullptr;
    QDoubleSpinBox* rotateZSpinBox = nullptr;
    QDoubleSpinBox* scaleXSpinBox = nullptr;
    QDoubleSpinBox* scaleYSpinBox = nullptr;
    QDoubleSpinBox* scaleZSpinBox = nullptr;
    QDoubleSpinBox* jointOrientXSpinBox = nullptr;
    QDoubleSpinBox* jointOrientYSpinBox = nullptr;
    QDoubleSpinBox* jointOrientZSpinBox = nullptr;
    QLabel* bindPoseStatusLabel = nullptr;
    QLabel* skinBindingStatusLabel = nullptr;
    QCheckBox* visibilityCheckBox = nullptr;
    QWidget* inspectorDetailsWidget = nullptr;
    QWidget* jointToolsWidget = nullptr;
};

struct InspectorActions
{
    QAction* markHierarchyParentAction = nullptr;
    QAction* parentToMarkedParentAction = nullptr;
    QAction* unparentSelectedAction = nullptr;
    QAction* bindSkinAction = nullptr;
    QAction* resetJointOrientationAction = nullptr;
    QAction* alignJointOrientationAction = nullptr;
    QAction* captureBindPoseAction = nullptr;
    QAction* captureBindPoseRecursiveAction = nullptr;
    QAction* frameSelectedAction = nullptr;
    QPushButton* frameSelectedButton = nullptr;
    QPushButton* resetJointOrientationButton = nullptr;
    QPushButton* alignJointOrientationButton = nullptr;
    QPushButton* captureBindPoseButton = nullptr;
    QPushButton* captureBindPoseRecursiveButton = nullptr;
};

void clearSelectionUi(const InspectorWidgets& widgets, bool hasScene);
void populateSelectionUi(const InspectorWidgets& widgets, const SceneObject& object);
void setInspectorEnabled(const InspectorWidgets& widgets, bool enabled);
void updateSelectionActions(
    const InspectorActions& actions,
    const SceneObject* selectedObject,
    const SceneObject* markedHierarchyParentObject,
    std::uint64_t markedHierarchyParentId);
}
