#include "scene/PrimitiveMeshFactory.h"

#include <QtMath>

namespace
{
constexpr float kPi = 3.14159265358979323846f;

void appendTriangle(MeshData& mesh,
    const QVector3D& a,
    const QVector3D& b,
    const QVector3D& c,
    const QVector3D& color)
{
    const QVector3D normal = QVector3D::crossProduct(b - a, c - a).normalized();
    const std::uint32_t baseIndex = static_cast<std::uint32_t>(mesh.positions.size());

    mesh.positions.append(a);
    mesh.positions.append(b);
    mesh.positions.append(c);
    mesh.normals.append(normal);
    mesh.normals.append(normal);
    mesh.normals.append(normal);
    mesh.colors.append(color);
    mesh.colors.append(color);
    mesh.colors.append(color);
    mesh.indices.append(baseIndex);
    mesh.indices.append(baseIndex + 1);
    mesh.indices.append(baseIndex + 2);

    mesh.bounds.expandToInclude(a);
    mesh.bounds.expandToInclude(b);
    mesh.bounds.expandToInclude(c);
}

void appendTriangleWithNormals(MeshData& mesh,
    const QVector3D& a,
    const QVector3D& b,
    const QVector3D& c,
    const QVector3D& normalA,
    const QVector3D& normalB,
    const QVector3D& normalC,
    const QVector3D& color)
{
    const std::uint32_t baseIndex = static_cast<std::uint32_t>(mesh.positions.size());

    mesh.positions.append(a);
    mesh.positions.append(b);
    mesh.positions.append(c);
    mesh.normals.append(normalA.normalized());
    mesh.normals.append(normalB.normalized());
    mesh.normals.append(normalC.normalized());
    mesh.colors.append(color);
    mesh.colors.append(color);
    mesh.colors.append(color);
    mesh.indices.append(baseIndex);
    mesh.indices.append(baseIndex + 1);
    mesh.indices.append(baseIndex + 2);

    mesh.bounds.expandToInclude(a);
    mesh.bounds.expandToInclude(b);
    mesh.bounds.expandToInclude(c);
}

void appendQuad(MeshData& mesh,
    const QVector3D& a,
    const QVector3D& b,
    const QVector3D& c,
    const QVector3D& d,
    const QVector3D& color)
{
    appendTriangle(mesh, a, b, c, color);
    appendTriangle(mesh, a, c, d, color);
}

void appendQuadWithNormals(MeshData& mesh,
    const QVector3D& a,
    const QVector3D& b,
    const QVector3D& c,
    const QVector3D& d,
    const QVector3D& normalA,
    const QVector3D& normalB,
    const QVector3D& normalC,
    const QVector3D& normalD,
    const QVector3D& color)
{
    appendTriangleWithNormals(mesh, a, b, c, normalA, normalB, normalC, color);
    appendTriangleWithNormals(mesh, a, c, d, normalA, normalC, normalD, color);
}

MeshData createCube()
{
    MeshData mesh;
    const QVector3D color(0.85f, 0.58f, 0.26f);
    const float h = 0.5f;

    const QVector3D p000(-h, -h, -h);
    const QVector3D p001(-h, -h, h);
    const QVector3D p010(-h, h, -h);
    const QVector3D p011(-h, h, h);
    const QVector3D p100(h, -h, -h);
    const QVector3D p101(h, -h, h);
    const QVector3D p110(h, h, -h);
    const QVector3D p111(h, h, h);

    appendQuad(mesh, p001, p101, p111, p011, color);
    appendQuad(mesh, p100, p000, p010, p110, color);
    appendQuad(mesh, p000, p001, p011, p010, color);
    appendQuad(mesh, p101, p100, p110, p111, color);
    appendQuad(mesh, p010, p011, p111, p110, color);
    appendQuad(mesh, p000, p100, p101, p001, color);
    return mesh;
}

MeshData createPlane()
{
    MeshData mesh;
    const QVector3D color(0.78f, 0.68f, 0.32f);
    appendQuad(mesh,
        QVector3D(-0.5f, 0.0f, -0.5f),
        QVector3D(0.5f, 0.0f, -0.5f),
        QVector3D(0.5f, 0.0f, 0.5f),
        QVector3D(-0.5f, 0.0f, 0.5f),
        color);
    return mesh;
}

MeshData createDisc()
{
    MeshData mesh;
    const QVector3D color(0.84f, 0.65f, 0.30f);
    const QVector3D center(0.0f, 0.0f, 0.0f);
    constexpr int segments = 32;

    for (int segment = 0; segment < segments; ++segment) {
        const float angleA = (static_cast<float>(segment) / segments) * (kPi * 2.0f);
        const float angleB = (static_cast<float>(segment + 1) / segments) * (kPi * 2.0f);
        const QVector3D a(qCos(angleA) * 0.5f, 0.0f, qSin(angleA) * 0.5f);
        const QVector3D b(qCos(angleB) * 0.5f, 0.0f, qSin(angleB) * 0.5f);
        appendTriangle(mesh, center, b, a, color);
    }

    return mesh;
}

MeshData createCylinderLike(bool cone)
{
    MeshData mesh;
    const QVector3D color(cone ? QVector3D(0.88f, 0.57f, 0.25f) : QVector3D(0.82f, 0.55f, 0.22f));
    constexpr int segments = 24;
    const float radius = 0.5f;
    const float halfHeight = 0.5f;
    const QVector3D topCenter(0.0f, halfHeight, 0.0f);
    const QVector3D bottomCenter(0.0f, -halfHeight, 0.0f);

    for (int segment = 0; segment < segments; ++segment) {
        const float angleA = (static_cast<float>(segment) / segments) * (kPi * 2.0f);
        const float angleB = (static_cast<float>(segment + 1) / segments) * (kPi * 2.0f);

        const QVector3D bottomA(qCos(angleA) * radius, -halfHeight, qSin(angleA) * radius);
        const QVector3D bottomB(qCos(angleB) * radius, -halfHeight, qSin(angleB) * radius);
        const QVector3D topA = cone ? topCenter : QVector3D(qCos(angleA) * radius, halfHeight, qSin(angleA) * radius);
        const QVector3D topB = cone ? topCenter : QVector3D(qCos(angleB) * radius, halfHeight, qSin(angleB) * radius);

        if (cone) {
            const QVector3D sideNormalA(qCos(angleA), radius / (halfHeight * 2.0f), qSin(angleA));
            const QVector3D sideNormalB(qCos(angleB), radius / (halfHeight * 2.0f), qSin(angleB));
            const QVector3D tipNormal = (sideNormalA + sideNormalB).normalized();
            appendTriangleWithNormals(mesh, bottomA, bottomB, topCenter, sideNormalA, sideNormalB, tipNormal, color);
        } else {
            const QVector3D sideNormalA(qCos(angleA), 0.0f, qSin(angleA));
            const QVector3D sideNormalB(qCos(angleB), 0.0f, qSin(angleB));
            appendQuadWithNormals(mesh, bottomA, bottomB, topB, topA, sideNormalA, sideNormalB, sideNormalB, sideNormalA, color);
            appendTriangle(mesh, topCenter, topA, topB, color);
        }

        appendTriangle(mesh, bottomCenter, bottomB, bottomA, color);
    }

    return mesh;
}

MeshData createSphere()
{
    MeshData mesh;
    const QVector3D color(0.89f, 0.60f, 0.28f);
    constexpr int rings = 16;
    constexpr int segments = 24;
    const float radius = 0.5f;

    for (int ring = 0; ring < rings; ++ring) {
        const float v0 = static_cast<float>(ring) / rings;
        const float v1 = static_cast<float>(ring + 1) / rings;
        const float phi0 = (v0 - 0.5f) * kPi;
        const float phi1 = (v1 - 0.5f) * kPi;

        for (int segment = 0; segment < segments; ++segment) {
            const float u0 = static_cast<float>(segment) / segments;
            const float u1 = static_cast<float>(segment + 1) / segments;
            const float theta0 = u0 * kPi * 2.0f;
            const float theta1 = u1 * kPi * 2.0f;

            const QVector3D p00(qCos(phi0) * qCos(theta0) * radius, qSin(phi0) * radius, qCos(phi0) * qSin(theta0) * radius);
            const QVector3D p01(qCos(phi0) * qCos(theta1) * radius, qSin(phi0) * radius, qCos(phi0) * qSin(theta1) * radius);
            const QVector3D p10(qCos(phi1) * qCos(theta0) * radius, qSin(phi1) * radius, qCos(phi1) * qSin(theta0) * radius);
            const QVector3D p11(qCos(phi1) * qCos(theta1) * radius, qSin(phi1) * radius, qCos(phi1) * qSin(theta1) * radius);

            appendQuadWithNormals(mesh,
                p00, p01, p11, p10,
                p00.normalized(), p01.normalized(), p11.normalized(), p10.normalized(),
                color);
        }
    }

    return mesh;
}

MeshData createTorus()
{
    MeshData mesh;
    const QVector3D color(0.87f, 0.62f, 0.28f);
    constexpr int majorSegments = 24;
    constexpr int minorSegments = 12;
    const float majorRadius = 0.55f;
    const float minorRadius = 0.18f;

    for (int major = 0; major < majorSegments; ++major) {
        const float u0 = (static_cast<float>(major) / majorSegments) * (kPi * 2.0f);
        const float u1 = (static_cast<float>(major + 1) / majorSegments) * (kPi * 2.0f);

        for (int minor = 0; minor < minorSegments; ++minor) {
            const float v0 = (static_cast<float>(minor) / minorSegments) * (kPi * 2.0f);
            const float v1 = (static_cast<float>(minor + 1) / minorSegments) * (kPi * 2.0f);

            auto pointAt = [&](float u, float v) {
                const float ringRadius = majorRadius + qCos(v) * minorRadius;
                return QVector3D(qCos(u) * ringRadius, qSin(v) * minorRadius, qSin(u) * ringRadius);
            };

            auto normalAt = [&](float u, float v) {
                return QVector3D(qCos(u) * qCos(v), qSin(v), qSin(u) * qCos(v)).normalized();
            };

            appendQuadWithNormals(mesh,
                pointAt(u0, v0), pointAt(u1, v0), pointAt(u1, v1), pointAt(u0, v1),
                normalAt(u0, v0), normalAt(u1, v0), normalAt(u1, v1), normalAt(u0, v1),
                color);
        }
    }

    return mesh;
}

MeshData createPyramid()
{
    MeshData mesh;
    const QVector3D color(0.86f, 0.56f, 0.24f);
    const QVector3D top(0.0f, 0.5f, 0.0f);
    const QVector3D a(-0.5f, -0.5f, -0.5f);
    const QVector3D b(0.5f, -0.5f, -0.5f);
    const QVector3D c(0.5f, -0.5f, 0.5f);
    const QVector3D d(-0.5f, -0.5f, 0.5f);
    appendTriangle(mesh, a, b, top, color);
    appendTriangle(mesh, b, c, top, color);
    appendTriangle(mesh, c, d, top, color);
    appendTriangle(mesh, d, a, top, color);
    appendQuad(mesh, a, d, c, b, color);
    return mesh;
}

MeshData createPrism()
{
    MeshData mesh;
    const QVector3D color(0.84f, 0.54f, 0.20f);
    const QVector3D topA(-0.5f, 0.5f, -0.35f);
    const QVector3D topB(0.5f, 0.5f, -0.35f);
    const QVector3D topC(0.0f, 0.5f, 0.5f);
    const QVector3D bottomA(-0.5f, -0.5f, -0.35f);
    const QVector3D bottomB(0.5f, -0.5f, -0.35f);
    const QVector3D bottomC(0.0f, -0.5f, 0.5f);
    appendTriangle(mesh, topA, topB, topC, color);
    appendTriangle(mesh, bottomA, bottomC, bottomB, color);
    appendQuad(mesh, bottomA, bottomB, topB, topA, color);
    appendQuad(mesh, bottomB, bottomC, topC, topB, color);
    appendQuad(mesh, bottomC, bottomA, topA, topC, color);
    return mesh;
}
}

namespace PrimitiveMeshFactory
{
QString displayName(Type type)
{
    switch (type) {
    case Type::Sphere: return "Sphere";
    case Type::Cube: return "Cube";
    case Type::Cylinder: return "Cylinder";
    case Type::Cone: return "Cone";
    case Type::Torus: return "Torus";
    case Type::Plane: return "Plane";
    case Type::Disc: return "Disc";
    case Type::Pyramid: return "Pyramid";
    case Type::Prism: return "Prism";
    }

    return "Primitive";
}

bool isImplemented(Type type)
{
    switch (type) {
    case Type::Sphere:
    case Type::Cube:
    case Type::Cylinder:
    case Type::Cone:
    case Type::Torus:
    case Type::Plane:
    case Type::Disc:
    case Type::Pyramid:
    case Type::Prism:
        return true;
    }

    return false;
}

MeshData createMesh(Type type)
{
    switch (type) {
    case Type::Sphere: return createSphere();
    case Type::Cube: return createCube();
    case Type::Cylinder: return createCylinderLike(false);
    case Type::Cone: return createCylinderLike(true);
    case Type::Torus: return createTorus();
    case Type::Plane: return createPlane();
    case Type::Disc: return createDisc();
    case Type::Pyramid: return createPyramid();
    case Type::Prism: return createPrism();
    }

    return {};
}
}
