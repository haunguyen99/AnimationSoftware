#include "rigging/scene/JointRiggingOps.h"

#include <algorithm>

#include "scene/Scene.h"
#include "scene/SceneMath.h"

namespace JointRiggingOps
{
SceneObject::Id createJoint(Scene& scene, const QString& name, SceneObject::Id parentId)
{
    if (parentId != 0 && !scene.contains(parentId)) {
        return 0;
    }

    SceneObject::Id nextId = 1;
    const QVector<SceneObject::Id> existingIds = scene.allObjectIds();
    if (!existingIds.isEmpty()) {
        nextId = *std::max_element(existingIds.cbegin(), existingIds.cend()) + 1;
    }

    const QString jointName = name.isEmpty() ? QString("joint_%1").arg(nextId) : name;
    const SceneObject::Id id = scene.createObject(jointName, SceneObject::Kind::Joint);
    if (parentId != 0 && !scene.reparentObject(id, parentId)) {
        scene.removeObject(id);
        return 0;
    }

    SceneObject* object = scene.findObject(id);
    if (object != nullptr) {
        object->setHasBindPose(true);
        object->setBindPoseLocalTransform(object->authoredTransform());
    }

    scene.rebuildWorldData();
    return id;
}

bool setJointOrientation(Scene& scene, SceneObject::Id id, const QQuaternion& orientation)
{
    SceneObject* object = scene.findObject(id);
    if (object == nullptr || !object->isJoint()) {
        return false;
    }

    object->setJointOrientation(orientation);
    scene.rebuildWorldData();
    return true;
}

bool resetJointOrientation(Scene& scene, SceneObject::Id id)
{
    return setJointOrientation(scene, id, QQuaternion());
}

bool alignJointOrientationToChild(Scene& scene, SceneObject::Id id)
{
    SceneObject* object = scene.findObject(id);
    if (object == nullptr || !object->isJoint() || object->childIds().isEmpty()) {
        return false;
    }

    const SceneObject* child = scene.findObject(object->childIds().first());
    if (child == nullptr) {
        return false;
    }

    const QVector3D aimVector = child->localTransform().translation.normalized();
    if (aimVector.lengthSquared() < 0.0001f) {
        object->setJointOrientation(QQuaternion());
    } else {
        object->setJointOrientation(QQuaternion::rotationTo(QVector3D(1.0f, 0.0f, 0.0f), aimVector));
    }

    scene.rebuildWorldData();
    return true;
}

bool captureBindPose(Scene& scene, SceneObject::Id id, bool recursive)
{
    SceneObject* object = scene.findObject(id);
    if (object == nullptr || !object->isJoint()) {
        return false;
    }

    object->setBindPoseLocalTransform(object->localTransform());
    object->setHasBindPose(true);

    if (recursive) {
        for (SceneObject::Id childId : object->childIds()) {
            const SceneObject* child = scene.findObject(childId);
            if (child != nullptr && child->isJoint()) {
                captureBindPose(scene, childId, true);
            }
        }
    }

    return true;
}

QMatrix4x4 bindPoseWorldTransform(const Scene& scene, SceneObject::Id id)
{
    const SceneObject* object = scene.findObject(id);
    if (object == nullptr) {
        return QMatrix4x4();
    }

    QVector<SceneObject::Id> chain;
    chain.reserve(16);

    const SceneObject* current = object;
    while (current != nullptr) {
        chain.append(current->id());
        current = current->parentId() == 0 ? nullptr : scene.findObject(current->parentId());
    }

    QMatrix4x4 worldMatrix;
    for (auto it = chain.crbegin(); it != chain.crend(); ++it) {
        const SceneObject* chainObject = scene.findObject(*it);
        if (chainObject == nullptr) {
            continue;
        }

        Transform bindTransform = chainObject->localTransform();
        if (chainObject->isJoint() && chainObject->hasBindPose()) {
            bindTransform = chainObject->bindPoseLocalTransform();
            bindTransform.rotation = chainObject->jointOrientation() * bindTransform.rotation;
        } else if (chainObject->hasSkinBinding()) {
            bindTransform = chainObject->skinBindLocalTransform();
        } else {
            bindTransform = chainObject->localTransform();
            if (chainObject->isJoint()) {
                bindTransform.rotation = chainObject->jointOrientation() * bindTransform.rotation;
            }
        }

        worldMatrix *= SceneMath::composeMatrix(bindTransform);
    }

    return worldMatrix;
}
}
