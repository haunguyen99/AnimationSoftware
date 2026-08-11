#include "animation/data/CurveInterpolator.h"

#include <cmath>

namespace
{

constexpr double kPi = 3.14159265358979323846;

double cubicBezierValue(double p0, double p1, double p2, double p3, double t)
{
    const double mt = 1.0 - t;
    return mt * mt * mt * p0 + 3.0 * mt * mt * t * p1 + 3.0 * mt * t * t * p2 + t * t * t * p3;
}

// Binary search: find Bezier parameter t such that bezier_x(t) == targetX.
double findBezierT(double x0, double x1, double x2, double x3, double targetX)
{
    double lo = 0.0;
    double hi = 1.0;
    for (int i = 0; i < 32; ++i) {
        const double mid = (lo + hi) * 0.5;
        const double x = cubicBezierValue(x0, x1, x2, x3, mid);
        if (x < targetX) {
            lo = mid;
        } else {
            hi = mid;
        }
    }
    return (lo + hi) * 0.5;
}

struct EffectiveTangent
{
    float outAngle;
    float outWeight; // fraction of segment length
    float inAngle;
    float inWeight;
};

// Catmull-Rom auto tangent for the key at index.
EffectiveTangent computeAutoTangent(const QVector<CurveInterpolator::CurveKey>& keys, int index)
{
    const CurveInterpolator::CurveKey& curr = keys[index];
    const bool hasPrev = index > 0;
    const bool hasNext = index < keys.size() - 1;

    double slope = 0.0;
    if (hasPrev && hasNext) {
        const double df = static_cast<double>(keys[index + 1].frame - keys[index - 1].frame);
        if (std::abs(df) > 1e-8) {
            slope = (keys[index + 1].value - keys[index - 1].value) / df;
        }
    } else if (hasNext) {
        const double df = static_cast<double>(keys[index + 1].frame - curr.frame);
        if (std::abs(df) > 1e-8) {
            slope = (keys[index + 1].value - curr.value) / df;
        }
    } else if (hasPrev) {
        const double df = static_cast<double>(curr.frame - keys[index - 1].frame);
        if (std::abs(df) > 1e-8) {
            slope = (curr.value - keys[index - 1].value) / df;
        }
    }

    const float angle = static_cast<float>(std::atan(slope) * 180.0 / kPi);
    return { angle, 1.0f / 3.0f, angle, 1.0f / 3.0f };
}

} // namespace

namespace CurveInterpolator
{

double evaluateChannel(const QVector<CurveKey>& keys, float frame)
{
    if (keys.isEmpty()) {
        return 0.0;
    }
    if (frame <= keys.first().frame) {
        return keys.first().value;
    }
    if (frame >= keys.last().frame) {
        return keys.last().value;
    }

    for (int i = 0; i < keys.size() - 1; ++i) {
        const CurveKey& a = keys[i];
        const CurveKey& b = keys[i + 1];
        if (frame < a.frame || frame > b.frame) {
            continue;
        }
        if (std::abs(a.frame - b.frame) < 1e-8f) {
            return b.value;
        }

        // Stepped: hold a's value until b
        if (a.tangent.mode == TangentMode::Stepped) {
            return a.value;
        }

        // Linear: simple lerp
        if (a.tangent.mode == TangentMode::Linear) {
            const double t = static_cast<double>(frame - a.frame) / static_cast<double>(b.frame - a.frame);
            return a.value + t * (b.value - a.value);
        }

        // Auto, Flat, Broken: cubic bezier
        const EffectiveTangent tanA = (a.tangent.mode == TangentMode::Auto)
            ? computeAutoTangent(keys, i)
            : EffectiveTangent{ a.tangent.outAngle, a.tangent.outWeight, a.tangent.inAngle, a.tangent.inWeight };

        const EffectiveTangent tanB = (b.tangent.mode == TangentMode::Auto)
            ? computeAutoTangent(keys, i + 1)
            : EffectiveTangent{ b.tangent.outAngle, b.tangent.outWeight, b.tangent.inAngle, b.tangent.inWeight };

        const float outAngle = (a.tangent.mode == TangentMode::Flat) ? 0.0f : tanA.outAngle;
        const float inAngle  = (b.tangent.mode == TangentMode::Flat) ? 0.0f : tanB.inAngle;

        const double segLen   = static_cast<double>(b.frame - a.frame);
        const double outSlope = std::tan(static_cast<double>(outAngle) * kPi / 180.0);
        const double inSlope  = std::tan(static_cast<double>(inAngle) * kPi / 180.0);

        // Control points in (frame, value) space — weight is fraction of segment
        const double cp1x = a.frame + tanA.outWeight * segLen;
        const double cp1y = a.value + tanA.outWeight * segLen * outSlope;
        const double cp2x = b.frame - tanB.inWeight * segLen;
        const double cp2y = b.value - tanB.inWeight * segLen * inSlope;

        const double tParam = findBezierT(a.frame, cp1x, cp2x, b.frame, frame);
        return cubicBezierValue(a.value, cp1y, cp2y, b.value, tParam);
    }

    return keys.last().value;
}

} // namespace CurveInterpolator
