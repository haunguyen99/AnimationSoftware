#pragma once

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLWidget>
#include <QPoint>
#include <QString>
#include <QElapsedTimer>

#include <functional>

#include "engine/runtime/EditorViewportSceneController.h"
#include "io/FbxImporter.h"
#include "rendering/ViewportRenderer.h"
#include "scene/PrimitiveMeshFactory.h"
#include "scene/Scene.h"
#include "viewport/EditorCamera.h"
#include "viewport/gizmo/GizmoTypes.h"

class ViewportWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
public:
    using GizmoHandle = ::GizmoHandle;

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
    enum class CameraViewPreset
    {
        Perspective,
        Front,
        Back,
        Left,
        Right,
        Top,
        Bottom
    };

    explicit ViewportWidget(QWidget* parent = nullptr);

    const Scene& scene() const;
    void requestRender();
    void resetCamera();
    void setCameraViewPreset(CameraViewPreset preset);
    CameraViewPreset cameraViewPreset() const;
    QString cameraViewLabel() const;
    void frameScene();
    void frameObject(SceneObject::Id objectId);
    void setSelectedObject(SceneObject::Id objectId);
    SceneObject::Id selectedObject() const;
    void setWireframeEnabled(bool enabled);
    void setAxisVisible(bool visible);
    void setBackfaceCullingEnabled(bool enabled);
    void setSelectionOutlineVisible(bool visible);
    void setTransformMode(TransformMode mode);
    TransformMode transformMode() const;
    void setAxisOrientation(AxisOrientation orientation);
    AxisOrientation axisOrientation() const;
    void setSelectionChangedCallback(std::function<void(SceneObject::Id)> callback);
    void setObjectTransformChangedCallback(std::function<void(SceneObject::Id)> callback);
    void setBeforeSceneMutationCallback(std::function<void()> callback);
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
    bool setJointOrientation(SceneObject::Id objectId, const QQuaternion& orientation);
    bool resetJointOrientation(SceneObject::Id objectId);
    bool alignJointOrientationToChild(SceneObject::Id objectId);
    bool captureBindPose(SceneObject::Id objectId, bool recursive = false);
    SceneObject::Id createPrimitive(PrimitiveMeshFactory::Type type, const QString& name = {});
    SceneObject::Id createJoint(const QString& name = {}, SceneObject::Id parentId = 0);

protected:
    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    struct DragState
    {
        bool active = false;
        bool historyCaptured = false;
        QPoint startMousePosition;
        QPointF startScreenVector;
        Transform startTransform;
        QVector3D gizmoOrigin;
        float gizmoSize = 1.0f;
        GizmoHandle handle = GizmoHandle::None;
        QVector3D dragPlaneOrigin;
        QVector3D dragPlaneNormal;
        QVector3D dragStartWorldPoint;
        QMatrix4x4 previewStartWorldMatrix;
    };

    void syncRendererSelection();
    void syncSceneToRenderer();
    void syncSceneToRendererExcludingSelection();
    void applyRendererSelectionState();
    void beginInteractivePreview();
    void updateInteractivePreview();
    void endInteractivePreview();
    void updateSelectedObject(SceneObject::Id objectId);
    SceneObject::Id pickObjectAtScreenPos(const QPoint& position) const;
    GizmoHandle pickGizmoHandleAtScreenPos(const QPoint& position) const;
    QVector<QVector3D> gizmoAxesWorld() const;
    QVector3D gizmoAxisDirectionWorld(GizmoHandle handle) const;
    QVector3D gizmoPlaneNormalWorld(GizmoHandle handle) const;
    bool gizmoHandleUsesPlaneDrag(GizmoHandle handle) const;
    QMatrix4x4 parentWorldTransform() const;
    QVector3D selectedObjectWorldOrigin() const;
    QString cameraViewLabelText() const;
    QVector3D gizmoOrigin() const;
    float gizmoSize() const;
    EditorViewportSceneController::Context sceneControllerContext();
    void notifyBeforeSceneMutation();
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
    std::function<void()> beforeSceneMutationCallback_;
    bool autoKeyEnabled_ = false;
    bool suppressBeforeSceneMutationCallback_ = false;
    QElapsedTimer dragRenderSyncTimer_;
    bool interactivePreviewActive_ = false;
};
