#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector3D>

#include <cstdlib>
#include <iostream>

#include "scene/Bounds3D.h"
#include "scene/Scene.h"

namespace
{
bool fuzzyCompare(float a, float b, float epsilon = 0.001f)
{
    return std::abs(a - b) <= epsilon;
}

bool fuzzyCompare(const QVector3D& a, const QVector3D& b, float epsilon = 0.001f)
{
    return fuzzyCompare(a.x(), b.x(), epsilon)
        && fuzzyCompare(a.y(), b.y(), epsilon)
        && fuzzyCompare(a.z(), b.z(), epsilon);
}

int fail(const char* message)
{
    std::cerr << message << '\n';
    return EXIT_FAILURE;
}
}

int main()
{
    Scene scene;
    const SceneObject::Id parentId = scene.createObject("Parent");
    const SceneObject::Id childId = scene.createObject("Child");

    SceneObject* parent = scene.findObject(parentId);
    SceneObject* child = scene.findObject(childId);
    if (parent == nullptr || child == nullptr) {
        return fail("scene object create failed");
    }

    parent->addChildId(childId);
    child->setParentId(parentId);

    Transform parentTransform;
    parentTransform.translation = QVector3D(10.0f, 0.0f, 0.0f);
    parent->setAuthoredTransform(parentTransform);
    parent->setLocalTransform(parentTransform);
    parent->setLocalBounds(Bounds3D::fromMinMax(QVector3D(-1.0f, -1.0f, -1.0f), QVector3D(1.0f, 1.0f, 1.0f)));

    Transform childTransform;
    childTransform.translation = QVector3D(0.0f, 2.0f, 0.0f);
    child->setAuthoredTransform(childTransform);
    child->setLocalTransform(childTransform);
    child->setLocalBounds(Bounds3D::fromMinMax(QVector3D(-0.5f, -0.5f, -0.5f), QVector3D(0.5f, 0.5f, 0.5f)));

    scene.rebuildWorldData();

    if (!fuzzyCompare(parent->worldBounds().center(), QVector3D(10.0f, 0.0f, 0.0f))) {
        return fail("parent world bounds wrong");
    }

    if (!fuzzyCompare(child->worldBounds().center(), QVector3D(10.0f, 2.0f, 0.0f))) {
        return fail("child world bounds wrong");
    }

    Transform movedChild = child->localTransform();
    movedChild.translation = QVector3D(3.0f, 2.0f, 0.0f);
    if (!scene.setLocalTransform(childId, movedChild)) {
        return fail("setLocalTransform failed");
    }

    child = scene.findObject(childId);
    if (child == nullptr) {
        return fail("child missing after transform");
    }

    if (!fuzzyCompare(child->worldBounds().center(), QVector3D(13.0f, 2.0f, 0.0f))) {
        return fail("child world bounds not rebuilt after local transform change");
    }

    const QMatrix4x4 childWorld = scene.worldTransform(childId);
    if (!fuzzyCompare(childWorld * QVector3D(0.0f, 0.0f, 0.0f), QVector3D(13.0f, 2.0f, 0.0f))) {
        return fail("child world matrix wrong");
    }

    Scene animationScene;
    const SceneObject::Id animatedId = animationScene.createObject("Animated");
    if (!animationScene.setLocalTransform(animatedId, Transform())) {
        return fail("initial animated transform failed");
    }

    if (!animationScene.setObjectKeyframe(animatedId, 0)) {
        return fail("set keyframe at frame 0 failed");
    }

    animationScene.setCurrentFrame(10);
    Transform endTransform;
    endTransform.translation = QVector3D(10.0f, 4.0f, -2.0f);
    endTransform.scale = QVector3D(2.0f, 3.0f, 4.0f);
    if (!animationScene.setLocalTransform(animatedId, endTransform)) {
        return fail("set animated end transform failed");
    }

    if (!animationScene.setObjectKeyframe(animatedId, 10)) {
        return fail("set keyframe at frame 10 failed");
    }

    animationScene.setCurrentFrame(5);
    const SceneObject* animatedObject = animationScene.findObject(animatedId);
    if (animatedObject == nullptr) {
        return fail("animated object missing");
    }

    if (!fuzzyCompare(animatedObject->localTransform().translation, QVector3D(5.0f, 2.0f, -1.0f))) {
        return fail("animated translation interpolation wrong");
    }

    if (!fuzzyCompare(animatedObject->localTransform().scale, QVector3D(1.5f, 2.0f, 2.5f))) {
        return fail("animated scale interpolation wrong");
    }

    Scene autoKeyScene;
    const SceneObject::Id autoKeyId = autoKeyScene.createObject("AutoKeyed");
    autoKeyScene.setCurrentFrame(12);

    Transform autoKeyTransform;
    autoKeyTransform.translation = QVector3D(8.0f, 1.0f, 0.0f);
    if (!autoKeyScene.setLocalTransform(autoKeyId, autoKeyTransform, true)) {
        return fail("auto key transform failed");
    }

    const SceneObject* autoKeyObject = autoKeyScene.findObject(autoKeyId);
    if (autoKeyObject == nullptr) {
        return fail("auto keyed object missing");
    }

    if (autoKeyObject->transformKeyframes().size() != 2) {
        return fail("auto key should seed two keyframes");
    }

    if (autoKeyObject->transformKeyframes().first().frame != 0 || autoKeyObject->transformKeyframes().last().frame != 12) {
        return fail("auto key seeded wrong frames");
    }

    autoKeyScene.setCurrentFrame(6);
    autoKeyObject = autoKeyScene.findObject(autoKeyId);
    if (autoKeyObject == nullptr || !fuzzyCompare(autoKeyObject->localTransform().translation, QVector3D(4.0f, 0.5f, 0.0f))) {
        return fail("auto key interpolation wrong");
    }

    if (!autoKeyScene.removeObjectKeyframe(autoKeyId, 12)) {
        return fail("remove keyframe failed");
    }

    autoKeyScene.setCurrentFrame(12);
    autoKeyObject = autoKeyScene.findObject(autoKeyId);
    if (autoKeyObject == nullptr) {
        return fail("auto keyed object missing after delete");
    }

    if (autoKeyObject->hasTransformKeyframe(12)) {
        return fail("deleted keyframe still present");
    }

    if (!fuzzyCompare(autoKeyObject->localTransform().translation, QVector3D())) {
        return fail("deleted key should fall back to remaining animation value");
    }

    return EXIT_SUCCESS;
}
