#pragma once

#include <QVector>

#include "animation/data/KeyTangent.h"

namespace CurveInterpolator
{

struct CurveKey
{
    float frame = 0.f;
    double value = 0.0;
    KeyTangent tangent;
};

// Evaluate a single animation channel at the given frame.
// keys must be sorted ascending by frame. Returns 0.0 for empty keys.
double evaluateChannel(const QVector<CurveKey>& keys, float frame);

} // namespace CurveInterpolator
