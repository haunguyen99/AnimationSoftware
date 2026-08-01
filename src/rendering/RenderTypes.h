#pragma once

#include <QVector>
#include <QVector3D>

#include <cstdint>

struct RenderVertex
{
    QVector3D position;
    QVector3D normal;
    QVector3D color;
};
