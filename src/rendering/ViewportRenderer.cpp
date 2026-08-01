#include "rendering/ViewportRenderer.h"

#include "core/logging/LogCategories.h"
#include "rendering/geometry/ViewportOverlayGeometryBuilder.h"
#include "rendering/scene/ViewportRenderSceneAdapter.h"
#include "rendering/ShaderUtils.h"
#include "viewport/EditorCamera.h"

#include <QDebug>
#include <QMatrix4x4>
#include <QOpenGLContext>
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
        uniform mat4 uModel;
        uniform mat4 uView;
        uniform bool uUseLighting;

        out vec3 vColor;
        out float vLighting;

        void main()
        {
            vColor = inColor;

            vec3 normal = normalize(mat3(uView * uModel) * inNormal);
            // Editor viewports prioritize readability over physically-correct
            // shading, especially in front/side/top orthographic views.
            vec3 keyLightDir = normalize(vec3(0.0, 0.0, 1.0));
            vec3 fillLightDir = normalize(vec3(0.45, 0.25, 0.85));

            float key = abs(dot(normal, keyLightDir));
            float fill = abs(dot(normal, fillLightDir));
            float wrapped = key * 0.72 + fill * 0.28;

            float viewportLighting = 0.46 + wrapped * 0.54;
            vLighting = uUseLighting ? clamp(viewportLighting, 0.0, 1.0) : 1.0;

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
}

ViewportRenderer::ViewportRenderer()
    : vertexBuffer_(QOpenGLBuffer::VertexBuffer)
    , importedVertexBuffer_(QOpenGLBuffer::VertexBuffer)
    , importedIndexBuffer_(QOpenGLBuffer::IndexBuffer)
    , previewImportedVertexBuffer_(QOpenGLBuffer::VertexBuffer)
    , previewImportedIndexBuffer_(QOpenGLBuffer::IndexBuffer)
    , jointVertexBuffer_(QOpenGLBuffer::VertexBuffer)
    , previewJointVertexBuffer_(QOpenGLBuffer::VertexBuffer)
    , selectionVertexBuffer_(QOpenGLBuffer::VertexBuffer)
    , gizmoVertexBuffer_(QOpenGLBuffer::VertexBuffer)
{
}

ViewportRenderer::~ViewportRenderer()
{
    if (QOpenGLContext::currentContext() == nullptr) {
        qCWarning(logViewport) << "renderer teardown skipped GL buffer destroy because no current context";
        return;
    }

    destroyGlResources();
}

void ViewportRenderer::destroyGlResources()
{
    if (shaderProgram_) {
        shaderProgram_.reset();
    }

    if (vao_.isCreated()) {
        vao_.destroy();
    }

    if (importedVao_.isCreated()) {
        importedVao_.destroy();
    }

    if (jointVao_.isCreated()) {
        jointVao_.destroy();
    }

    if (previewImportedVao_.isCreated()) {
        previewImportedVao_.destroy();
    }

    if (previewJointVao_.isCreated()) {
        previewJointVao_.destroy();
    }

    if (selectionVao_.isCreated()) {
        selectionVao_.destroy();
    }

    if (gizmoVao_.isCreated()) {
        gizmoVao_.destroy();
    }

    if (vertexBuffer_.isCreated()) {
        vertexBuffer_.destroy();
    }

    if (importedVertexBuffer_.isCreated()) {
        importedVertexBuffer_.destroy();
    }

    if (importedIndexBuffer_.isCreated()) {
        importedIndexBuffer_.destroy();
    }

    if (jointVertexBuffer_.isCreated()) {
        jointVertexBuffer_.destroy();
    }

    if (previewImportedVertexBuffer_.isCreated()) {
        previewImportedVertexBuffer_.destroy();
    }

    if (previewImportedIndexBuffer_.isCreated()) {
        previewImportedIndexBuffer_.destroy();
    }

    if (previewJointVertexBuffer_.isCreated()) {
        previewJointVertexBuffer_.destroy();
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

    destroyGlResources();
    gridVertices_.clear();
    axisVertices_.clear();
    importedVertices_.clear();
    importedIndices_.clear();
    previewImportedVertices_.clear();
    previewImportedIndices_.clear();
    jointVertices_.clear();
    previewJointVertices_.clear();
    selectionVertices_.clear();
    gizmoVertices_.clear();
    previewModelMatrix_.setToIdentity();

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

    if (!jointVao_.create()) {
        qCWarning(logViewport) << "failed to create joint VAO.";
        return false;
    }
    jointVao_.bind();
    if (!jointVertexBuffer_.create()) {
        qCWarning(logViewport) << "failed to create joint vertex buffer.";
        jointVao_.release();
        return false;
    }
    jointVao_.release();

    if (!previewImportedVao_.create()) {
        qCWarning(logViewport) << "failed to create preview imported mesh VAO.";
        return false;
    }
    previewImportedVao_.bind();
    if (!previewImportedVertexBuffer_.create()) {
        qCWarning(logViewport) << "failed to create preview imported mesh vertex buffer.";
        previewImportedVao_.release();
        return false;
    }
    if (!previewImportedIndexBuffer_.create()) {
        qCWarning(logViewport) << "failed to create preview imported mesh index buffer.";
        previewImportedVao_.release();
        return false;
    }
    previewImportedVao_.release();

    if (!previewJointVao_.create()) {
        qCWarning(logViewport) << "failed to create preview joint VAO.";
        return false;
    }
    previewJointVao_.bind();
    if (!previewJointVertexBuffer_.create()) {
        qCWarning(logViewport) << "failed to create preview joint vertex buffer.";
        previewJointVao_.release();
        return false;
    }
    previewJointVao_.release();

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
    functions_->glEnable(GL_DEPTH_TEST);
    functions_->glDepthFunc(GL_LEQUAL);
    functions_->glDepthMask(GL_TRUE);
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
    shaderProgram_->setUniformValue("uModel", QMatrix4x4());

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

    if (!previewImportedVertices_.isEmpty() && !previewImportedIndices_.isEmpty()) {
        const QMatrix4x4 previewMvp = camera.projectionMatrix() * viewMatrix * previewModelMatrix_;
        shaderProgram_->setUniformValue("uModel", previewModelMatrix_);
        shaderProgram_->setUniformValue("uMvp", previewMvp);
        shaderProgram_->setUniformValue("uUseLighting", true);
        previewImportedVao_.bind();
        previewImportedIndexBuffer_.bind();
        functions_->glDrawElements(GL_TRIANGLES, previewImportedIndices_.size(), GL_UNSIGNED_INT, nullptr);
        previewImportedIndexBuffer_.release();
        previewImportedVao_.release();
        shaderProgram_->setUniformValue("uModel", QMatrix4x4());
    }

    if (options.showSelectionOutline && !selectionVertices_.isEmpty()) {
        shaderProgram_->setUniformValue("uMvp", meshMvp);
        shaderProgram_->setUniformValue("uUseLighting", false);
        selectionVao_.bind();
        functions_->glDisable(GL_CULL_FACE);
        functions_->glEnable(GL_DEPTH_TEST);
        functions_->glDepthMask(GL_FALSE);
        functions_->glLineWidth(1.0f);
        functions_->glDrawArrays(GL_LINES, 0, selectionVertices_.size());
        selectionVao_.release();
    }

    if (!jointVertices_.isEmpty()) {
        shaderProgram_->setUniformValue("uMvp", meshMvp);
        shaderProgram_->setUniformValue("uUseLighting", false);
        jointVao_.bind();
        functions_->glDisable(GL_CULL_FACE);
        functions_->glDisable(GL_DEPTH_TEST);
        functions_->glDepthMask(GL_FALSE);
        functions_->glLineWidth(1.0f);
        functions_->glDrawArrays(GL_LINES, 0, jointVertices_.size());
        functions_->glEnable(GL_DEPTH_TEST);
        jointVao_.release();
    }

    if (!previewJointVertices_.isEmpty()) {
        const QMatrix4x4 previewMvp = camera.projectionMatrix() * viewMatrix * previewModelMatrix_;
        shaderProgram_->setUniformValue("uModel", previewModelMatrix_);
        shaderProgram_->setUniformValue("uMvp", previewMvp);
        shaderProgram_->setUniformValue("uUseLighting", false);
        previewJointVao_.bind();
        functions_->glDisable(GL_CULL_FACE);
        functions_->glDisable(GL_DEPTH_TEST);
        functions_->glDepthMask(GL_FALSE);
        functions_->glLineWidth(1.0f);
        functions_->glDrawArrays(GL_LINES, 0, previewJointVertices_.size());
        functions_->glEnable(GL_DEPTH_TEST);
        previewJointVao_.release();
        shaderProgram_->setUniformValue("uModel", QMatrix4x4());
    }

    if (!gizmoVertices_.isEmpty()) {
        shaderProgram_->setUniformValue("uMvp", meshMvp);
        shaderProgram_->setUniformValue("uUseLighting", false);
        gizmoVao_.bind();
        functions_->glDisable(GL_CULL_FACE);
        functions_->glDisable(GL_DEPTH_TEST);
        functions_->glDepthMask(GL_FALSE);
        functions_->glLineWidth(3.0f);
        functions_->glDrawArrays(GL_LINES, 0, gizmoVertices_.size());
        functions_->glEnable(GL_DEPTH_TEST);
        gizmoVao_.release();
    }

    vao_.bind();
    shaderProgram_->setUniformValue("uMvp", gridMvp);
    shaderProgram_->setUniformValue("uUseLighting", false);
    functions_->glEnable(GL_DEPTH_TEST);
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

void ViewportRenderer::syncScene(const ViewportRenderSceneData& sceneData)
{
    Q_ASSERT(functions_ != nullptr);
    if (functions_ == nullptr || !importedVertexBuffer_.isCreated() || !importedIndexBuffer_.isCreated()) {
        return;
    }

    importedVertices_ = sceneData.importedVertices;
    importedIndices_ = sceneData.importedIndices;
    jointVertices_ = sceneData.jointVertices;

    uploadImportedMesh();
    uploadJointGeometry();
}

void ViewportRenderer::setSelectedBounds(const Bounds3D& bounds)
{
    selectionVertices_ = ViewportOverlayGeometryBuilder::buildSelectionBounds(bounds);
    uploadSelectionBounds();
}

void ViewportRenderer::setPreviewScene(const ViewportRenderSceneData& sceneData, const QMatrix4x4& modelMatrix)
{
    previewImportedVertices_ = sceneData.importedVertices;
    previewImportedIndices_ = sceneData.importedIndices;
    previewJointVertices_ = sceneData.jointVertices;
    previewModelMatrix_ = modelMatrix;
    uploadPreviewMesh();
    uploadPreviewJointGeometry();
}

void ViewportRenderer::updatePreviewTransform(const QMatrix4x4& modelMatrix)
{
    previewModelMatrix_ = modelMatrix;
}

void ViewportRenderer::clearPreviewScene()
{
    previewImportedVertices_.clear();
    previewImportedIndices_.clear();
    previewJointVertices_.clear();
    previewModelMatrix_.setToIdentity();
    uploadPreviewMesh();
    uploadPreviewJointGeometry();
}

void ViewportRenderer::setGizmo(const QVector3D& origin, float size, GizmoMode mode, const QVector<QVector3D>& axes, const QVector3D& cameraForward, int activeAxis)
{
    gizmoVertices_ = ViewportOverlayGeometryBuilder::buildGizmo(origin, size, mode, axes, cameraForward, activeAxis);
    uploadGizmo();
}

void ViewportRenderer::clearGizmo()
{
    gizmoVertices_.clear();
    uploadGizmo();
}

void ViewportRenderer::uploadImportedMesh()
{
    Q_ASSERT(importedVertexBuffer_.isCreated());
    Q_ASSERT(importedIndexBuffer_.isCreated());

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

void ViewportRenderer::uploadPreviewMesh()
{
    if (!previewImportedVertexBuffer_.isCreated() || !previewImportedIndexBuffer_.isCreated()) {
        return;
    }

    previewImportedVao_.bind();
    previewImportedVertexBuffer_.bind();
    previewImportedVertexBuffer_.allocate(
        previewImportedVertices_.isEmpty() ? nullptr : previewImportedVertices_.constData(),
        previewImportedVertices_.size() * sizeof(Vertex));

    previewImportedIndexBuffer_.bind();
    previewImportedIndexBuffer_.allocate(
        previewImportedIndices_.isEmpty() ? nullptr : previewImportedIndices_.constData(),
        previewImportedIndices_.size() * sizeof(std::uint32_t));

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

    previewImportedVertexBuffer_.release();
    previewImportedIndexBuffer_.release();
    previewImportedVao_.release();
}

void ViewportRenderer::uploadJointGeometry()
{
    Q_ASSERT(jointVertexBuffer_.isCreated());

    jointVao_.bind();
    jointVertexBuffer_.bind();
    jointVertexBuffer_.allocate(
        jointVertices_.isEmpty() ? nullptr : jointVertices_.constData(),
        jointVertices_.size() * sizeof(Vertex));

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

    jointVertexBuffer_.release();
    jointVao_.release();
}

void ViewportRenderer::uploadPreviewJointGeometry()
{
    if (!previewJointVertexBuffer_.isCreated() || shaderProgram_ == nullptr) {
        return;
    }

    previewJointVao_.bind();
    previewJointVertexBuffer_.bind();
    previewJointVertexBuffer_.allocate(
        previewJointVertices_.isEmpty() ? nullptr : previewJointVertices_.constData(),
        previewJointVertices_.size() * sizeof(Vertex));

    shaderProgram_->bind();
    shaderProgram_->enableAttributeArray(0);
    shaderProgram_->setAttributeBuffer(0, GL_FLOAT, offsetof(Vertex, position), 3, sizeof(Vertex));
    shaderProgram_->enableAttributeArray(1);
    shaderProgram_->setAttributeBuffer(1, GL_FLOAT, offsetof(Vertex, normal), 3, sizeof(Vertex));
    shaderProgram_->enableAttributeArray(2);
    shaderProgram_->setAttributeBuffer(2, GL_FLOAT, offsetof(Vertex, color), 3, sizeof(Vertex));
    shaderProgram_->release();

    previewJointVertexBuffer_.release();
    previewJointVao_.release();
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
