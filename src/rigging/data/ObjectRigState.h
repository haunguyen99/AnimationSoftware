#pragma once

#include <QQuaternion>
#include <QVector>

#include <cstdint>

#include "scene/Transform.h"

struct SkinWeight
{
    std::uint64_t jointId = 0;
    float weight = 0.0f;
};

using VertexSkinWeights = QVector<SkinWeight>;
using SkinWeightTable = QVector<VertexSkinWeights>;

class ObjectRigState
{
public:
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
    const QVector<std::uint64_t>& skinJointIds() const;
    void setSkinJointIds(const QVector<std::uint64_t>& jointIds);
    const SkinWeightTable& skinWeights() const;
    void setSkinWeights(const SkinWeightTable& weights);
    void clearSkinBinding();

private:
    QQuaternion jointOrientation_ { 1.0f, 0.0f, 0.0f, 0.0f };
    Transform bindPoseLocalTransform_;
    bool hasBindPose_ = false;
    bool hasSkinBinding_ = false;
    Transform skinBindLocalTransform_;
    QVector<std::uint64_t> skinJointIds_;
    SkinWeightTable skinWeights_;
};
