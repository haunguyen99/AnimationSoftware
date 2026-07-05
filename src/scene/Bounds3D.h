#pragma once

#include <QVector3D>

class Bounds3D
{
public:
    Bounds3D();

    static Bounds3D fromMinMax(const QVector3D& minPoint, const QVector3D& maxPoint);

    bool isValid() const;
    void reset();
    void expandToInclude(const QVector3D& point);
    void expandToInclude(const Bounds3D& other);

    QVector3D min() const;
    QVector3D max() const;
    QVector3D center() const;
    QVector3D extents() const;
    float radius() const;

private:
    QVector3D minPoint_;
    QVector3D maxPoint_;
    bool valid_ = false;
};

