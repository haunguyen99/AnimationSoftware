#pragma once

#include <QHash>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QString>
#include <QVector>

#include "animation/scene/SceneAnimationState.h"
#include "scene/Bounds3D.h"
#include "scene/MeshData.h"
#include "scene/SceneObject.h"

class Scene
{
public:
    Scene();

    SceneObject::Id createObject(const QString& name = {}, SceneObject::Kind kind = SceneObject::Kind::Transform);
    SceneObject::Id createJoint(const QString& name = {}, SceneObject::Id parentId = 0);
    bool contains(SceneObject::Id id) const;

    SceneObject* findObject(SceneObject::Id id);
    const SceneObject* findObject(SceneObject::Id id) const;

    QVector<SceneObject::Id> rootObjectIds() const;
    QVector<SceneObject::Id> allObjectIds() const;
    int addMesh(const MeshData& meshData);
    const MeshData* findMesh(int meshHandle) const;
    QVector<int> allMeshHandles() const;

    void clear();
    bool isEmpty() const;
    void appendScene(const Scene& other);
    bool setObjectName(SceneObject::Id id, const QString& name);
    int currentFrame() const;
    void setCurrentFrame(int frame);
    bool setLocalTransform(SceneObject::Id id, const Transform& transform, bool autoKeyEnabled = false);
    bool setLocalTransformInteractive(SceneObject::Id id, const Transform& transform, bool autoKeyEnabled = false);
    bool setObjectKeyframe(SceneObject::Id id, int frame);
    bool removeObjectKeyframe(SceneObject::Id id, int frame);
    bool removeObjectKeyframesInRange(SceneObject::Id id, int startFrame, int endFrame);
    bool duplicateObjectKeyframe(SceneObject::Id id, int sourceFrame, int targetFrame);
    bool offsetObjectKeyframes(SceneObject::Id id, int frameDelta);
    bool offsetObjectKeyframesInRange(SceneObject::Id id, int startFrame, int endFrame, int frameDelta);
    bool scaleAllObjectKeyframes(double scaleFactor);
    int nextObjectKeyframe(SceneObject::Id id, int frame) const;
    int previousObjectKeyframe(SceneObject::Id id, int frame) const;
    bool setObjectVisible(SceneObject::Id id, bool visible);
    bool setJointOrientation(SceneObject::Id id, const QQuaternion& orientation);
    bool resetJointOrientation(SceneObject::Id id);
    bool alignJointOrientationToChild(SceneObject::Id id);
    bool captureBindPose(SceneObject::Id id, bool recursive = false);
    bool bindObjectToSkeleton(SceneObject::Id objectId, SceneObject::Id rootJointId);
    bool setObjectSkinBinding(SceneObject::Id id, const QVector<SceneObject::Id>& jointIds, const SkinWeightTable& weights);
    bool clearObjectSkinBinding(SceneObject::Id id);
    bool buildDeformedMesh(SceneObject::Id objectId, int meshHandle, MeshData* deformedMesh) const;
    bool reparentObject(SceneObject::Id id, SceneObject::Id newParentId, bool keepWorldTransform = true);
    bool removeObject(SceneObject::Id id);
    SceneObject::Id duplicateSubtree(SceneObject::Id id, SceneObject::Id newParentId = 0);
    void optimizeStorage();
    QMatrix4x4 worldTransform(SceneObject::Id id) const;
    QMatrix4x4 bindPoseWorldTransform(SceneObject::Id id) const;

    void rebuildSceneBounds();
    void rebuildWorldData();
    const Bounds3D& sceneBounds() const;

    QString debugDump() const;

private:
    Transform composeObjectLocalTransform(const SceneObject& object, const Transform& baseTransform) const;
    bool isDescendantOf(SceneObject::Id id, SceneObject::Id potentialAncestorId) const;
    SceneObject::Id duplicateSubtreeRecursive(const Scene& sourceScene, SceneObject::Id sourceId, SceneObject::Id newParentId);
    void removeObjectRecursive(SceneObject::Id id);
    void rebuildWorldDataForObject(SceneObject::Id objectId, const QMatrix4x4& parentWorldMatrix);

    SceneObject::Id nextId_ = 1;
    int nextMeshHandle_ = 1;
    QHash<SceneObject::Id, SceneObject> objects_;
    QHash<int, MeshData> meshes_;
    Bounds3D sceneBounds_;
    SceneAnimationState animationState_;
};
