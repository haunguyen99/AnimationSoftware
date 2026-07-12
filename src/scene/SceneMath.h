#pragma once

#include <QMatrix4x4>
#include <QVector3D>

#include "scene/Bounds3D.h"
#include "scene/Transform.h"

namespace SceneMath
{
QMatrix4x4 composeMatrix(const Transform& transform);
Bounds3D transformBounds(const Bounds3D& bounds, const QMatrix4x4& matrix);
QVector3D transformDirection(const QMatrix4x4& matrix, const QVector3D& direction);
}
