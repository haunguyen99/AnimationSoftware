#pragma once

#include <QList>
#include <QWidget>

#include <functional>

#include "ViewportWidget.h"

class QFrame;
class QGridLayout;
class QMenuBar;
class QAction;

class ViewportWorkspaceWidget : public QWidget
{
public:
    using TransformMode = ViewportWidget::TransformMode;
    using AxisOrientation = ViewportWidget::AxisOrientation;
    using CameraViewPreset = ViewportWidget::CameraViewPreset;

    explicit ViewportWorkspaceWidget(QWidget* parent = nullptr);

    const Scene& scene() const;
    Scene sceneSnapshot() const;
    bool containsObject(SceneObject::Id objectId) const;
    const SceneObject* findObject(SceneObject::Id objectId) const;
    QVector<SceneObject::Id> rootObjectIds() const;
    QVector<SceneObject::Id> allObjectIds() const;
    bool isSceneEmpty() const;
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
    int nextObjectKeyframe(SceneObject::Id objectId, int frame) const;
    int previousObjectKeyframe(SceneObject::Id objectId, int frame) const;
    bool setObjectKeyframe(SceneObject::Id objectId, int frame);
    bool removeObjectKeyframe(SceneObject::Id objectId, int frame);
    bool setJointOrientation(SceneObject::Id objectId, const QQuaternion& orientation);
    bool resetJointOrientation(SceneObject::Id objectId);
    bool alignJointOrientationToChild(SceneObject::Id objectId);
    bool captureBindPose(SceneObject::Id objectId, bool recursive = false);
    SceneObject::Id createPrimitive(PrimitiveMeshFactory::Type type, const QString& name = {});
    SceneObject::Id createJoint(const QString& name = {}, SceneObject::Id parentId = 0);

    bool quadViewEnabled() const;
    void toggleQuadView();
    CameraViewPreset activeCameraViewPreset() const;
    bool isCameraViewVisible(CameraViewPreset preset) const;
    void setWorkspaceActive(bool active);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    struct ViewPane
    {
        ViewportWidget* viewport = nullptr;
        QFrame* frame = nullptr;
        QMenuBar* menuBar = nullptr;
    };

    void configurePane(ViewPane& pane, const QString& viewportObjectName, CameraViewPreset preset);
    void rebuildViewportLayout();
    void setActiveViewport(ViewportWidget* viewport);
    void refreshActiveViewportFrameStyles();
    void refreshViewportMenuStates();
    void syncSceneToAllViewports(ViewportWidget* sourceViewport);
    void syncSelectionToAllViewports();
    void frameAllViewports();
    QList<ViewportWidget*> allViewports() const;
    ViewPane* paneForViewport(ViewportWidget* viewport);
    const ViewPane* paneForViewport(ViewportWidget* viewport) const;
    ViewPane* paneForPreset(CameraViewPreset preset);
    const ViewPane* paneForPreset(CameraViewPreset preset) const;

    QWidget* layoutHost_ = nullptr;
    QGridLayout* gridLayout_ = nullptr;
    ViewPane perspectivePane_;
    ViewPane topPane_;
    ViewPane frontPane_;
    ViewPane sidePane_;
    ViewportWidget* activeViewport_ = nullptr;
    std::function<void(SceneObject::Id)> selectionChangedCallback_;
    std::function<void(SceneObject::Id)> objectTransformChangedCallback_;
    std::function<void()> beforeSceneMutationCallback_;
    SceneObject::Id selectedObjectId_ = 0;
    bool quadViewEnabled_ = false;
    bool wireframeEnabled_ = false;
    bool axisVisible_ = true;
    bool backfaceCullingEnabled_ = false;
    bool selectionOutlineVisible_ = true;
    TransformMode transformMode_ = TransformMode::Translate;
    AxisOrientation axisOrientation_ = AxisOrientation::World;
    bool autoKeyEnabled_ = false;
    bool workspaceActive_ = true;
    QString lastImportMessage_;
    bool lastImportSucceeded_ = false;
    QList<QAction*> wireframeActions_;
    QList<QAction*> axisActions_;
    QList<QAction*> backfaceActions_;
    QList<QAction*> selectionOutlineActions_;
    QList<QAction*> quadViewActions_;
};
