#include "rendering/geometry/gizmo/RotateGizmoGeometry.h"

#include <QtMath>

namespace
{
constexpr int kGizmoArcSegments = 96;
constexpr float kRotateRingRadiusRatio = 0.86f;
constexpr float kViewRingRadiusRatio = 1.20f;

void appendLine(QVector<RenderVertex>& vertices, const QVector3D& a, const QVector3D& b, const QVector3D& color)
{
    const QVector3D normal(0.0f, 1.0f, 0.0f);
    vertices.append({ a, normal, color });
    vertices.append({ b, normal, color });
}

void appendRotateRing(
    QVector<RenderVertex>& vertices,
    const QVector3D& origin,
    const QVector3D& axis,
    const QVector3D& color,
    float size,
    const QVector3D& cameraForward,
    bool hideBackFacing)
{
    const QVector3D normalAxis = axis.normalized();
    const QVector3D viewDirection = cameraForward.normalized();
    QVector3D basisA = QVector3D::crossProduct(normalAxis, QVector3D(0.0f, 1.0f, 0.0f));
    if (basisA.lengthSquared() < 0.0001f) {
        basisA = QVector3D::crossProduct(normalAxis, QVector3D(1.0f, 0.0f, 0.0f));
    }
    basisA.normalize();
    const QVector3D basisB = QVector3D::crossProduct(normalAxis, basisA).normalized();

    for (int segment = 0; segment < kGizmoArcSegments; ++segment) {
        const float angleA = (static_cast<float>(segment) / kGizmoArcSegments) * 360.0f;
        const float angleB = (static_cast<float>(segment + 1) / kGizmoArcSegments) * 360.0f;
        const float radiansA = qDegreesToRadians(angleA);
        const float radiansB = qDegreesToRadians(angleB);

        const QVector3D pointA = origin + (basisA * qCos(radiansA) + basisB * qSin(radiansA)) * size;
        const QVector3D pointB = origin + (basisA * qCos(radiansB) + basisB * qSin(radiansB)) * size;
        if (hideBackFacing) {
            const QVector3D averageVector = ((pointA - origin) + (pointB - origin)).normalized();
            const float facing = QVector3D::dotProduct(averageVector, viewDirection);
            if (facing >= 0.0f) {
                continue;
            }
        }

        appendLine(vertices, pointA, pointB, color);
    }
}
}

namespace RotateGizmoGeometry
{
void appendGizmo(
    QVector<RenderVertex>& vertices,
    const QVector3D& origin,
    float size,
    const QVector<QVector3D>& axes,
    const QVector3D& cameraForward,
    const QVector3D& xColor,
    const QVector3D& yColor,
    const QVector3D& zColor,
    int activeAxis)
{
    appendRotateRing(vertices, origin, axes[0], xColor, size * kRotateRingRadiusRatio, cameraForward, true);
    appendRotateRing(vertices, origin, axes[1], yColor, size * kRotateRingRadiusRatio, cameraForward, true);
    appendRotateRing(vertices, origin, axes[2], zColor, size * kRotateRingRadiusRatio, cameraForward, true);
    appendRotateRing(
        vertices,
        origin,
        -cameraForward.normalized(),
        activeAxis == 6 ? QVector3D(0.98f, 0.82f, 0.24f) : QVector3D(0.74f, 0.92f, 0.98f),
        size * kViewRingRadiusRatio,
        cameraForward,
        false);
}
}
