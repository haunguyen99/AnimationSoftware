#include "scene/SceneMath.h"

namespace SceneMath
{
QMatrix4x4 composeMatrix(const Transform& transform)
{
    QMatrix4x4 matrix;
    matrix.translate(transform.translation);
    matrix.rotate(transform.rotation);
    matrix.scale(transform.scale);
    return matrix;
}

Bounds3D transformBounds(const Bounds3D& bounds, const QMatrix4x4& matrix)
{
    if (!bounds.isValid()) {
        return bounds;
    }

    const QVector3D minPoint = bounds.min();
    const QVector3D maxPoint = bounds.max();
    const QVector<QVector3D> corners = {
        QVector3D(minPoint.x(), minPoint.y(), minPoint.z()),
        QVector3D(maxPoint.x(), minPoint.y(), minPoint.z()),
        QVector3D(minPoint.x(), maxPoint.y(), minPoint.z()),
        QVector3D(maxPoint.x(), maxPoint.y(), minPoint.z()),
        QVector3D(minPoint.x(), minPoint.y(), maxPoint.z()),
        QVector3D(maxPoint.x(), minPoint.y(), maxPoint.z()),
        QVector3D(minPoint.x(), maxPoint.y(), maxPoint.z()),
        QVector3D(maxPoint.x(), maxPoint.y(), maxPoint.z())
    };

    Bounds3D transformed;
    for (const QVector3D& corner : corners) {
        transformed.expandToInclude(matrix * corner);
    }

    return transformed;
}

QVector3D transformDirection(const QMatrix4x4& matrix, const QVector3D& direction)
{
    return matrix.mapVector(direction).normalized();
}
}
