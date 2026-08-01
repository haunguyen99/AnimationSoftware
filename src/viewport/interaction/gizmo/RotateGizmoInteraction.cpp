#include "viewport/interaction/gizmo/RotateGizmoInteraction.h"

#include <QLineF>
#include <QVector2D>
#include <QtMath>

#include "viewport/interaction/ViewportInteractionMath.h"

namespace
{
constexpr int kRotatePickSegments = 48;

qreal distancePointToSegment(const QPointF& point, const QPointF& a, const QPointF& b)
{
    const QVector2D line(b - a);
    if (qFuzzyIsNull(line.lengthSquared())) {
        return QLineF(point, a).length();
    }

    const QVector2D offset(point - a);
    const float t = qBound(0.0f, QVector2D::dotProduct(offset, line) / line.lengthSquared(), 1.0f);
    const QPointF closest = a + (b - a) * t;
    return QLineF(point, closest).length();
}
}

namespace RotateGizmoInteraction
{
int pickHandle(
    const QPoint& position,
    const QVector3D& origin,
    float size,
    const QVector<QVector3D>& worldAxes,
    const QVector3D& cameraForward,
    const QMatrix4x4& projectionMatrix,
    const QMatrix4x4& viewMatrix,
    int viewportWidth,
    int viewportHeight)
{
    if (worldAxes.size() < 3) {
        return -1;
    }

    const QPointF mousePoint(position);
    int bestAxis = -1;
    qreal bestDistance = 14.0;

    for (int axis = 0; axis < 3; ++axis) {
        const QVector3D normalAxis = worldAxes[axis].normalized();
        QVector3D basisA = QVector3D::crossProduct(normalAxis, QVector3D(0.0f, 1.0f, 0.0f));
        if (basisA.lengthSquared() < 0.0001f) {
            basisA = QVector3D::crossProduct(normalAxis, QVector3D(1.0f, 0.0f, 0.0f));
        }
        basisA.normalize();
        const QVector3D basisB = QVector3D::crossProduct(normalAxis, basisA).normalized();

        qreal axisDistance = bestDistance;
        for (int segment = 0; segment < kRotatePickSegments; ++segment) {
            const float angleA = (static_cast<float>(segment) / kRotatePickSegments) * 360.0f;
            const float angleB = (static_cast<float>(segment + 1) / kRotatePickSegments) * 360.0f;
            const float radiansA = qDegreesToRadians(angleA);
            const float radiansB = qDegreesToRadians(angleB);

            const QVector3D pointA3D = origin + (basisA * qCos(radiansA) + basisB * qSin(radiansA)) * (size * 0.9f);
            const QVector3D pointB3D = origin + (basisA * qCos(radiansB) + basisB * qSin(radiansB)) * (size * 0.9f);
            const QPointF pointA = ViewportInteractionMath::projectWorldToScreen(projectionMatrix, viewMatrix, pointA3D, viewportWidth, viewportHeight);
            const QPointF pointB = ViewportInteractionMath::projectWorldToScreen(projectionMatrix, viewMatrix, pointB3D, viewportWidth, viewportHeight);

            axisDistance = qMin(axisDistance, distancePointToSegment(mousePoint, pointA, pointB));
        }

        if (axisDistance < bestDistance) {
            bestDistance = axisDistance;
            bestAxis = axis;
        }
    }

    QVector3D viewBasisA = QVector3D::crossProduct(cameraForward.normalized(), QVector3D(0.0f, 1.0f, 0.0f));
    if (viewBasisA.lengthSquared() < 0.0001f) {
        viewBasisA = QVector3D::crossProduct(cameraForward.normalized(), QVector3D(1.0f, 0.0f, 0.0f));
    }
    viewBasisA.normalize();
    const QVector3D viewBasisB = QVector3D::crossProduct(cameraForward.normalized(), viewBasisA).normalized();

    qreal viewRingDistance = bestDistance;
    for (int segment = 0; segment < kRotatePickSegments; ++segment) {
        const float angleA = (static_cast<float>(segment) / kRotatePickSegments) * 360.0f;
        const float angleB = (static_cast<float>(segment + 1) / kRotatePickSegments) * 360.0f;
        const float radiansA = qDegreesToRadians(angleA);
        const float radiansB = qDegreesToRadians(angleB);

        const QVector3D pointA3D = origin + (viewBasisA * qCos(radiansA) + viewBasisB * qSin(radiansA)) * (size * 1.14f);
        const QVector3D pointB3D = origin + (viewBasisA * qCos(radiansB) + viewBasisB * qSin(radiansB)) * (size * 1.14f);
        const QPointF pointA = ViewportInteractionMath::projectWorldToScreen(projectionMatrix, viewMatrix, pointA3D, viewportWidth, viewportHeight);
        const QPointF pointB = ViewportInteractionMath::projectWorldToScreen(projectionMatrix, viewMatrix, pointB3D, viewportWidth, viewportHeight);
        viewRingDistance = qMin(viewRingDistance, distancePointToSegment(mousePoint, pointA, pointB));
    }

    if (viewRingDistance < bestDistance) {
        return 6;
    }

    return bestAxis;
}
}
