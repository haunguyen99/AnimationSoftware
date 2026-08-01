#include "rendering/geometry/ViewportOverlayGeometryBuilder.h"
#include "rendering/geometry/gizmo/RotateGizmoGeometry.h"
#include "rendering/geometry/gizmo/ScaleGizmoGeometry.h"
#include "rendering/geometry/gizmo/TranslateGizmoGeometry.h"

#include <QtMath>

namespace
{
constexpr int kGizmoArcSegments = 96;
constexpr float kTranslateShaftRatio = 0.78f;
constexpr float kTranslateHeadLengthRatio = 0.22f;
constexpr float kTranslateHeadWidthRatio = 0.10f;
constexpr float kScaleHandleSizeRatio = 0.08f;
constexpr float kRotateRingRadiusRatio = 0.86f;
constexpr float kViewRingRadiusRatio = 1.20f;
constexpr float kTranslatePlaneStartRatio = 0.22f;
constexpr float kTranslatePlaneEndRatio = 0.42f;
constexpr float kTranslateCenterExtentRatio = 0.10f;
constexpr float kScalePlaneStartRatio = 0.18f;
constexpr float kScalePlaneEndRatio = 0.34f;
constexpr float kScaleCenterExtentRatio = 0.08f;

QVector3D axisColor(int axisIndex, bool active)
{
    if (active) {
        return QVector3D(0.98f, 0.82f, 0.24f);
    }

    switch (axisIndex) {
    case 0:
        return QVector3D(0.89f, 0.26f, 0.22f);
    case 1:
        return QVector3D(0.33f, 0.78f, 0.35f);
    case 2:
    default:
        return QVector3D(0.23f, 0.52f, 0.92f);
    }
}

QVector3D planeColor(const QVector3D& axisColorA, const QVector3D& axisColorB, bool active)
{
    if (active) {
        return QVector3D(0.98f, 0.82f, 0.24f);
    }

    return ((axisColorA + axisColorB) * 0.5f) * 0.9f;
}

void appendLine(QVector<RenderVertex>& vertices,
    const QVector3D& a,
    const QVector3D& b,
    const QVector3D& color)
{
    const QVector3D normal(0.0f, 1.0f, 0.0f);
    vertices.append({ a, normal, color });
    vertices.append({ b, normal, color });
}

void appendTranslateAxis(QVector<RenderVertex>& vertices,
    const QVector3D& origin,
    const QVector3D& axis,
    const QVector3D& color,
    float size)
{
    const QVector3D normalizedAxis = axis.normalized();
    const QVector3D tip = origin + normalizedAxis * size;
    const QVector3D headBase = origin + normalizedAxis * (size * kTranslateShaftRatio);
    appendLine(vertices, origin, headBase, color);
    appendLine(vertices, headBase, tip, color);

    QVector3D tangent = QVector3D::crossProduct(normalizedAxis, QVector3D(0.0f, 1.0f, 0.0f));
    if (tangent.lengthSquared() < 0.0001f) {
        tangent = QVector3D::crossProduct(normalizedAxis, QVector3D(1.0f, 0.0f, 0.0f));
    }
    tangent.normalize();
    QVector3D bitangent = QVector3D::crossProduct(normalizedAxis, tangent).normalized();

    const QVector3D back = normalizedAxis * (size * kTranslateHeadLengthRatio);
    const QVector3D sideA = tangent * size * kTranslateHeadWidthRatio;
    const QVector3D sideB = bitangent * size * kTranslateHeadWidthRatio;

    appendLine(vertices, tip, tip - back + sideA, color);
    appendLine(vertices, tip, tip - back - sideA, color);
    appendLine(vertices, tip, tip - back + sideB, color);
    appendLine(vertices, tip, tip - back - sideB, color);
}

void appendScaleAxis(QVector<RenderVertex>& vertices,
    const QVector3D& origin,
    const QVector3D& axis,
    const QVector3D& color,
    float size)
{
    const QVector3D normalizedAxis = axis.normalized();
    const QVector3D handleCenter = origin + normalizedAxis * size;
    appendLine(vertices, origin, handleCenter, color);

    QVector3D tangent = QVector3D::crossProduct(normalizedAxis, QVector3D(0.0f, 1.0f, 0.0f));
    if (tangent.lengthSquared() < 0.0001f) {
        tangent = QVector3D::crossProduct(normalizedAxis, QVector3D(1.0f, 0.0f, 0.0f));
    }
    tangent.normalize();
    QVector3D bitangent = QVector3D::crossProduct(normalizedAxis, tangent).normalized();

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

void appendRotateRing(QVector<RenderVertex>& vertices,
    const QVector3D& origin,
    const QVector3D& axis,
    const QVector3D& color,
    float size,
    const QVector3D& cameraForward,
    bool emphasizeVisibility)
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
        QVector3D segmentColor = color;
        if (emphasizeVisibility) {
            const QVector3D averageVector = ((pointA - origin) + (pointB - origin)).normalized();
            const float facing = QVector3D::dotProduct(averageVector, viewDirection);
            if (facing >= 0.0f) {
                continue;
            }
        }

        appendLine(vertices, pointA, pointB, segmentColor);
    }
}

void appendPlaneHandle(QVector<RenderVertex>& vertices,
    const QVector3D& origin,
    const QVector3D& axisA,
    const QVector3D& axisB,
    const QVector3D& color,
    float size)
{
    const QVector3D normalizedAxisA = axisA.normalized();
    const QVector3D normalizedAxisB = axisB.normalized();
    const float start = size * kTranslatePlaneStartRatio;
    const float end = size * kTranslatePlaneEndRatio;

    const QVector3D p00 = origin + normalizedAxisA * start + normalizedAxisB * start;
    const QVector3D p10 = origin + normalizedAxisA * end + normalizedAxisB * start;
    const QVector3D p11 = origin + normalizedAxisA * end + normalizedAxisB * end;
    const QVector3D p01 = origin + normalizedAxisA * start + normalizedAxisB * end;

    appendLine(vertices, p00, p10, color);
    appendLine(vertices, p10, p11, color);
    appendLine(vertices, p11, p01, color);
    appendLine(vertices, p01, p00, color);
}

void appendViewPlaneHandle(QVector<RenderVertex>& vertices,
    const QVector3D& origin,
    const QVector3D& cameraForward,
    const QVector3D& color,
    float size)
{
    QVector3D basisA = QVector3D::crossProduct(cameraForward.normalized(), QVector3D(0.0f, 1.0f, 0.0f));
    if (basisA.lengthSquared() < 0.0001f) {
        basisA = QVector3D::crossProduct(cameraForward.normalized(), QVector3D(1.0f, 0.0f, 0.0f));
    }
    basisA.normalize();
    const QVector3D basisB = QVector3D::crossProduct(cameraForward.normalized(), basisA).normalized();
    const float extent = size * kTranslateCenterExtentRatio;

    const QVector3D p00 = origin - basisA * extent - basisB * extent;
    const QVector3D p10 = origin + basisA * extent - basisB * extent;
    const QVector3D p11 = origin + basisA * extent + basisB * extent;
    const QVector3D p01 = origin - basisA * extent + basisB * extent;

    appendLine(vertices, p00, p10, color);
    appendLine(vertices, p10, p11, color);
    appendLine(vertices, p11, p01, color);
    appendLine(vertices, p01, p00, color);
}

void appendScalePlaneHandle(QVector<RenderVertex>& vertices,
    const QVector3D& origin,
    const QVector3D& axisA,
    const QVector3D& axisB,
    const QVector3D& color,
    float size)
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

void appendScaleCenterHandle(QVector<RenderVertex>& vertices,
    const QVector3D& origin,
    const QVector3D& cameraForward,
    const QVector3D& color,
    float size)
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

namespace ViewportOverlayGeometryBuilder
{
QVector<RenderVertex> buildSelectionBounds(const Bounds3D& bounds)
{
    QVector<RenderVertex> vertices;
    if (!bounds.isValid()) {
        return vertices;
    }

    const QVector3D minPoint = bounds.min();
    const QVector3D maxPoint = bounds.max();

    const QVector3D p000(minPoint.x(), minPoint.y(), minPoint.z());
    const QVector3D p100(maxPoint.x(), minPoint.y(), minPoint.z());
    const QVector3D p010(minPoint.x(), maxPoint.y(), minPoint.z());
    const QVector3D p110(maxPoint.x(), maxPoint.y(), minPoint.z());
    const QVector3D p001(minPoint.x(), minPoint.y(), maxPoint.z());
    const QVector3D p101(maxPoint.x(), minPoint.y(), maxPoint.z());
    const QVector3D p011(minPoint.x(), maxPoint.y(), maxPoint.z());
    const QVector3D p111(maxPoint.x(), maxPoint.y(), maxPoint.z());

    const QVector3D color(1.0f, 0.82f, 0.24f);

    appendLine(vertices, p000, p100, color);
    appendLine(vertices, p100, p110, color);
    appendLine(vertices, p110, p010, color);
    appendLine(vertices, p010, p000, color);

    appendLine(vertices, p001, p101, color);
    appendLine(vertices, p101, p111, color);
    appendLine(vertices, p111, p011, color);
    appendLine(vertices, p011, p001, color);

    appendLine(vertices, p000, p001, color);
    appendLine(vertices, p100, p101, color);
    appendLine(vertices, p110, p111, color);
    appendLine(vertices, p010, p011, color);
    return vertices;
}

QVector<RenderVertex> buildGizmo(
    const QVector3D& origin,
    float size,
    ViewportRenderer::GizmoMode mode,
    const QVector<QVector3D>& axes,
    const QVector3D& cameraForward,
    int activeAxis)
{
    QVector<RenderVertex> vertices;
    if (axes.size() < 3) {
        return vertices;
    }

    const QVector3D xColor = axisColor(0, activeAxis == 0);
    const QVector3D yColor = axisColor(1, activeAxis == 1);
    const QVector3D zColor = axisColor(2, activeAxis == 2);

    if (mode == ViewportRenderer::GizmoMode::Translate) {
        TranslateGizmoGeometry::appendGizmo(vertices, origin, size, axes, cameraForward, xColor, yColor, zColor, activeAxis);
    } else if (mode == ViewportRenderer::GizmoMode::Rotate) {
        RotateGizmoGeometry::appendGizmo(vertices, origin, size, axes, cameraForward, xColor, yColor, zColor, activeAxis);
    } else {
        ScaleGizmoGeometry::appendGizmo(vertices, origin, size, axes, cameraForward, xColor, yColor, zColor, activeAxis);
    }

    return vertices;
}
}
