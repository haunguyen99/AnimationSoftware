#include "viewport/gizmo/drag/RotateGizmoDrag.h"

#include <QVector2D>
#include <QtMath>

#include "viewport/interaction/ViewportInteractionMath.h"

namespace RotateGizmoDrag
{
bool applyScreen(
    Transform& transform,
    const Transform& startTransform,
    const QPoint& currentPosition,
    const QPointF& startScreenVector,
    const QVector3D& gizmoOrigin,
    const QVector3D& dragPlaneNormal,
    const QMatrix4x4& projectionMatrix,
    const QMatrix4x4& viewMatrix,
    int viewportWidth,
    int viewportHeight,
    const QMatrix4x4& parentWorldTransform)
{
    const QPointF originScreen = ViewportInteractionMath::projectWorldToScreen(
        projectionMatrix,
        viewMatrix,
        gizmoOrigin,
        viewportWidth,
        viewportHeight);
    const QPointF currentScreenVector = QPointF(currentPosition) - originScreen;
    const QVector2D startVector(startScreenVector);
    const QVector2D currentVector(currentScreenVector);
    if (qFuzzyIsNull(startVector.lengthSquared()) || qFuzzyIsNull(currentVector.lengthSquared())) {
        return false;
    }

    const float angleDegrees = -qRadiansToDegrees(qAtan2(
        startVector.x() * currentVector.y() - startVector.y() * currentVector.x(),
        QVector2D::dotProduct(startVector, currentVector)));

    transform = startTransform;
    const QVector3D localAxis = parentWorldTransform.inverted().mapVector(dragPlaneNormal).normalized();
    transform.rotation = QQuaternion::fromAxisAndAngle(localAxis, angleDegrees) * transform.rotation;
    return true;
}

bool applyAxis(
    Transform& transform,
    const Transform& startTransform,
    const QPoint& currentPosition,
    const QVector3D& gizmoOrigin,
    const QVector3D& dragPlaneOrigin,
    const QVector3D& dragPlaneNormal,
    const QVector3D& dragStartWorldPoint,
    const QMatrix4x4& projectionMatrix,
    const QMatrix4x4& viewMatrix,
    int viewportWidth,
    int viewportHeight,
    const QMatrix4x4& parentWorldTransform,
    bool worldOrientation,
    const QVector3D& localAxis)
{
    QVector3D rayOrigin;
    QVector3D rayDirection;
    ViewportInteractionMath::screenPosToWorldRay(
        projectionMatrix,
        viewMatrix,
        viewportWidth,
        viewportHeight,
        currentPosition,
        rayOrigin,
        rayDirection);

    QVector3D currentHitPoint;
    if (!ViewportInteractionMath::intersectRayPlane(
            rayOrigin,
            rayDirection,
            dragPlaneOrigin,
            dragPlaneNormal,
            &currentHitPoint)) {
        return false;
    }

    QVector3D startVector = (dragStartWorldPoint - gizmoOrigin);
    QVector3D currentVector = (currentHitPoint - gizmoOrigin);
    if (qFuzzyIsNull(startVector.lengthSquared()) || qFuzzyIsNull(currentVector.lengthSquared())) {
        return false;
    }

    startVector.normalize();
    currentVector.normalize();
    const QVector3D rotationAxis = dragPlaneNormal.normalized();
    const float sinAngle = QVector3D::dotProduct(QVector3D::crossProduct(startVector, currentVector), rotationAxis);
    const float cosAngle = QVector3D::dotProduct(startVector, currentVector);
    const float angleDegrees = qRadiansToDegrees(qAtan2(sinAngle, cosAngle));

    transform = startTransform;
    if (worldOrientation) {
        const QVector3D worldLocalAxis = parentWorldTransform.inverted().mapVector(dragPlaneNormal).normalized();
        transform.rotation = QQuaternion::fromAxisAndAngle(worldLocalAxis, angleDegrees) * transform.rotation;
    } else {
        transform.rotation = transform.rotation * QQuaternion::fromAxisAndAngle(localAxis, angleDegrees);
    }
    return true;
}
}
