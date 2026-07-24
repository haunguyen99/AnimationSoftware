#pragma once

#include <QString>
#include <QVector>

#include <cstdint>

#include "scene/AnimationData.h"
#include "scene/Bounds3D.h"
#include "scene/Transform.h"

struct SkinWeight
{
    std::uint64_t jointId = 0;
    float weight = 0.0f;
};

using VertexSkinWeights = QVector<SkinWeight>;
using SkinWeightTable = QVector<VertexSkinWeights>;

class SceneObject
{
public:
    using Id = std::uint64_t;
    enum class Kind
    {
        Transform,
        Joint
    };

    explicit SceneObject(Id id = 0);

    Id id() const;

    const QString& name() const;
    void setName(const QString& name);
    Kind kind() const;
    void setKind(Kind kind);
    bool isJoint() const;

    Id parentId() const;
    void setParentId(Id parentId);

    const QVector<Id>& childIds() const;
    void addChildId(Id childId);
    bool removeChildId(Id childId);
    void clearChildren();

    const Transform& localTransform() const;
    void setLocalTransform(const Transform& transform);
    const Transform& authoredTransform() const;
    void setAuthoredTransform(const Transform& transform);
    const QQuaternion& jointOrientation() const;
    void setJointOrientation(const QQuaternion& orientation);
    const Transform& bindPoseLocalTransform() const;
    void setBindPoseLocalTransform(const Transform& transform);
    bool hasBindPose() const;
    void setHasBindPose(bool hasBindPose);
    bool hasSkinBinding() const;
    void setHasSkinBinding(bool hasSkinBinding);
    const Transform& skinBindLocalTransform() const;
    void setSkinBindLocalTransform(const Transform& transform);
    const QVector<Id>& skinJointIds() const;
    void setSkinJointIds(const QVector<Id>& jointIds);
    const SkinWeightTable& skinWeights() const;
    void setSkinWeights(const SkinWeightTable& weights);
    void clearSkinBinding();
    bool hasAnimation() const;
    bool hasTransformKeyframe(int frame) const;
    const TransformKeyframeTrack& transformKeyframes() const;
    void setTransformKeyframes(const TransformKeyframeTrack& keyframes);
    void setTransformKeyframe(int frame, const Transform& transform);
    bool removeTransformKeyframe(int frame);
    bool duplicateTransformKeyframe(int sourceFrame, int targetFrame);
    bool offsetAllTransformKeyframes(int frameDelta);
    int nextTransformKeyframeAfter(int frame) const;
    int previousTransformKeyframeBefore(int frame) const;

    const Bounds3D& localBounds() const;
    void setLocalBounds(const Bounds3D& bounds);

    const Bounds3D& worldBounds() const;
    void setWorldBounds(const Bounds3D& bounds);

    const QVector<int>& meshHandles() const;
    void addMeshHandle(int meshHandle);
    void clearMeshHandles();
    bool isVisible() const;
    void setVisible(bool visible);

private:
    Id id_ = 0;
    QString name_;
    Kind kind_ = Kind::Transform;
    Id parentId_ = 0;
    QVector<Id> childIds_;
    QVector<int> meshHandles_;
    Transform localTransform_;
    Transform authoredTransform_;
    QQuaternion jointOrientation_ { 1.0f, 0.0f, 0.0f, 0.0f };
    Transform bindPoseLocalTransform_;
    bool hasBindPose_ = false;
    bool hasSkinBinding_ = false;
    Transform skinBindLocalTransform_;
    QVector<Id> skinJointIds_;
    SkinWeightTable skinWeights_;
    TransformKeyframeTrack transformKeyframes_;
    Bounds3D localBounds_;
    Bounds3D worldBounds_;
    bool visible_ = true;
};
