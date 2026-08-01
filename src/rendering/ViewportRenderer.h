#pragma once

#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLVertexArrayObject>
#include <QVector>
#include <QVector3D>

#include <memory>

#include "rendering/RenderTypes.h"
#include "scene/Bounds3D.h"

class EditorCamera;
class QOpenGLShaderProgram;
struct ViewportRenderSceneData;

struct ViewportRenderOptions
{
    bool showAxis = true;
    bool wireframe = false;
    bool backfaceCulling = false;
    bool showSelectionOutline = true;
};

class ViewportRenderer
{
public:
    enum class GizmoMode
    {
        Translate,
        Rotate,
        Scale
    };

    ViewportRenderer();
    ~ViewportRenderer();

    bool initialize(QOpenGLFunctions_3_3_Core* functions);
    void resize(int width, int height);
    void render(const EditorCamera& camera, const ViewportRenderOptions& options);
    void syncScene(const ViewportRenderSceneData& sceneData);
    void setPreviewScene(const ViewportRenderSceneData& sceneData, const QMatrix4x4& modelMatrix);
    void updatePreviewTransform(const QMatrix4x4& modelMatrix);
    void clearPreviewScene();
    void setSelectedBounds(const Bounds3D& bounds);
    void setGizmo(const QVector3D& origin, float size, GizmoMode mode, const QVector<QVector3D>& axes, const QVector3D& cameraForward, int activeAxis);
    void clearGizmo();

    using Vertex = RenderVertex;

private:
    void destroyGlResources();
    void createSceneGeometry();
    void uploadGeometry();
    void uploadImportedMesh();
    void uploadJointGeometry();
    void uploadPreviewMesh();
    void uploadPreviewJointGeometry();
    void uploadSelectionBounds();
    void uploadGizmo();

    QOpenGLFunctions_3_3_Core* functions_ = nullptr;
    std::unique_ptr<QOpenGLShaderProgram> shaderProgram_;
    QOpenGLVertexArrayObject vao_;
    QOpenGLVertexArrayObject importedVao_;
    QOpenGLVertexArrayObject previewImportedVao_;
    QOpenGLVertexArrayObject jointVao_;
    QOpenGLVertexArrayObject previewJointVao_;
    QOpenGLVertexArrayObject selectionVao_;
    QOpenGLVertexArrayObject gizmoVao_;
    QOpenGLBuffer vertexBuffer_;
    QOpenGLBuffer importedVertexBuffer_;
    QOpenGLBuffer importedIndexBuffer_;
    QOpenGLBuffer previewImportedVertexBuffer_;
    QOpenGLBuffer previewImportedIndexBuffer_;
    QOpenGLBuffer jointVertexBuffer_;
    QOpenGLBuffer previewJointVertexBuffer_;
    QOpenGLBuffer selectionVertexBuffer_;
    QOpenGLBuffer gizmoVertexBuffer_;
    QVector<Vertex> gridVertices_;
    QVector<Vertex> axisVertices_;
    QVector<Vertex> importedVertices_;
    QVector<std::uint32_t> importedIndices_;
    QVector<Vertex> previewImportedVertices_;
    QVector<std::uint32_t> previewImportedIndices_;
    QVector<Vertex> jointVertices_;
    QVector<Vertex> previewJointVertices_;
    QVector<Vertex> selectionVertices_;
    QVector<Vertex> gizmoVertices_;
    QMatrix4x4 previewModelMatrix_;
    int viewportWidth_ = 1;
    int viewportHeight_ = 1;
};
