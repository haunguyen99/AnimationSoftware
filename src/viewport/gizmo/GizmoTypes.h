#pragma once

enum class GizmoHandle
{
    None = -1,
    X = 0,
    Y = 1,
    Z = 2,
    XY = 3,
    YZ = 4,
    XZ = 5,
    ViewPlane = 6
};

inline bool isAxisGizmoHandle(GizmoHandle handle)
{
    return handle == GizmoHandle::X
        || handle == GizmoHandle::Y
        || handle == GizmoHandle::Z;
}

inline bool isPlaneGizmoHandle(GizmoHandle handle)
{
    return handle == GizmoHandle::XY
        || handle == GizmoHandle::YZ
        || handle == GizmoHandle::XZ;
}

inline bool isScreenGizmoHandle(GizmoHandle handle)
{
    return handle == GizmoHandle::ViewPlane;
}
