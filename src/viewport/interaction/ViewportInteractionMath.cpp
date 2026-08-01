#include "viewport/interaction/ViewportInteractionMath.h"

#include <QLineF>
#include <QPolygonF>
#include <QVector2D>
#include <QtMath>

#include <limits>

namespace
{
constexpr int kRotatePickSegments = 48;
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
    if (polygon.size() < 2) {
        return std::numeric_limits<qreal>::max();
    }

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
}

namespace ViewportInteractionMath
{
float rayBoundsDistance(const QVector3D& rayOrigin, const QVector3D& rayDirection, const Bounds3D& bounds)
{
    if (!bounds.isValid()) {
        return -1.0f;
    }

    float tMin = 0.0f;
    float tMax = std::numeric_limits<float>::max();
    const QVector3D minPoint = bounds.min();
    const QVector3D maxPoint = bounds.max();

    for (int axis = 0; axis < 3; ++axis) {
        const float origin = axis == 0 ? rayOrigin.x() : (axis == 1 ? rayOrigin.y() : rayOrigin.z());
        const float direction = axis == 0 ? rayDirection.x() : (axis == 1 ? rayDirection.y() : rayDirection.z());
        const float minValue = axis == 0 ? minPoint.x() : (axis == 1 ? minPoint.y() : minPoint.z());
        const float maxValue = axis == 0 ? maxPoint.x() : (axis == 1 ? maxPoint.y() : maxPoint.z());

        if (qFuzzyIsNull(direction)) {
            if (origin < minValue || origin > maxValue) {
                return -1.0f;
            }
            continue;
        }

        const float inverse = 1.0f / direction;
        float t1 = (minValue - origin) * inverse;
        float t2 = (maxValue - origin) * inverse;
        if (t1 > t2) {
            std::swap(t1, t2);
        }

        tMin = qMax(tMin, t1);
        tMax = qMin(tMax, t2);
        if (tMin > tMax) {
            return -1.0f;
        }
    }

    return tMin;
}

QPointF projectWorldToScreen(
    const QMatrix4x4& projectionMatrix,
    const QMatrix4x4& viewMatrix,
    const QVector3D& worldPosition,
    int viewportWidth,
    int viewportHeight)
{
    const QVector4D clipPosition = projectionMatrix * viewMatrix * QVector4D(worldPosition, 1.0f);
    if (qFuzzyIsNull(clipPosition.w())) {
        return QPointF();
    }

    const QVector3D ndc = clipPosition.toVector3DAffine();
    const qreal screenX = (ndc.x() * 0.5 + 0.5) * viewportWidth;
    const qreal screenY = (1.0 - (ndc.y() * 0.5 + 0.5)) * viewportHeight;
    return QPointF(screenX, screenY);
}

void screenPosToWorldRay(
    const QMatrix4x4& projectionMatrix,
    const QMatrix4x4& viewMatrix,
    int viewportWidth,
    int viewportHeight,
    const QPoint& position,
    QVector3D& rayOrigin,
    QVector3D& rayDirection)
{
    const QMatrix4x4 inverseProjectionView = (projectionMatrix * viewMatrix).inverted();
    const float x = (2.0f * static_cast<float>(position.x()) / qMax(1, viewportWidth)) - 1.0f;
    const float y = 1.0f - (2.0f * static_cast<float>(position.y()) / qMax(1, viewportHeight));
    const QVector4D nearPoint = inverseProjectionView * QVector4D(x, y, -1.0f, 1.0f);
    const QVector4D farPoint = inverseProjectionView * QVector4D(x, y, 1.0f, 1.0f);
    rayOrigin = nearPoint.toVector3DAffine();
    rayDirection = (farPoint.toVector3DAffine() - rayOrigin).normalized();
}

bool intersectRayPlane(
    const QVector3D& rayOrigin,
    const QVector3D& rayDirection,
    const QVector3D& planeOrigin,
    const QVector3D& planeNormal,
    QVector3D* hitPoint)
{
    const QVector3D normalizedNormal = planeNormal.normalized();
    const float denominator = QVector3D::dotProduct(normalizedNormal, rayDirection);
    if (qAbs(denominator) < 0.0001f) {
        return false;
    }

    const float t = QVector3D::dotProduct(planeOrigin - rayOrigin, normalizedNormal) / denominator;
    if (t < 0.0f) {
        return false;
    }

    if (hitPoint != nullptr) {
        *hitPoint = rayOrigin + rayDirection * t;
    }
    return true;
}

SceneObject::Id pickObjectAtScreenPos(
    const Scene& scene,
    const QMatrix4x4& projectionMatrix,
    const QMatrix4x4& viewMatrix,
    int viewportWidth,
    int viewportHeight,
    const QPoint& position)
{
    QVector3D rayOrigin;
    QVector3D rayDirection;
    screenPosToWorldRay(projectionMatrix, viewMatrix, viewportWidth, viewportHeight, position, rayOrigin, rayDirection);

    SceneObject::Id bestId = 0;
    float bestDistance = std::numeric_limits<float>::max();
    for (SceneObject::Id objectId : scene.allObjectIds()) {
        const SceneObject* object = scene.findObject(objectId);
        if (object == nullptr || !object->isVisible() || !object->worldBounds().isValid()) {
            continue;
        }

        const float distance = rayBoundsDistance(rayOrigin, rayDirection, object->worldBounds());
        if (distance >= 0.0f && distance < bestDistance) {
            bestDistance = distance;
            bestId = objectId;
        }
    }

    return bestId;
}

int pickRotateGizmoAxisAtScreenPos(
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
            const QPointF pointA = projectWorldToScreen(projectionMatrix, viewMatrix, pointA3D, viewportWidth, viewportHeight);
            const QPointF pointB = projectWorldToScreen(projectionMatrix, viewMatrix, pointB3D, viewportWidth, viewportHeight);

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
        const QPointF pointA = projectWorldToScreen(projectionMatrix, viewMatrix, pointA3D, viewportWidth, viewportHeight);
        const QPointF pointB = projectWorldToScreen(projectionMatrix, viewMatrix, pointB3D, viewportWidth, viewportHeight);
        viewRingDistance = qMin(viewRingDistance, distancePointToSegment(mousePoint, pointA, pointB));
    }

    if (viewRingDistance < bestDistance) {
        return 6;
    }

    return bestAxis;
}

int pickTranslateScaleGizmoAxisAtScreenPos(
    const QPoint& position,
    const QVector3D& origin,
    float size,
    const QVector<QVector3D>& worldAxes,
    const QMatrix4x4& projectionMatrix,
    const QMatrix4x4& viewMatrix,
    int viewportWidth,
    int viewportHeight,
    bool includeScaleHandle)
{
    if (worldAxes.size() < 3) {
        return -1;
    }

    const QPointF mousePoint(position);
    int bestAxis = -1;
    qreal bestDistance = includeScaleHandle ? 12.0 : 8.0;

    for (int axis = 0; axis < 3; ++axis) {
        const QPointF a = projectWorldToScreen(projectionMatrix, viewMatrix, origin, viewportWidth, viewportHeight);
        const QPointF b = projectWorldToScreen(projectionMatrix, viewMatrix, origin + worldAxes[axis] * size, viewportWidth, viewportHeight);
        qreal distance = distancePointToSegment(mousePoint, a, b);

        if (includeScaleHandle) {
            distance = qMin(distance, QLineF(mousePoint, b).length());
        }

        if (distance < bestDistance) {
            bestDistance = distance;
            bestAxis = axis;
        }
    }

    return bestAxis;
}

int pickTranslateGizmoHandleAtScreenPos(
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

    const int axisHandle = pickTranslateScaleGizmoAxisAtScreenPos(
        position,
        origin,
        size,
        worldAxes,
        projectionMatrix,
        viewMatrix,
        viewportWidth,
        viewportHeight,
        false);

    if (bestPlaneHandle != -1) {
        return bestPlaneHandle;
    }

    return axisHandle;
}

int pickScaleGizmoHandleAtScreenPos(
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
    const float planeStart = size * 0.18f;
    const float planeEnd = size * 0.34f;
    const float centerHalfExtent = size * 0.08f;

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

    const int axisHandle = pickTranslateScaleGizmoAxisAtScreenPos(
        position,
        origin,
        size,
        worldAxes,
        projectionMatrix,
        viewMatrix,
        viewportWidth,
        viewportHeight,
        true);

    if (bestPlaneHandle != -1) {
        return bestPlaneHandle;
    }

    return axisHandle;
}
}
