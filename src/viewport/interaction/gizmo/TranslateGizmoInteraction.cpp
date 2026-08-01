#include "viewport/interaction/gizmo/TranslateGizmoInteraction.h"

#include <QLineF>
#include <QPolygonF>
#include <QVector2D>

#include <limits>

#include "viewport/interaction/ViewportInteractionMath.h"

namespace
{
constexpr qreal kPlanePickPadding = 10.0;

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

qreal distancePointToPolygonEdges(const QPointF& point, const QPolygonF& polygon)
{
    qreal bestDistance = std::numeric_limits<qreal>::max();
    for (int index = 0; index < polygon.size(); ++index) {
        const QPointF a = polygon[index];
        const QPointF b = polygon[(index + 1) % polygon.size()];
        bestDistance = qMin(bestDistance, distancePointToSegment(point, a, b));
    }
    return bestDistance;
}

QPolygonF buildPlaneHandlePolygon(
    const QVector3D& origin,
    const QVector3D& axisA,
    const QVector3D& axisB,
    float start,
    float end,
    const QMatrix4x4& projectionMatrix,
    const QMatrix4x4& viewMatrix,
    int viewportWidth,
    int viewportHeight)
{
    return QPolygonF {
        ViewportInteractionMath::projectWorldToScreen(projectionMatrix, viewMatrix, origin + axisA * start + axisB * start, viewportWidth, viewportHeight),
        ViewportInteractionMath::projectWorldToScreen(projectionMatrix, viewMatrix, origin + axisA * end + axisB * start, viewportWidth, viewportHeight),
        ViewportInteractionMath::projectWorldToScreen(projectionMatrix, viewMatrix, origin + axisA * end + axisB * end, viewportWidth, viewportHeight),
        ViewportInteractionMath::projectWorldToScreen(projectionMatrix, viewMatrix, origin + axisA * start + axisB * end, viewportWidth, viewportHeight)
    };
}

int pickAxisHandle(
    const QPoint& position,
    const QVector3D& origin,
    float size,
    const QVector<QVector3D>& worldAxes,
    const QMatrix4x4& projectionMatrix,
    const QMatrix4x4& viewMatrix,
    int viewportWidth,
    int viewportHeight)
{
    const QPointF mousePoint(position);
    int bestAxis = -1;
    qreal bestDistance = 8.0;

    for (int axis = 0; axis < 3; ++axis) {
        const QPointF a = ViewportInteractionMath::projectWorldToScreen(projectionMatrix, viewMatrix, origin, viewportWidth, viewportHeight);
        const QPointF b = ViewportInteractionMath::projectWorldToScreen(projectionMatrix, viewMatrix, origin + worldAxes[axis] * size, viewportWidth, viewportHeight);
        const qreal distance = distancePointToSegment(mousePoint, a, b);
        if (distance < bestDistance) {
            bestDistance = distance;
            bestAxis = axis;
        }
    }

    return bestAxis;
}
}

namespace TranslateGizmoInteraction
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
    const float planeStart = size * 0.22f;
    const float planeEnd = size * 0.42f;
    const float centerHalfExtent = size * 0.10f;

    struct PlaneCandidate
    {
        int handle;
        QPolygonF polygon;
    };

    const QVector<PlaneCandidate> planeCandidates = {
        { 3, buildPlaneHandlePolygon(origin, worldAxes[0], worldAxes[1], planeStart, planeEnd, projectionMatrix, viewMatrix, viewportWidth, viewportHeight) },
        { 4, buildPlaneHandlePolygon(origin, worldAxes[1], worldAxes[2], planeStart, planeEnd, projectionMatrix, viewMatrix, viewportWidth, viewportHeight) },
        { 5, buildPlaneHandlePolygon(origin, worldAxes[0], worldAxes[2], planeStart, planeEnd, projectionMatrix, viewMatrix, viewportWidth, viewportHeight) }
    };

    int bestPlaneHandle = -1;
    qreal bestPlaneDistance = kPlanePickPadding;
    for (const PlaneCandidate& candidate : planeCandidates) {
        if (candidate.polygon.containsPoint(mousePoint, Qt::OddEvenFill)) {
            return candidate.handle;
        }

        const qreal planeDistance = distancePointToPolygonEdges(mousePoint, candidate.polygon);
        if (planeDistance < bestPlaneDistance) {
            bestPlaneDistance = planeDistance;
            bestPlaneHandle = candidate.handle;
        }
    }

    const QVector3D centerPlaneNormal = -cameraForward.normalized();
    QVector3D centerBasisA = QVector3D::crossProduct(centerPlaneNormal, QVector3D(0.0f, 1.0f, 0.0f));
    if (centerBasisA.lengthSquared() < 0.0001f) {
        centerBasisA = QVector3D::crossProduct(centerPlaneNormal, QVector3D(1.0f, 0.0f, 0.0f));
    }
    centerBasisA.normalize();
    const QVector3D centerBasisB = QVector3D::crossProduct(centerPlaneNormal, centerBasisA).normalized();
    const QPolygonF centerPolygon = buildPlaneHandlePolygon(
        origin,
        centerBasisA,
        centerBasisB,
        -centerHalfExtent,
        centerHalfExtent,
        projectionMatrix,
        viewMatrix,
        viewportWidth,
        viewportHeight);

    if (centerPolygon.containsPoint(mousePoint, Qt::OddEvenFill)) {
        return 6;
    }

    const qreal centerDistance = distancePointToPolygonEdges(mousePoint, centerPolygon);
    if (centerDistance < bestPlaneDistance) {
        return 6;
    }

    const int axisHandle = pickAxisHandle(
        position,
        origin,
        size,
        worldAxes,
        projectionMatrix,
        viewMatrix,
        viewportWidth,
        viewportHeight);

    if (bestPlaneHandle != -1) {
        return bestPlaneHandle;
    }

    return axisHandle;
}
}
