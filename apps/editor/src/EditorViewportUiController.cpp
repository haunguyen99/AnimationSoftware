#include "EditorViewportUiController.h"

#include <QAction>

namespace EditorViewportUiController
{
void applyTransformModeChecks(const ActionSet& actions, TransformMode mode)
{
    if (actions.translateAction != nullptr) {
        actions.translateAction->setChecked(mode == TransformMode::Translate);
    }
    if (actions.rotateAction != nullptr) {
        actions.rotateAction->setChecked(mode == TransformMode::Rotate);
    }
    if (actions.scaleAction != nullptr) {
        actions.scaleAction->setChecked(mode == TransformMode::Scale);
    }
}

OperationResult buildTransformModeResult(TransformMode mode)
{
    OperationResult result;
    result.success = true;
    switch (mode) {
    case TransformMode::Translate:
        result.commandLine = "setToolTo MoveSuperContext;";
        result.resultLine = "// Result: move tool //";
        result.statusMessage = "Transform mode: Translate";
        break;
    case TransformMode::Rotate:
        result.commandLine = "setToolTo RotateSuperContext;";
        result.resultLine = "// Result: rotate tool //";
        result.statusMessage = "Transform mode: Rotate";
        break;
    case TransformMode::Scale:
        result.commandLine = "setToolTo ScaleSuperContext;";
        result.resultLine = "// Result: scale tool //";
        result.statusMessage = "Transform mode: Scale";
        break;
    }
    return result;
}

void applyCameraPresetChecks(const ActionSet& actions, CameraPreset preset)
{
    if (actions.perspectiveCameraAction != nullptr) {
        actions.perspectiveCameraAction->setChecked(preset == CameraPreset::Perspective);
    }
    if (actions.frontCameraAction != nullptr) {
        actions.frontCameraAction->setChecked(preset == CameraPreset::Front);
    }
    if (actions.backCameraAction != nullptr) {
        actions.backCameraAction->setChecked(preset == CameraPreset::Back);
    }
    if (actions.leftCameraAction != nullptr) {
        actions.leftCameraAction->setChecked(preset == CameraPreset::Left);
    }
    if (actions.rightCameraAction != nullptr) {
        actions.rightCameraAction->setChecked(preset == CameraPreset::Right);
    }
    if (actions.topCameraAction != nullptr) {
        actions.topCameraAction->setChecked(preset == CameraPreset::Top);
    }
    if (actions.bottomCameraAction != nullptr) {
        actions.bottomCameraAction->setChecked(preset == CameraPreset::Bottom);
    }
}

OperationResult buildCameraPresetResult(CameraPreset preset)
{
    OperationResult result;
    result.success = true;
    switch (preset) {
    case CameraPreset::Front:
        result.commentLine = "Camera preset: Front";
        result.statusMessage = "Camera view: Front";
        break;
    case CameraPreset::Back:
        result.commentLine = "Camera preset: Back";
        result.statusMessage = "Camera view: Back";
        break;
    case CameraPreset::Left:
        result.commentLine = "Camera preset: Left";
        result.statusMessage = "Camera view: Left";
        break;
    case CameraPreset::Right:
        result.commentLine = "Camera preset: Right";
        result.statusMessage = "Camera view: Right";
        break;
    case CameraPreset::Top:
        result.commentLine = "Camera preset: Top";
        result.statusMessage = "Camera view: Top";
        break;
    case CameraPreset::Bottom:
        result.commentLine = "Camera preset: Bottom";
        result.statusMessage = "Camera view: Bottom";
        break;
    case CameraPreset::Perspective:
        result.commentLine = "Camera preset: Perspective";
        result.statusMessage = "Camera view: Perspective";
        break;
    }
    return result;
}

void applyAxisOrientationChecks(const ActionSet& actions, AxisOrientation orientation)
{
    if (actions.worldAxisAction != nullptr) {
        actions.worldAxisAction->setChecked(orientation == AxisOrientation::World);
    }
    if (actions.localAxisAction != nullptr) {
        actions.localAxisAction->setChecked(orientation == AxisOrientation::Local);
    }
}

OperationResult buildAxisOrientationResult(AxisOrientation orientation)
{
    OperationResult result;
    result.success = true;
    if (orientation == AxisOrientation::World) {
        result.commentLine = "Axis orientation set to World";
        result.statusMessage = "Axis orientation: World";
    } else {
        result.commentLine = "Axis orientation set to Local";
        result.statusMessage = "Axis orientation: Local";
    }
    return result;
}

OperationResult buildWorkspaceLayoutResult()
{
    OperationResult result;
    result.success = true;
    result.commentLine = "Workspace layout restored";
    result.statusMessage = "Workspace layout restored";
    return result;
}

OperationResult buildPolygonPrimitivesWindowResult()
{
    OperationResult result;
    result.success = true;
    result.commentLine = "Polygon Primitives window opened";
    return result;
}
}
