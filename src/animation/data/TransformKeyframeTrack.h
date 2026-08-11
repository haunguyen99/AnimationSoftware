#pragma once

#include <QVector>

#include "animation/data/KeyTangent.h"
#include "scene/Transform.h"

struct TransformKeyframe
{
    int frame = 0;
    Transform transform;
    KeyTangent tangent;
};

using TransformKeyframeTrack = QVector<TransformKeyframe>;
