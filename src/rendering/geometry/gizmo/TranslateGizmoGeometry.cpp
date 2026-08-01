#include "rendering/geometry/gizmo/TranslateGizmoGeometry.h"

namespace
{
constexpr float kTranslateShaftRatio = 0.78f;
constexpr float kTranslateHeadLengthRatio = 0.22f;
constexpr float kTranslateHeadWidthRatio = 0.10f;
constexpr float kTranslatePlaneStartRatio = 0.22f;
constexpr float kTranslatePlaneEndRatio = 0.42f;
constexpr float kTranslateCenterExtentRatio = 0.10f;

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

void appendTranslateAxis(QVector<RenderVertex>& vertices, const QVector3D& origin, const QVector3D& axis, const QVector3D& color, float size)
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
    const QVector3D bitangent = QVector3D::crossProduct(normalizedAxis, tangent).normalized();

    const QVector3D back = normalizedAxis * (size * kTranslateHeadLengthRatio);
    const QVector3D sideA = tangent * size * kTranslateHeadWidthRatio;
    const QVector3D sideB = bitangent * size * kTranslateHeadWidthRatio;

    appendLine(vertices, tip, tip - back + sideA, color);
    appendLine(vertices, tip, tip - back - sideA, color);
    appendLine(vertices, tip, tip - back + sideB, color);
    appendLine(vertices, tip, tip - back - sideB, color);
}

void appendPlaneHandle(QVector<RenderVertex>& vertices, const QVector3D& origin, const QVector3D& axisA, const QVector3D& axisB, const QVector3D& color, float size)
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

void appendViewPlaneHandle(QVector<RenderVertex>& vertices, const QVector3D& origin, const QVector3D& cameraForward, const QVector3D& color, float size)
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
}

namespace TranslateGizmoGeometry
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
    appendTranslateAxis(vertices, origin, axes[0], xColor, size);
    appendTranslateAxis(vertices, origin, axes[1], yColor, size);
    appendTranslateAxis(vertices, origin, axes[2], zColor, size);
    appendPlaneHandle(vertices, origin, axes[0], axes[1], planeColor(xColor, yColor, activeAxis == 3), size);
    appendPlaneHandle(vertices, origin, axes[1], axes[2], planeColor(yColor, zColor, activeAxis == 4), size);
    appendPlaneHandle(vertices, origin, axes[0], axes[2], planeColor(xColor, zColor, activeAxis == 5), size);
    appendViewPlaneHandle(vertices, origin, -cameraForward, activeAxis == 6 ? QVector3D(0.98f, 0.82f, 0.24f) : QVector3D(0.92f, 0.92f, 0.92f), size);
}
}
