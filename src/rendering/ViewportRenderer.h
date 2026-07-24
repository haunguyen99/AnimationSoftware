#pragma once

#include <QOpenGLBuffer>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLVertexArrayObject>
#include <QVector>
#include <QVector3D>

#include <memory>

#include "scene/Bounds3D.h"

class EditorCamera;
class QOpenGLShaderProgram;
class Scene;

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
    void syncScene(const Scene& scene);
    void setSelectedBounds(const Bounds3D& bounds);
    void setGizmo(const QVector3D& origin, float size, GizmoMode mode, const QVector<QVector3D>& axes, const QVector3D& cameraForward, int activeAxis);
    void clearGizmo();

    struct Vertex
    {
        QVector3D position;
        QVector3D normal;
        QVector3D color;
    };

private:
    void destroyGlResources();
    void createSceneGeometry();
    void uploadGeometry();
    void uploadImportedMesh(const Scene& scene);
    void uploadJointGeometry(const Scene& scene);
    void uploadSelectionBounds();
    void uploadGizmo();

    QOpenGLFunctions_3_3_Core* functions_ = nullptr;
    std::unique_ptr<QOpenGLShaderProgram> shaderProgram_;
    QOpenGLVertexArrayObject vao_;
    QOpenGLVertexArrayObject importedVao_;
    QOpenGLVertexArrayObject jointVao_;
    QOpenGLVertexArrayObject selectionVao_;
    QOpenGLVertexArrayObject gizmoVao_;
    QOpenGLBuffer vertexBuffer_;
    QOpenGLBuffer importedVertexBuffer_;
    QOpenGLBuffer importedIndexBuffer_;
    QOpenGLBuffer jointVertexBuffer_;
    QOpenGLBuffer selectionVertexBuffer_;
    QOpenGLBuffer gizmoVertexBuffer_;
    QVector<Vertex> gridVertices_;
    QVector<Vertex> axisVertices_;
    QVector<Vertex> importedVertices_;
    QVector<std::uint32_t> importedIndices_;
    QVector<Vertex> jointVertices_;
    QVector<Vertex> selectionVertices_;
    QVector<Vertex> gizmoVertices_;
    int viewportWidth_ = 1;
    int viewportHeight_ = 1;
};
