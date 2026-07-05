#include "rendering/ViewportRenderer.h"

#include "logging/LogCategories.h"
#include "rendering/ShaderUtils.h"
#include "scene/Scene.h"
#include "viewport/EditorCamera.h"

#include <QDebug>
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>

namespace
{
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
}

ViewportRenderer::ViewportRenderer()
    : vertexBuffer_(QOpenGLBuffer::VertexBuffer)
    , importedVertexBuffer_(QOpenGLBuffer::VertexBuffer)
    , importedIndexBuffer_(QOpenGLBuffer::IndexBuffer)
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
    functions_->glPolygonMode(GL_FRONT_AND_BACK, options.wireframe ? GL_LINE : GL_FILL);

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
        importedIndexBuffer_.release();
        importedVao_.release();
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
        if (object == nullptr || object->meshHandles().isEmpty()) {
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

            const int vertexCount = mesh->positions.size();
            for (int vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex) {
                const QVector3D position = mesh->positions[vertexIndex];
                const QVector3D normal = vertexIndex < mesh->normals.size()
                    ? mesh->normals[vertexIndex].normalized()
                    : QVector3D(0.0f, 1.0f, 0.0f);
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
