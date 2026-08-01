#include "rendering/geometry/gizmo/ScaleGizmoGeometry.h"

namespace
{
constexpr float kScaleHandleSizeRatio = 0.08f;
constexpr float kScalePlaneStartRatio = 0.18f;
constexpr float kScalePlaneEndRatio = 0.34f;
constexpr float kScaleCenterExtentRatio = 0.08f;

QVector3D planeColor(const QVector3D& axisColorA, const QVector3D& axisColorB, bool active)
{
    if (active) {
        return QVector3D(0.98f, 0.82f, 0.24f);
    }

    return ((axisColorA + axisColorB) * 0.5f) * 0.9f;
}

void appendLine(QVector<RenderVertex>& vertices, const QVector3D& a, const QVector3D& b, const QVector3D& color)
{
    const QVector3D normal(0.0f, 1.0f, 0.0f);
    vertices.append({ a, normal, color });
    vertices.append({ b, normal, color });
}

void appendScaleAxis(QVector<RenderVertex>& vertices, const QVector3D& origin, const QVector3D& axis, const QVector3D& color, float size)
{
    const QVector3D normalizedAxis = axis.normalized();
    const QVector3D handleCenter = origin + normalizedAxis * size;
    appendLine(vertices, origin, handleCenter, color);

    QVector3D tangent = QVector3D::crossProduct(normalizedAxis, QVector3D(0.0f, 1.0f, 0.0f));
    if (tangent.lengthSquared() < 0.0001f) {
        tangent = QVector3D::crossProduct(normalizedAxis, QVector3D(1.0f, 0.0f, 0.0f));
    }
    tangent.normalize();
    const QVector3D bitangent = QVector3D::crossProduct(normalizedAxis, tangent).normalized();

    const QVector3D sideA = tangent * size * kScaleHandleSizeRatio;
    const QVector3D sideB = bitangent * size * kScaleHandleSizeRatio;

    const QVector3D p1 = handleCenter - sideA - sideB;
    const QVector3D p2 = handleCenter + sideA - sideB;
    const QVector3D p3 = handleCenter + sideA + sideB;
    const QVector3D p4 = handleCenter - sideA + sideB;

    appendLine(vertices, p1, p2, color);
    appendLine(vertices, p2, p3, color);
    appendLine(vertices, p3, p4, color);
    appendLine(vertices, p4, p1, color);
}

void appendScalePlaneHandle(QVector<RenderVertex>& vertices, const QVector3D& origin, const QVector3D& axisA, const QVector3D& axisB, const QVector3D& color, float size)
{
    const QVector3D normalizedAxisA = axisA.normalized();
    const QVector3D normalizedAxisB = axisB.normalized();
    const float start = size * kScalePlaneStartRatio;
    const float end = size * kScalePlaneEndRatio;

    const QVector3D p00 = origin + normalizedAxisA * start + normalizedAxisB * start;
    const QVector3D p10 = origin + normalizedAxisA * end + normalizedAxisB * start;
    const QVector3D p11 = origin + normalizedAxisA * end + normalizedAxisB * end;
    const QVector3D p01 = origin + normalizedAxisA * start + normalizedAxisB * end;

    appendLine(vertices, p00, p10, color);
    appendLine(vertices, p10, p11, color);
    appendLine(vertices, p11, p01, color);
    appendLine(vertices, p01, p00, color);
}

void appendScaleCenterHandle(QVector<RenderVertex>& vertices, const QVector3D& origin, const QVector3D& cameraForward, const QVector3D& color, float size)
{
    QVector3D basisA = QVector3D::crossProduct(cameraForward.normalized(), QVector3D(0.0f, 1.0f, 0.0f));
    if (basisA.lengthSquared() < 0.0001f) {
        basisA = QVector3D::crossProduct(cameraForward.normalized(), QVector3D(1.0f, 0.0f, 0.0f));
    }
    basisA.normalize();
    const QVector3D basisB = QVector3D::crossProduct(cameraForward.normalized(), basisA).normalized();
    const float extent = size * kScaleCenterExtentRatio;

    const QVector3D p00 = origin - basisA * extent - basisB * extent;
    const QVector3D p10 = origin + basisA * extent - basisB * extent;
    const QVector3D p11 = origin + basisA * extent + basisB * extent;
    const QVector3D p01 = origin - basisA * extent + basisB * extent;

    appendLine(vertices, p00, p10, color);
    appendLine(vertices, p10, p11, color);
    appendLine(vertices, p11, p01, color);
    appendLine(vertices, p01, p00, color);
}
}

namespace ScaleGizmoGeometry
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
    appendScaleAxis(vertices, origin, axes[0], xColor, size);
    appendScaleAxis(vertices, origin, axes[1], yColor, size);
    appendScaleAxis(vertices, origin, axes[2], zColor, size);
    appendScalePlaneHandle(vertices, origin, axes[0], axes[1], planeColor(xColor, yColor, activeAxis == 3), size);
    appendScalePlaneHandle(vertices, origin, axes[1], axes[2], planeColor(yColor, zColor, activeAxis == 4), size);
    appendScalePlaneHandle(vertices, origin, axes[0], axes[2], planeColor(xColor, zColor, activeAxis == 5), size);
    appendScaleCenterHandle(vertices, origin, -cameraForward, activeAxis == 6 ? QVector3D(0.98f, 0.82f, 0.24f) : QVector3D(0.92f, 0.92f, 0.92f), size);
}
}
