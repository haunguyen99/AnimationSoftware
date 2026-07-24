#include "EditorChannelBoxController.h"

#include "EditorSceneQueryController.h"

namespace
{
using EditorSceneQueryController::objectDisplayName;
}

namespace EditorChannelBoxController
{
OperationResult applyTransform(const Context& context, std::uint64_t selectedObjectId, const TransformInput& input)
{
    OperationResult result;
    if (selectedObjectId == 0) {
        return result;
    }

    const SceneObject* object = context.findObject(selectedObjectId);
    if (object == nullptr) {
        return result;
    }

    Transform transform = object->localTransform();
    transform.translation = input.translation;
    transform.rotation = QQuaternion::fromEulerAngles(input.rotationEulerDegrees);
    transform.scale = input.scale;

    if (!context.setObjectLocalTransform(selectedObjectId, transform)) {
        return result;
    }

    result.success = true;
    result.objectId = selectedObjectId;
    result.transform = transform;
    result.statusMessage = "Channel Box updated";
    return result;
}

OperationResult applyJointOrientation(const Context& context, std::uint64_t selectedObjectId, const QVector3D& eulerDegrees)
{
    OperationResult result;
    if (selectedObjectId == 0) {
        return result;
    }

    const SceneObject* object = context.findObject(selectedObjectId);
    if (object == nullptr || !object->isJoint()) {
        return result;
    }

    if (!context.setJointOrientation(selectedObjectId, QQuaternion::fromEulerAngles(eulerDegrees))) {
        result.errorMessage = "Joint orientation update failed";
        return result;
    }

    result.success = true;
    result.objectId = selectedObjectId;
    result.jointOrientationEulerDegrees = eulerDegrees;
    result.commandLine = QString("jointOrient %1 -euler %2 %3 %4;")
            .arg(objectDisplayName(*object))
            .arg(eulerDegrees.x(), 0, 'f', 3)
            .arg(eulerDegrees.y(), 0, 'f', 3)
            .arg(eulerDegrees.z(), 0, 'f', 3);
    result.resultLine = QString("// Result: joint orientation updated on %1 //").arg(objectDisplayName(*object));
    result.statusMessage = "Joint orientation updated";
    return result;
}

OperationResult applyVisibility(const Context& context, std::uint64_t selectedObjectId, bool visible)
{
    OperationResult result;
    if (selectedObjectId == 0) {
        return result;
    }

    const SceneObject* object = context.findObject(selectedObjectId);
    if (object == nullptr) {
        return result;
    }

    if (!context.setObjectVisibility(selectedObjectId, visible)) {
        return result;
    }

    result.success = true;
    result.objectId = selectedObjectId;
    result.visibilityChanged = true;
    result.visible = visible;
    result.statusMessage = visible ? "Visibility on" : "Visibility off";
    return result;
}
}
