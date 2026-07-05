#pragma once

#include <QOpenGLBuffer>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLVertexArrayObject>
#include <QVector>
#include <QVector3D>

#include <memory>

class EditorCamera;
class QOpenGLShaderProgram;
class Scene;

struct ViewportRenderOptions
{
    bool showAxis = true;
    bool wireframe = false;
    bool backfaceCulling = false;
};

class ViewportRenderer
{
public:
    ViewportRenderer();
    ~ViewportRenderer();

    bool initialize(QOpenGLFunctions_3_3_Core* functions);
    void resize(int width, int height);
    void render(const EditorCamera& camera, const ViewportRenderOptions& options);
    void syncScene(const Scene& scene);

private:
    struct Vertex
    {
        QVector3D position;
        QVector3D normal;
        QVector3D color;
    };

    void createSceneGeometry();
    void uploadGeometry();
    void uploadImportedMesh(const Scene& scene);

    QOpenGLFunctions_3_3_Core* functions_ = nullptr;
    std::unique_ptr<QOpenGLShaderProgram> shaderProgram_;
    QOpenGLVertexArrayObject vao_;
    QOpenGLVertexArrayObject importedVao_;
    QOpenGLBuffer vertexBuffer_;
    QOpenGLBuffer importedVertexBuffer_;
    QOpenGLBuffer importedIndexBuffer_;
    QVector<Vertex> gridVertices_;
    QVector<Vertex> axisVertices_;
    QVector<Vertex> importedVertices_;
    QVector<std::uint32_t> importedIndices_;
    int viewportWidth_ = 1;
    int viewportHeight_ = 1;
};
