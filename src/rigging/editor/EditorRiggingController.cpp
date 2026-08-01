#include "rigging/editor/EditorRiggingController.h"

#include "EditorSceneQueryController.h"

namespace
{
using EditorSceneQueryController::objectDisplayName;

EditorRiggingController::OperationResult buildJointValidationFailure(
    const EditorRiggingController::Context& context,
    std::uint64_t selectedObjectId,
    const QString& emptySelectionMessage,
    const QString& invalidSelectionMessage)
{
    EditorRiggingController::OperationResult result;
    if (selectedObjectId == 0) {
        result.errorMessage = emptySelectionMessage;
        return result;
    }

    const SceneObject* object = context.findObject(selectedObjectId);
    if (object == nullptr || !object->isJoint()) {
        result.errorMessage = invalidSelectionMessage;
        return result;
    }

    result.success = true;
    result.focusObjectId = selectedObjectId;
    return result;
}
}

namespace EditorRiggingController
{
OperationResult markHierarchyParent(const Context& context, std::uint64_t selectedObjectId)
{
    OperationResult result;
    if (selectedObjectId == 0) {
        result.errorMessage = "Select an object to mark as parent";
        return result;
    }

    const SceneObject* object = context.findObject(selectedObjectId);
    if (object == nullptr) {
        result.errorMessage = "Selected object is no longer available";
        return result;
    }

    const QString objectName = objectDisplayName(*object);
    result.success = true;
    result.focusObjectId = selectedObjectId;
    result.markedHierarchyParentId = selectedObjectId;
    result.commentLine = QString("Marked hierarchy parent: %1").arg(objectName);
    result.statusMessage = QString("Marked parent: %1").arg(objectName);
    return result;
}

OperationResult reparentObject(const Context& context, std::uint64_t childId, std::uint64_t newParentId)
{
    OperationResult result;
    const SceneObject* childObject = context.findObject(childId);
    const SceneObject* parentObject = newParentId == 0 ? nullptr : context.findObject(newParentId);
    if (childObject == nullptr || (newParentId != 0 && parentObject == nullptr)) {
        result.errorMessage = "Hierarchy target is no longer available";
        return result;
    }

    if (childId == newParentId) {
        result.errorMessage = "Cannot parent an object to itself";
        return result;
    }

    const QString childName = objectDisplayName(*childObject);
    const QString parentName = parentObject == nullptr ? QString() : objectDisplayName(*parentObject);

    Scene updatedScene = context.sceneSnapshot();
    if (!updatedScene.reparentObject(childId, newParentId)) {
        result.errorMessage = newParentId == 0 ? "Unparent operation failed" : "Parent operation failed";
        return result;
    }

    result.success = true;
    result.hasScene = true;
    result.focusObjectId = childId;
    result.scene = std::move(updatedScene);
    if (newParentId == 0) {
        result.commandLine = QString("unparent %1;").arg(childName);
        result.resultLine = QString("// Result: unparented %1 //").arg(childName);
        result.statusMessage = QString("Unparented %1").arg(childName);
    } else {
        result.commandLine = QString("parent %1 %2;").arg(childName, parentName);
        result.resultLine = QString("// Result: parented %1 under %2 //").arg(childName, parentName);
        result.statusMessage = QString("Parented %1 under %2").arg(childName, parentName);
    }
    return result;
}

OperationResult bindSelectedMeshToMarkedJoint(
    const Context& context,
    std::uint64_t selectedObjectId,
    std::uint64_t markedHierarchyParentId)
{
    OperationResult result;
    if (selectedObjectId == 0 || markedHierarchyParentId == 0) {
        result.errorMessage = "Select a mesh and mark a joint first";
        return result;
    }

    const SceneObject* meshObject = context.findObject(selectedObjectId);
    const SceneObject* jointObject = context.findObject(markedHierarchyParentId);
    if (meshObject == nullptr || jointObject == nullptr) {
        result.errorMessage = "Bind target is no longer available";
        return result;
    }

    if (meshObject->meshHandles().isEmpty()) {
        result.errorMessage = "Selected object has no mesh to bind";
        return result;
    }

    if (!jointObject->isJoint()) {
        result.errorMessage = "Marked object is not a joint";
        return result;
    }

    Scene updatedScene = context.sceneSnapshot();
    if (!updatedScene.bindObjectToSkeleton(selectedObjectId, markedHierarchyParentId)) {
        result.errorMessage = "Bind skin failed";
        return result;
    }

    const QString meshName = objectDisplayName(*meshObject);
    const QString jointName = objectDisplayName(*jointObject);
    result.success = true;
    result.hasScene = true;
    result.focusObjectId = selectedObjectId;
    result.scene = std::move(updatedScene);
    result.commandLine = QString("bindSkin %1 %2;").arg(meshName, jointName);
    result.resultLine = QString("// Result: bound %1 to %2 //").arg(meshName, jointName);
    result.statusMessage = QString("Bound %1 to %2").arg(meshName, jointName);
    return result;
}

OperationResult resetSelectedJointOrientation(const Context& context, std::uint64_t selectedObjectId)
{
    OperationResult result = buildJointValidationFailure(
        context,
        selectedObjectId,
        "Select a joint to reset orientation",
        "Selected object is not a joint");
    if (!result.success) {
        return result;
    }

    const SceneObject* object = context.findObject(selectedObjectId);
    if (!context.resetJointOrientation(selectedObjectId)) {
        result.success = false;
        result.errorMessage = "Reset joint orientation failed";
        return result;
    }

    result.commandLine = QString("jointOrient %1 -reset;").arg(objectDisplayName(*object));
    result.resultLine = QString("// Result: reset joint orientation on %1 //").arg(objectDisplayName(*object));
    result.statusMessage = "Joint orientation reset";
    return result;
}

OperationResult alignSelectedJointOrientationToChild(const Context& context, std::uint64_t selectedObjectId)
{
    OperationResult result = buildJointValidationFailure(
        context,
        selectedObjectId,
        "Select a joint to align orientation",
        "Selected object is not a joint");
    if (!result.success) {
        return result;
    }

    const SceneObject* object = context.findObject(selectedObjectId);
    if (!context.alignJointOrientationToChild(selectedObjectId)) {
        result.success = false;
        result.errorMessage = "Align orientation needs a child joint offset";
        return result;
    }

    result.commandLine = QString("jointOrient %1 -alignToChild;").arg(objectDisplayName(*object));
    result.resultLine = QString("// Result: aligned joint orientation on %1 //").arg(objectDisplayName(*object));
    result.statusMessage = "Joint orientation aligned to child";
    return result;
}

OperationResult captureSelectedBindPose(const Context& context, std::uint64_t selectedObjectId, bool recursive)
{
    OperationResult result = buildJointValidationFailure(
        context,
        selectedObjectId,
        "Select a joint to capture bind pose",
        "Selected object is not a joint");
    if (!result.success) {
        return result;
    }

    const SceneObject* object = context.findObject(selectedObjectId);
    if (!context.captureBindPose(selectedObjectId, recursive)) {
        result.success = false;
        result.errorMessage = recursive ? "Recursive capture bind pose failed" : "Capture bind pose failed";
        return result;
    }

    if (recursive) {
        result.commandLine = QString("bindPose -capture -recursive %1;").arg(objectDisplayName(*object));
        result.resultLine = QString("// Result: captured bind pose on %1 recursively //").arg(objectDisplayName(*object));
        result.statusMessage = "Bind pose captured recursively";
    } else {
        result.commandLine = QString("bindPose -capture %1;").arg(objectDisplayName(*object));
        result.resultLine = QString("// Result: captured bind pose on %1 //").arg(objectDisplayName(*object));
        result.statusMessage = "Bind pose captured";
    }
    return result;
}
}
