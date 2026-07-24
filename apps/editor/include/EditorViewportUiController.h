#pragma once

#include <functional>

#include <QString>

class QAction;
class QDockWidget;

namespace EditorViewportUiController
{
enum class TransformMode
{
    Translate,
    Rotate,
    Scale
};

enum class CameraPreset
{
    Perspective,
    Front,
    Back,
    Left,
    Right,
    Top,
    Bottom
};

enum class AxisOrientation
{
    World,
    Local
};

struct ActionSet
{
    QAction* translateAction = nullptr;
    QAction* rotateAction = nullptr;
    QAction* scaleAction = nullptr;
    QAction* perspectiveCameraAction = nullptr;
    QAction* frontCameraAction = nullptr;
    QAction* backCameraAction = nullptr;
    QAction* leftCameraAction = nullptr;
    QAction* rightCameraAction = nullptr;
    QAction* topCameraAction = nullptr;
    QAction* bottomCameraAction = nullptr;
    QAction* worldAxisAction = nullptr;
    QAction* localAxisAction = nullptr;
};

struct WorkspaceSet
{
    QDockWidget* outlinerDock = nullptr;
    QDockWidget* inspectorDock = nullptr;
    QDockWidget* timeSliderDock = nullptr;
    QDockWidget* polygonPrimitivesDock = nullptr;
};

struct OperationResult
{
    bool success = false;
    QString statusMessage;
    QString commentLine;
    QString commandLine;
    QString resultLine;
};

void applyTransformModeChecks(const ActionSet& actions, TransformMode mode);
OperationResult buildTransformModeResult(TransformMode mode);
void applyCameraPresetChecks(const ActionSet& actions, CameraPreset preset);
OperationResult buildCameraPresetResult(CameraPreset preset);
void applyAxisOrientationChecks(const ActionSet& actions, AxisOrientation orientation);
OperationResult buildAxisOrientationResult(AxisOrientation orientation);
OperationResult buildWorkspaceLayoutResult();
OperationResult buildPolygonPrimitivesWindowResult();
}
