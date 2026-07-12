#pragma once

#include <QVector>

#include "scene/Transform.h"

struct TransformKeyframe
{
    int frame = 0;
    Transform transform;
};

using TransformKeyframeTrack = QVector<TransformKeyframe>;
