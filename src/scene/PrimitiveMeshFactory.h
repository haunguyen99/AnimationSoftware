#pragma once

#include <QString>

#include "scene/MeshData.h"

namespace PrimitiveMeshFactory
{
enum class Type
{
    Sphere,
    Cube,
    Cylinder,
    Cone,
    Torus,
    Plane,
    Disc,
    Pyramid,
    Prism
};

QString displayName(Type type);
bool isImplemented(Type type);
MeshData createMesh(Type type);
}
