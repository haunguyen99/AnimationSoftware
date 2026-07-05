#pragma once

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLWidget>
#include <QPoint>

#include "io/FbxImporter.h"
#include "rendering/ViewportRenderer.h"
#include "scene/Scene.h"
#include "viewport/EditorCamera.h"

class ViewportWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
public:
    explicit ViewportWidget(QWidget* parent = nullptr);

    void requestRender();
    void resetCamera();
    void frameScene();
    void setWireframeEnabled(bool enabled);
    void setAxisVisible(bool visible);
    void setBackfaceCullingEnabled(bool enabled);
    bool importFbx(const QString& filePath);
    QString lastImportMessage() const;
    bool lastImportSucceeded() const;

protected:
    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    int viewportWidth_ = 0;
    int viewportHeight_ = 0;
    QPoint lastMousePosition_;
    Qt::MouseButtons activeButtons_;
    EditorCamera camera_;
    ViewportRenderer renderer_;
    ViewportRenderOptions renderOptions_;
    Scene scene_;
    FbxImporter importer_;
    QString lastImportMessage_;
    bool lastImportSucceeded_ = false;
    bool viewportInitialized_ = false;
    QString viewportInitFailureReason_;
};
