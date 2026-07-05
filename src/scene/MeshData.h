#pragma once

#include <QVector>
#include <QVector3D>

#include <cstdint>

#include "scene/Bounds3D.h"

struct MeshData
{
    QVector<QVector3D> positions;
    QVector<QVector3D> normals;
    QVector<QVector3D> colors;
    QVector<std::uint32_t> indices;
    Bounds3D bounds;
};
