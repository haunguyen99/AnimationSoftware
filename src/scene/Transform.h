#pragma once

#include <QQuaternion>
#include <QVector3D>

struct Transform
{
    QVector3D translation { 0.0f, 0.0f, 0.0f };
    QQuaternion rotation;
    QVector3D scale { 1.0f, 1.0f, 1.0f };
};

