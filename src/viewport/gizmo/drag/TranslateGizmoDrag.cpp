#include "viewport/gizmo/drag/TranslateGizmoDrag.h"

#include <QLineF>
#include <QVector2D>

#include "viewport/interaction/ViewportInteractionMath.h"

namespace TranslateGizmoDrag
{
bool applyPlane(
    Transform& transform,
    const Transform& startTransform,
    const QPoint& currentPosition,
    const QVector3D& dragPlaneOrigin,
    const QVector3D& dragPlaneNormal,
    const QVector3D& dragStartWorldPoint,
    const QMatrix4x4& projectionMatrix,
    const QMatrix4x4& viewMatrix,
    int viewportWidth,
    int viewportHeight,
    const QMatrix4x4& parentWorldTransform)
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

    transform = startTransform;
    const QVector3D worldDelta = currentHitPoint - dragStartWorldPoint;
    transform.translation += parentWorldTransform.inverted().mapVector(worldDelta);
    return true;
}

bool applyAxis(
    Transform& transform,
    const Transform& startTransform,
    const QPoint& currentPosition,
    const QPoint& startMousePosition,
    const QVector3D& gizmoOrigin,
    float gizmoSize,
    const QVector3D& worldAxis,
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
    const QPointF axisScreen = ViewportInteractionMath::projectWorldToScreen(
        projectionMatrix,
        viewMatrix,
        gizmoOrigin + worldAxis * gizmoSize,
        viewportWidth,
        viewportHeight);

    QVector2D axisDirection(axisScreen - originScreen);
    if (qFuzzyIsNull(axisDirection.lengthSquared())) {
        return false;
    }

    axisDirection.normalize();
    const QVector2D mouseDelta(currentPosition - startMousePosition);
    const float screenDelta = QVector2D::dotProduct(mouseDelta, axisDirection);
    const float axisPixelLength = QLineF(originScreen, axisScreen).length();
    if (qFuzzyIsNull(axisPixelLength)) {
        return false;
    }

    transform = startTransform;
    const float worldDelta = screenDelta * (gizmoSize / axisPixelLength);
    transform.translation += parentWorldTransform.inverted().mapVector(worldAxis * worldDelta);
    return true;
}
}
