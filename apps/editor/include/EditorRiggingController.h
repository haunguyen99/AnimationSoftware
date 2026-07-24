#pragma once

#include <cstdint>
#include <functional>

#include <QString>

#include "scene/Scene.h"

namespace EditorRiggingController
{
struct Context
{
    std::function<const SceneObject*(std::uint64_t)> findObject;
    std::function<Scene()> sceneSnapshot;
    std::function<bool(std::uint64_t)> resetJointOrientation;
    std::function<bool(std::uint64_t)> alignJointOrientationToChild;
    std::function<bool(std::uint64_t, bool)> captureBindPose;
};

struct OperationResult
{
    bool success = false;
    bool hasScene = false;
    std::uint64_t focusObjectId = 0;
    std::uint64_t markedHierarchyParentId = 0;
    Scene scene;
    QString errorMessage;
    QString statusMessage;
    QString commentLine;
    QString commandLine;
    QString resultLine;
};

OperationResult markHierarchyParent(const Context& context, std::uint64_t selectedObjectId);
OperationResult reparentObject(const Context& context, std::uint64_t childId, std::uint64_t newParentId);
OperationResult bindSelectedMeshToMarkedJoint(
    const Context& context,
    std::uint64_t selectedObjectId,
    std::uint64_t markedHierarchyParentId);
OperationResult resetSelectedJointOrientation(const Context& context, std::uint64_t selectedObjectId);
OperationResult alignSelectedJointOrientationToChild(const Context& context, std::uint64_t selectedObjectId);
OperationResult captureSelectedBindPose(const Context& context, std::uint64_t selectedObjectId, bool recursive);
}
