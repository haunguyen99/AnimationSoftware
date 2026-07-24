#pragma once

#include <cstdint>
#include <functional>

#include <QVector3D>

#include "scene/Scene.h"

namespace EditorChannelBoxController
{
struct TransformInput
{
    QVector3D translation;
    QVector3D rotationEulerDegrees;
    QVector3D scale;
};

struct Context
{
    std::function<const SceneObject*(std::uint64_t)> findObject;
    std::function<bool(std::uint64_t, const Transform&)> setObjectLocalTransform;
    std::function<bool(std::uint64_t, const QQuaternion&)> setJointOrientation;
    std::function<bool(std::uint64_t, bool)> setObjectVisibility;
};

struct OperationResult
{
    bool success = false;
    std::uint64_t objectId = 0;
    bool visibilityChanged = false;
    bool visible = true;
    QString errorMessage;
    QString statusMessage;
    QString commandLine;
    QString resultLine;
    Transform transform;
    QVector3D jointOrientationEulerDegrees;
};

OperationResult applyTransform(const Context& context, std::uint64_t selectedObjectId, const TransformInput& input);
OperationResult applyJointOrientation(const Context& context, std::uint64_t selectedObjectId, const QVector3D& eulerDegrees);
OperationResult applyVisibility(const Context& context, std::uint64_t selectedObjectId, bool visible);
}
