#include "scene/Bounds3D.h"

#include <QtMath>

Bounds3D::Bounds3D()
{
    reset();
}

Bounds3D Bounds3D::fromMinMax(const QVector3D& minPoint, const QVector3D& maxPoint)
{
    Bounds3D bounds;
    bounds.minPoint_ = minPoint;
    bounds.maxPoint_ = maxPoint;
    bounds.valid_ = true;
    return bounds;
}

bool Bounds3D::isValid() const
{
    return valid_;
}

void Bounds3D::reset()
{
    minPoint_ = QVector3D();
    maxPoint_ = QVector3D();
    valid_ = false;
}

void Bounds3D::expandToInclude(const QVector3D& point)
{
    if (!valid_) {
        minPoint_ = point;
        maxPoint_ = point;
        valid_ = true;
        return;
    }

    minPoint_.setX(qMin(minPoint_.x(), point.x()));
    minPoint_.setY(qMin(minPoint_.y(), point.y()));
    minPoint_.setZ(qMin(minPoint_.z(), point.z()));
    maxPoint_.setX(qMax(maxPoint_.x(), point.x()));
    maxPoint_.setY(qMax(maxPoint_.y(), point.y()));
    maxPoint_.setZ(qMax(maxPoint_.z(), point.z()));
}

void Bounds3D::expandToInclude(const Bounds3D& other)
{
    if (!other.isValid()) {
        return;
    }

    expandToInclude(other.min());
    expandToInclude(other.max());
}

QVector3D Bounds3D::min() const
{
    return minPoint_;
}

QVector3D Bounds3D::max() const
{
    return maxPoint_;
}

QVector3D Bounds3D::center() const
{
    return (minPoint_ + maxPoint_) * 0.5f;
}

QVector3D Bounds3D::extents() const
{
    return (maxPoint_ - minPoint_) * 0.5f;
}

float Bounds3D::radius() const
{
    return extents().length();
}

