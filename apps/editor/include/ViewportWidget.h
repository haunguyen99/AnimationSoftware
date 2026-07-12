#pragma once

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLWidget>
#include <QPoint>

#include <functional>

#include "io/FbxImporter.h"
#include "rendering/ViewportRenderer.h"
#include "scene/PrimitiveMeshFactory.h"
#include "scene/Scene.h"
#include "viewport/EditorCamera.h"

class ViewportWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
public:
    enum class TransformMode
    {
        Translate,
        Rotate,
        Scale
    };
    enum class AxisOrientation
    {
        World,
        Local
    };

    explicit ViewportWidget(QWidget* parent = nullptr);

    const Scene& scene() const;
    void requestRender();
    void resetCamera();
    void frameScene();
    void frameObject(SceneObject::Id objectId);
    void setSelectedObject(SceneObject::Id objectId);
    SceneObject::Id selectedObject() const;
    void setWireframeEnabled(bool enabled);
    void setAxisVisible(bool visible);
    void setBackfaceCullingEnabled(bool enabled);
    void setTransformMode(TransformMode mode);
    TransformMode transformMode() const;
    void setAxisOrientation(AxisOrientation orientation);
    AxisOrientation axisOrientation() const;
    void setSelectionChangedCallback(std::function<void(SceneObject::Id)> callback);
    void setObjectTransformChangedCallback(std::function<void(SceneObject::Id)> callback);
    void setAutoKeyEnabled(bool enabled);
    bool autoKeyEnabled() const;
    bool importFbx(const QString& filePath);
    void clearScene();
    void replaceScene(const Scene& scene);
    void optimizeSceneStorage();
    QString lastImportMessage() const;
    bool lastImportSucceeded() const;
    bool setObjectLocalTransform(SceneObject::Id objectId, const Transform& transform);
    bool setObjectVisibility(SceneObject::Id objectId, bool visible);
    int currentFrame() const;
    void setCurrentFrame(int frame);
    bool setObjectKeyframe(SceneObject::Id objectId, int frame);
    bool removeObjectKeyframe(SceneObject::Id objectId, int frame);
    SceneObject::Id createPrimitive(PrimitiveMeshFactory::Type type, const QString& name = {});

protected:
    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    enum class GizmoAxis
    {
        None = -1,
        X = 0,
        Y = 1,
        Z = 2
    };

    struct DragState
    {
        bool active = false;
        QPoint startMousePosition;
        Transform startTransform;
        QVector3D gizmoOrigin;
        float gizmoSize = 1.0f;
        GizmoAxis axis = GizmoAxis::None;
    };

    void syncRendererSelection();
    void syncSceneToRenderer();
    void applyRendererSelectionState();
    void updateSelectedObject(SceneObject::Id objectId);
    SceneObject::Id pickObjectAtScreenPos(const QPoint& position) const;
    GizmoAxis pickGizmoAxisAtScreenPos(const QPoint& position) const;
    QPointF projectWorldToScreen(const QVector3D& worldPosition) const;
    QVector<QVector3D> gizmoAxesWorld() const;
    QVector3D gizmoAxisDirectionWorld(GizmoAxis axis) const;
    QMatrix4x4 parentWorldTransform() const;
    QVector3D gizmoOrigin() const;
    float gizmoSize() const;
    void applyDrag(const QPoint& currentPosition);
    void handleHotkeys(QMouseEvent* event);

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
    SceneObject::Id selectedObjectId_ = 0;
    TransformMode transformMode_ = TransformMode::Translate;
    AxisOrientation axisOrientation_ = AxisOrientation::World;
    DragState dragState_;
    std::function<void(SceneObject::Id)> selectionChangedCallback_;
    std::function<void(SceneObject::Id)> objectTransformChangedCallback_;
    bool autoKeyEnabled_ = false;
};
