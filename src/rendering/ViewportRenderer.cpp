#include "rendering/ViewportRenderer.h"

#include "logging/LogCategories.h"
#include "rendering/ShaderUtils.h"
#include "scene/Scene.h"
#include "scene/SceneMath.h"
#include "viewport/EditorCamera.h"

#include <QDebug>
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>

namespace
{
constexpr int kGizmoArcSegments = 48;
constexpr float kBasisParallelThreshold = 0.95f;

QString vertexShaderSource()
{
    return R"(
        #version 330 core
        layout(location = 0) in vec3 inPosition;
        layout(location = 1) in vec3 inNormal;
        layout(location = 2) in vec3 inColor;

        uniform mat4 uMvp;
        uniform mat4 uView;
        uniform bool uUseLighting;

        out vec3 vColor;
        out float vLighting;

        void main()
        {
            vColor = inColor;

            vec3 normal = normalize(mat3(uView) * inNormal);
            vec3 lightDir = vec3(0.0, 0.0, 1.0);
            float diffuse = max(dot(normal, lightDir), 0.0);
            vLighting = uUseLighting ? (0.25 + diffuse * 0.75) : 1.0;

            gl_Position = uMvp * vec4(inPosition, 1.0);
        }
    )";
}

QString fragmentShaderSource()
{
    return R"(
        #version 330 core
        in vec3 vColor;
        in float vLighting;

        out vec4 fragColor;

        void main()
        {
            fragColor = vec4(vColor * vLighting, 1.0);
        }
    )";
}

void appendLine(QVector<ViewportRenderer::Vertex>& vertices,
    const QVector3D& a,
    const QVector3D& b,
    const QVector3D& color)
{
    const QVector3D normal(0.0f, 1.0f, 0.0f);
    vertices.append({ a, normal, color });
    vertices.append({ b, normal, color });
}

void appendTranslateAxis(QVector<ViewportRenderer::Vertex>& vertices,
    const QVector3D& origin,
    const QVector3D& axis,
    const QVector3D& color,
    float size)
{
    const QVector3D normalizedAxis = axis.normalized();
    const QVector3D tip = origin + normalizedAxis * size;
    appendLine(vertices, origin, tip, color);

    QVector3D tangent = QVector3D::crossProduct(normalizedAxis, QVector3D(0.0f, 1.0f, 0.0f));
    if (tangent.lengthSquared() < 0.0001f) {
        tangent = QVector3D::crossProduct(normalizedAxis, QVector3D(1.0f, 0.0f, 0.0f));
    }
    tangent.normalize();
    QVector3D bitangent = QVector3D::crossProduct(normalizedAxis, tangent).normalized();

    const QVector3D back = normalizedAxis * (size * 0.22f);
    const QVector3D sideA = tangent * size * 0.18f;
    const QVector3D sideB = bitangent * size * 0.18f;
    appendLine(vertices, tip, tip - back + sideA, color);
    appendLine(vertices, tip, tip - back - sideA, color);
    appendLine(vertices, tip, tip - back + sideB, color);
    appendLine(vertices, tip, tip - back - sideB, color);
}

void appendScaleAxis(QVector<ViewportRenderer::Vertex>& vertices,
    const QVector3D& origin,
    const QVector3D& axis,
    const QVector3D& color,
    float size)
{
    const QVector3D normalizedAxis = axis.normalized();
    const QVector3D handleCenter = origin + normalizedAxis * size;
    appendLine(vertices, origin, handleCenter, color);

    QVector3D tangent = QVector3D::crossProduct(normalizedAxis, QVector3D(0.0f, 1.0f, 0.0f));
    if (tangent.lengthSquared() < 0.0001f) {
        tangent = QVector3D::crossProduct(normalizedAxis, QVector3D(1.0f, 0.0f, 0.0f));
    }
    tangent.normalize();
    QVector3D bitangent = QVector3D::crossProduct(normalizedAxis, tangent).normalized();

    const QVector3D sideA = tangent * size * 0.12f;
    const QVector3D sideB = bitangent * size * 0.12f;

    const QVector3D p1 = handleCenter - sideA - sideB;
    const QVector3D p2 = handleCenter + sideA - sideB;
    const QVector3D p3 = handleCenter + sideA + sideB;
    const QVector3D p4 = handleCenter - sideA + sideB;

    appendLine(vertices, p1, p2, color);
    appendLine(vertices, p2, p3, color);
    appendLine(vertices, p3, p4, color);
    appendLine(vertices, p4, p1, color);
}

void appendRotateRing(QVector<ViewportRenderer::Vertex>& vertices,
    const QVector3D& origin,
    const QVector3D& axis,
    const QVector3D& color,
    float size)
{
    const QVector3D normalAxis = axis.normalized();
    QVector3D basisA = QVector3D::crossProduct(normalAxis, QVector3D(0.0f, 1.0f, 0.0f));
    if (basisA.lengthSquared() < 0.0001f) {
        basisA = QVector3D::crossProduct(normalAxis, QVector3D(1.0f, 0.0f, 0.0f));
    }
    basisA.normalize();
    const QVector3D basisB = QVector3D::crossProduct(normalAxis, basisA).normalized();

    for (int segment = 0; segment < kGizmoArcSegments; ++segment) {
        const float angleA = (static_cast<float>(segment) / kGizmoArcSegments) * 360.0f;
        const float angleB = (static_cast<float>(segment + 1) / kGizmoArcSegments) * 360.0f;

        const float radiansA = qDegreesToRadians(angleA);
        const float radiansB = qDegreesToRadians(angleB);

        const QVector3D pointA = origin + (basisA * qCos(radiansA) + basisB * qSin(radiansA)) * size;
        const QVector3D pointB = origin + (basisA * qCos(radiansB) + basisB * qSin(radiansB)) * size;

        appendLine(vertices, pointA, pointB, color);
    }
}
}

ViewportRenderer::ViewportRenderer()
    : vertexBuffer_(QOpenGLBuffer::VertexBuffer)
    , importedVertexBuffer_(QOpenGLBuffer::VertexBuffer)
    , importedIndexBuffer_(QOpenGLBuffer::IndexBuffer)
    , selectionVertexBuffer_(QOpenGLBuffer::VertexBuffer)
    , gizmoVertexBuffer_(QOpenGLBuffer::VertexBuffer)
{
}

ViewportRenderer::~ViewportRenderer()
{
    if (vertexBuffer_.isCreated()) {
        vertexBuffer_.destroy();
    }

    if (importedVertexBuffer_.isCreated()) {
        importedVertexBuffer_.destroy();
    }

    if (importedIndexBuffer_.isCreated()) {
        importedIndexBuffer_.destroy();
    }

    if (selectionVertexBuffer_.isCreated()) {
        selectionVertexBuffer_.destroy();
    }

    if (gizmoVertexBuffer_.isCreated()) {
        gizmoVertexBuffer_.destroy();
    }
}

bool ViewportRenderer::initialize(QOpenGLFunctions_3_3_Core* functions)
{
    functions_ = functions;
    if (functions_ == nullptr) {
        qCWarning(logViewport) << "OpenGL functions pointer is null.";
        return false;
    }

    shaderProgram_ = ShaderUtils::buildProgram(vertexShaderSource(), fragmentShaderSource());
    if (shaderProgram_ == nullptr) {
        qCWarning(logViewport) << "failed to build shader program.";
        return false;
    }

    createSceneGeometry();

    if (!vao_.create()) {
        qCWarning(logViewport) << "failed to create grid/axis VAO.";
        return false;
    }
    vao_.bind();

    if (!vertexBuffer_.create()) {
        qCWarning(logViewport) << "failed to create grid/axis vertex buffer.";
        vao_.release();
        return false;
    }
    vertexBuffer_.bind();
    uploadGeometry();

    if (shaderProgram_) {
        shaderProgram_->bind();
        shaderProgram_->enableAttributeArray(0);
        shaderProgram_->setAttributeBuffer(0, GL_FLOAT, offsetof(Vertex, position), 3, sizeof(Vertex));
        shaderProgram_->enableAttributeArray(1);
        shaderProgram_->setAttributeBuffer(1, GL_FLOAT, offsetof(Vertex, normal), 3, sizeof(Vertex));
        shaderProgram_->enableAttributeArray(2);
        shaderProgram_->setAttributeBuffer(2, GL_FLOAT, offsetof(Vertex, color), 3, sizeof(Vertex));
        shaderProgram_->release();
    }

    vertexBuffer_.release();
    vao_.release();

    if (!importedVao_.create()) {
        qCWarning(logViewport) << "failed to create imported mesh VAO.";
        return false;
    }
    importedVao_.bind();
    if (!importedVertexBuffer_.create()) {
        qCWarning(logViewport) << "failed to create imported mesh vertex buffer.";
        importedVao_.release();
        return false;
    }
    if (!importedIndexBuffer_.create()) {
        qCWarning(logViewport) << "failed to create imported mesh index buffer.";
        importedVao_.release();
        return false;
    }
    importedVao_.release();

    if (!selectionVao_.create()) {
        qCWarning(logViewport) << "failed to create selection VAO.";
        return false;
    }
    selectionVao_.bind();
    if (!selectionVertexBuffer_.create()) {
        qCWarning(logViewport) << "failed to create selection vertex buffer.";
        selectionVao_.release();
        return false;
    }
    selectionVao_.release();

    if (!gizmoVao_.create()) {
        qCWarning(logViewport) << "failed to create gizmo VAO.";
        return false;
    }
    gizmoVao_.bind();
    if (!gizmoVertexBuffer_.create()) {
        qCWarning(logViewport) << "failed to create gizmo vertex buffer.";
        gizmoVao_.release();
        return false;
    }
    gizmoVao_.release();

    ShaderUtils::logOpenGlError(functions_, "ViewportRenderer::initialize");
    return true;
}

void ViewportRenderer::resize(int width, int height)
{
    viewportWidth_ = width;
    viewportHeight_ = height;

    if (functions_ != nullptr) {
        functions_->glViewport(0, 0, width, height);
    }
}

void ViewportRenderer::render(const EditorCamera& camera, const ViewportRenderOptions& options)
{
    if (functions_ == nullptr || shaderProgram_ == nullptr) {
        return;
    }

    functions_->glViewport(0, 0, viewportWidth_, viewportHeight_);
    functions_->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (options.backfaceCulling) {
        functions_->glEnable(GL_CULL_FACE);
        functions_->glCullFace(GL_BACK);
        functions_->glFrontFace(GL_CCW);
    } else {
        functions_->glDisable(GL_CULL_FACE);
    }

    const QMatrix4x4 viewMatrix = camera.viewMatrix();
    const QMatrix4x4 meshMvp = camera.projectionMatrix() * viewMatrix;
    const QMatrix4x4 gridMvp = camera.projectionMatrix() * viewMatrix;

    shaderProgram_->bind();
    shaderProgram_->setUniformValue("uView", viewMatrix);

    if (!importedVertices_.isEmpty() && !importedIndices_.isEmpty()) {
        shaderProgram_->setUniformValue("uMvp", meshMvp);
        shaderProgram_->setUniformValue("uUseLighting", true);
        importedVao_.bind();
        importedIndexBuffer_.bind();
        functions_->glDrawElements(GL_TRIANGLES, importedIndices_.size(), GL_UNSIGNED_INT, nullptr);

        if (options.wireframe) {
            functions_->glDisable(GL_CULL_FACE);
            functions_->glEnable(GL_POLYGON_OFFSET_LINE);
            functions_->glPolygonOffset(-1.0f, -1.0f);
            functions_->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            functions_->glLineWidth(1.0f);
            shaderProgram_->setUniformValue("uUseLighting", false);
            functions_->glDrawElements(GL_TRIANGLES, importedIndices_.size(), GL_UNSIGNED_INT, nullptr);
            functions_->glDisable(GL_POLYGON_OFFSET_LINE);
            functions_->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            if (options.backfaceCulling) {
                functions_->glEnable(GL_CULL_FACE);
                functions_->glCullFace(GL_BACK);
                functions_->glFrontFace(GL_CCW);
            }
        }

        importedIndexBuffer_.release();
        importedVao_.release();
    }

    if (!selectionVertices_.isEmpty()) {
        shaderProgram_->setUniformValue("uMvp", meshMvp);
        shaderProgram_->setUniformValue("uUseLighting", false);
        selectionVao_.bind();
        functions_->glDisable(GL_CULL_FACE);
        functions_->glDisable(GL_DEPTH_TEST);
        functions_->glDepthMask(GL_FALSE);
        functions_->glLineWidth(2.0f);
        functions_->glDrawArrays(GL_LINES, 0, selectionVertices_.size());
        functions_->glEnable(GL_DEPTH_TEST);
        selectionVao_.release();
    }

    if (!gizmoVertices_.isEmpty()) {
        shaderProgram_->setUniformValue("uMvp", meshMvp);
        shaderProgram_->setUniformValue("uUseLighting", false);
        gizmoVao_.bind();
        functions_->glDisable(GL_CULL_FACE);
        functions_->glEnable(GL_DEPTH_TEST);
        functions_->glDepthMask(GL_FALSE);
        functions_->glLineWidth(3.0f);
        functions_->glDrawArrays(GL_LINES, 0, gizmoVertices_.size());
        gizmoVao_.release();
    }

    vao_.bind();
    shaderProgram_->setUniformValue("uMvp", gridMvp);
    shaderProgram_->setUniformValue("uUseLighting", false);
    functions_->glDepthMask(GL_FALSE);
    functions_->glLineWidth(1.0f);
    functions_->glDrawArrays(GL_LINES, 0, gridVertices_.size());

    if (options.showAxis) {
        functions_->glLineWidth(1.0f);
        functions_->glDrawArrays(GL_LINES, gridVertices_.size(), axisVertices_.size());
    }

    vao_.release();

    functions_->glDepthMask(GL_TRUE);
    functions_->glDisable(GL_CULL_FACE);
    shaderProgram_->release();

    functions_->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    ShaderUtils::logOpenGlError(functions_, "ViewportRenderer::render");
}

void ViewportRenderer::createSceneGeometry()
{
    constexpr int extent = 20;
    const QVector3D upNormal(0.0f, 1.0f, 0.0f);
    const QVector3D majorLineColor(0.42f, 0.46f, 0.52f);
    const QVector3D minorLineColor(0.28f, 0.31f, 0.36f);

    for (int line = -extent; line <= extent; ++line) {
        const bool isMajor = (line % 5) == 0;
        const QVector3D color = isMajor ? majorLineColor : minorLineColor;

        gridVertices_.append({ QVector3D(static_cast<float>(line), 0.0f, static_cast<float>(-extent)), upNormal, color });
        gridVertices_.append({ QVector3D(static_cast<float>(line), 0.0f, static_cast<float>(extent)), upNormal, color });

        gridVertices_.append({ QVector3D(static_cast<float>(-extent), 0.0f, static_cast<float>(line)), upNormal, color });
        gridVertices_.append({ QVector3D(static_cast<float>(extent), 0.0f, static_cast<float>(line)), upNormal, color });
    }

    axisVertices_ = {
        { QVector3D(0.0f, 0.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector3D(0.95f, 0.30f, 0.30f) },
        { QVector3D(4.0f, 0.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector3D(0.95f, 0.30f, 0.30f) },
        { QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f), QVector3D(0.35f, 0.95f, 0.45f) },
        { QVector3D(0.0f, 4.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f), QVector3D(0.35f, 0.95f, 0.45f) },
        { QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 0.0f, 1.0f), QVector3D(0.35f, 0.55f, 1.00f) },
        { QVector3D(0.0f, 0.0f, 4.0f), QVector3D(0.0f, 0.0f, 1.0f), QVector3D(0.35f, 0.55f, 1.00f) }
    };
}

void ViewportRenderer::uploadGeometry()
{
    Q_ASSERT(vertexBuffer_.isCreated());
    Q_ASSERT(!gridVertices_.isEmpty());
    Q_ASSERT(!axisVertices_.isEmpty());

    QVector<Vertex> vertices;
    vertices.reserve(gridVertices_.size() + axisVertices_.size());

    vertices += gridVertices_;
    vertices += axisVertices_;

    vertexBuffer_.allocate(vertices.constData(), vertices.size() * sizeof(Vertex));
}

void ViewportRenderer::syncScene(const Scene& scene)
{
    Q_ASSERT(functions_ != nullptr);
    if (functions_ == nullptr) {
        return;
    }

    uploadImportedMesh(scene);
}

void ViewportRenderer::setSelectedBounds(const Bounds3D& bounds)
{
    selectionVertices_.clear();
    if (!bounds.isValid()) {
        uploadSelectionBounds();
        return;
    }

    const QVector3D minPoint = bounds.min();
    const QVector3D maxPoint = bounds.max();

    const QVector3D p000(minPoint.x(), minPoint.y(), minPoint.z());
    const QVector3D p100(maxPoint.x(), minPoint.y(), minPoint.z());
    const QVector3D p010(minPoint.x(), maxPoint.y(), minPoint.z());
    const QVector3D p110(maxPoint.x(), maxPoint.y(), minPoint.z());
    const QVector3D p001(minPoint.x(), minPoint.y(), maxPoint.z());
    const QVector3D p101(maxPoint.x(), minPoint.y(), maxPoint.z());
    const QVector3D p011(minPoint.x(), maxPoint.y(), maxPoint.z());
    const QVector3D p111(maxPoint.x(), maxPoint.y(), maxPoint.z());

    const QVector3D color(1.0f, 0.82f, 0.24f);
    const QVector3D normal(0.0f, 1.0f, 0.0f);

    auto addLine = [this, &color, &normal](const QVector3D& a, const QVector3D& b) {
        selectionVertices_.append({ a, normal, color });
        selectionVertices_.append({ b, normal, color });
    };

    addLine(p000, p100);
    addLine(p100, p110);
    addLine(p110, p010);
    addLine(p010, p000);

    addLine(p001, p101);
    addLine(p101, p111);
    addLine(p111, p011);
    addLine(p011, p001);

    addLine(p000, p001);
    addLine(p100, p101);
    addLine(p110, p111);
    addLine(p010, p011);

    uploadSelectionBounds();
}

void ViewportRenderer::setGizmo(const QVector3D& origin, float size, GizmoMode mode, const QVector<QVector3D>& axes, const QVector3D& cameraForward, int activeAxis)
{
    gizmoVertices_.clear();
    if (axes.size() < 3) {
        uploadGizmo();
        return;
    }

    const QVector3D xColor = activeAxis == 0 ? QVector3D(1.0f, 0.95f, 0.35f) : QVector3D(0.95f, 0.30f, 0.30f);
    const QVector3D yColor = activeAxis == 1 ? QVector3D(1.0f, 0.95f, 0.35f) : QVector3D(0.35f, 0.95f, 0.45f);
    const QVector3D zColor = activeAxis == 2 ? QVector3D(1.0f, 0.95f, 0.35f) : QVector3D(0.35f, 0.55f, 1.00f);

    if (mode == GizmoMode::Translate) {
        appendTranslateAxis(gizmoVertices_, origin, axes[0], xColor, size);
        appendTranslateAxis(gizmoVertices_, origin, axes[1], yColor, size);
        appendTranslateAxis(gizmoVertices_, origin, axes[2], zColor, size);
    } else if (mode == GizmoMode::Rotate) {
        appendRotateRing(gizmoVertices_, origin, axes[0], xColor, size * 0.9f);
        appendRotateRing(gizmoVertices_, origin, axes[1], yColor, size * 0.9f);
        appendRotateRing(gizmoVertices_, origin, axes[2], zColor, size * 0.9f);
        appendRotateRing(gizmoVertices_, origin, -cameraForward.normalized(), QVector3D(0.62f, 0.92f, 1.0f), size * 1.15f);
    } else {
        appendScaleAxis(gizmoVertices_, origin, axes[0], xColor, size);
        appendScaleAxis(gizmoVertices_, origin, axes[1], yColor, size);
        appendScaleAxis(gizmoVertices_, origin, axes[2], zColor, size);
    }

    uploadGizmo();
}

void ViewportRenderer::clearGizmo()
{
    gizmoVertices_.clear();
    uploadGizmo();
}

void ViewportRenderer::uploadImportedMesh(const Scene& scene)
{
    Q_ASSERT(importedVertexBuffer_.isCreated());
    Q_ASSERT(importedIndexBuffer_.isCreated());

    importedVertices_.clear();
    importedIndices_.clear();

    std::uint32_t vertexOffset = 0;
    const QVector<SceneObject::Id> objectIds = scene.allObjectIds();

    for (SceneObject::Id objectId : objectIds) {
        const SceneObject* object = scene.findObject(objectId);
        if (object == nullptr || !object->isVisible() || object->meshHandles().isEmpty()) {
            continue;
        }

        for (int meshHandle : object->meshHandles()) {
            const MeshData* mesh = scene.findMesh(meshHandle);
            if (mesh == nullptr) {
                Q_ASSERT_X(false, "ViewportRenderer::uploadImportedMesh", "SceneObject references missing mesh handle.");
                continue;
            }

            Q_ASSERT(mesh->positions.size() == mesh->normals.size());
            Q_ASSERT(mesh->positions.size() == mesh->colors.size());
            Q_ASSERT((mesh->indices.size() % 3) == 0);

            const QMatrix4x4 worldMatrix = scene.worldTransform(objectId);
            const QMatrix3x3 normalMatrix = worldMatrix.normalMatrix();
            const int vertexCount = mesh->positions.size();
            for (int vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex) {
                const QVector3D position = worldMatrix * mesh->positions[vertexIndex];
                const QVector3D sourceNormal = vertexIndex < mesh->normals.size()
                    ? mesh->normals[vertexIndex]
                    : QVector3D(0.0f, 1.0f, 0.0f);
                const QVector3D normal = QVector3D(
                    normalMatrix(0, 0) * sourceNormal.x() + normalMatrix(0, 1) * sourceNormal.y() + normalMatrix(0, 2) * sourceNormal.z(),
                    normalMatrix(1, 0) * sourceNormal.x() + normalMatrix(1, 1) * sourceNormal.y() + normalMatrix(1, 2) * sourceNormal.z(),
                    normalMatrix(2, 0) * sourceNormal.x() + normalMatrix(2, 1) * sourceNormal.y() + normalMatrix(2, 2) * sourceNormal.z()).normalized();
                const QVector3D color = vertexIndex < mesh->colors.size()
                    ? mesh->colors[vertexIndex]
                    : QVector3D(0.72f, 0.74f, 0.78f);

                importedVertices_.append({ position, normal, color });
            }

            for (std::uint32_t index : mesh->indices) {
                importedIndices_.append(vertexOffset + index);
            }

            vertexOffset += static_cast<std::uint32_t>(mesh->positions.size());
        }
    }

    if (!importedVertices_.isEmpty()) {
        importedVao_.bind();
        importedVertexBuffer_.bind();
        importedVertexBuffer_.allocate(importedVertices_.constData(), importedVertices_.size() * sizeof(Vertex));

        importedIndexBuffer_.bind();
        importedIndexBuffer_.allocate(importedIndices_.constData(), importedIndices_.size() * sizeof(std::uint32_t));

        if (shaderProgram_) {
            shaderProgram_->bind();
            shaderProgram_->enableAttributeArray(0);
            shaderProgram_->setAttributeBuffer(0, GL_FLOAT, offsetof(Vertex, position), 3, sizeof(Vertex));
            shaderProgram_->enableAttributeArray(1);
            shaderProgram_->setAttributeBuffer(1, GL_FLOAT, offsetof(Vertex, normal), 3, sizeof(Vertex));
            shaderProgram_->enableAttributeArray(2);
            shaderProgram_->setAttributeBuffer(2, GL_FLOAT, offsetof(Vertex, color), 3, sizeof(Vertex));
            shaderProgram_->release();
        }

        importedVertexBuffer_.release();
        importedIndexBuffer_.release();
        importedVao_.release();
    }
}

void ViewportRenderer::uploadSelectionBounds()
{
    if (!selectionVertexBuffer_.isCreated() || shaderProgram_ == nullptr) {
        return;
    }

    selectionVao_.bind();
    selectionVertexBuffer_.bind();
    selectionVertexBuffer_.allocate(
        selectionVertices_.isEmpty() ? nullptr : selectionVertices_.constData(),
        selectionVertices_.size() * sizeof(Vertex));

    if (shaderProgram_) {
        shaderProgram_->bind();
        shaderProgram_->enableAttributeArray(0);
        shaderProgram_->setAttributeBuffer(0, GL_FLOAT, offsetof(Vertex, position), 3, sizeof(Vertex));
        shaderProgram_->enableAttributeArray(1);
        shaderProgram_->setAttributeBuffer(1, GL_FLOAT, offsetof(Vertex, normal), 3, sizeof(Vertex));
        shaderProgram_->enableAttributeArray(2);
        shaderProgram_->setAttributeBuffer(2, GL_FLOAT, offsetof(Vertex, color), 3, sizeof(Vertex));
        shaderProgram_->release();
    }

    selectionVertexBuffer_.release();
    selectionVao_.release();
}

void ViewportRenderer::uploadGizmo()
{
    if (!gizmoVertexBuffer_.isCreated() || shaderProgram_ == nullptr) {
        return;
    }

    gizmoVao_.bind();
    gizmoVertexBuffer_.bind();
    gizmoVertexBuffer_.allocate(
        gizmoVertices_.isEmpty() ? nullptr : gizmoVertices_.constData(),
        gizmoVertices_.size() * sizeof(Vertex));

    if (shaderProgram_) {
        shaderProgram_->bind();
        shaderProgram_->enableAttributeArray(0);
        shaderProgram_->setAttributeBuffer(0, GL_FLOAT, offsetof(Vertex, position), 3, sizeof(Vertex));
        shaderProgram_->enableAttributeArray(1);
        shaderProgram_->setAttributeBuffer(1, GL_FLOAT, offsetof(Vertex, normal), 3, sizeof(Vertex));
        shaderProgram_->enableAttributeArray(2);
        shaderProgram_->setAttributeBuffer(2, GL_FLOAT, offsetof(Vertex, color), 3, sizeof(Vertex));
        shaderProgram_->release();
    }

    gizmoVertexBuffer_.release();
    gizmoVao_.release();
}
