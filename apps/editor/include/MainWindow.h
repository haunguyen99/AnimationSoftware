#pragma once

#include <QMainWindow>
#include <QPoint>
#include <QVector>

#include <cstdint>

#include "EditorHistoryController.h"
#include "ScriptCommandSystem.h"
#include "scene/PrimitiveMeshFactory.h"
#include "scene/Scene.h"

class ViewportWorkspaceWidget;
class KeyframeTimelineWidget;
class QAction;
class QToolBar;
class QStatusBar;
class QDockWidget;
class QTreeWidget;
class QTreeWidgetItem;
class QWidget;
class QLabel;
class QPushButton;
class QDoubleSpinBox;
class QCheckBox;
class QListWidget;
class QListWidgetItem;
class QPlainTextEdit;
class QSpinBox;
class QSlider;
class QTimer;

class MainWindow : public QMainWindow
{
public:
    MainWindow();

private:
    bool eventFilter(QObject* watched, QEvent* event) override;
    enum class TransformUiMode
    {
        Translate,
        Rotate,
        Scale
    };
    enum class AxisUiOrientation
    {
        World,
        Local
    };
    enum class ViewCameraUiPreset
    {
        Perspective,
        Front,
        Back,
        Left,
        Right,
        Top,
        Bottom
    };
    void createMenus();
    void createToolbar();
    void createDocks();
    QWidget* createOutlinerPanel();
    QWidget* createInspectorPanel();
    QWidget* createTimeSliderPanel();
    QWidget* createScriptEditorPanel();
    void newScene();
    void openScene();
    void importFbx();
    bool saveScene();
    bool saveSceneAs();
    bool openSceneFromPath(const QString& filePath, bool logToScript);
    bool importFbxFromPath(const QString& filePath, bool logToScript);
    bool saveSceneToPath(const QString& filePath, bool logToScript);
    bool incrementAndSave();
    bool archiveScene();
    bool exportAll();
    bool exportSelection();
    void savePreferences();
    void loadPreferences();
    void createPrimitiveFromPalette();
    void createJoint();
    void markSelectionAsHierarchyParent();
    void parentSelectionToMarkedParent();
    void unparentSelection();
    void bindSelectedMeshToMarkedJoint();
    void resetSelectedJointOrientation();
    void alignSelectedJointOrientationToChild();
    void captureSelectedBindPose();
    void captureSelectedBindPoseRecursive();
    bool reparentObjectInUi(std::uint64_t childId, std::uint64_t newParentId, bool logToScript = true);
    void showScriptEditorWindow();
    void executeScriptEditorAll();
    void executeScriptEditorSelection();
    void clearScriptHistory();
    void appendScriptHistoryLine(const QString& line);
    void appendScriptComment(const QString& line);
    bool executeScriptCommand(QString commandLine, QString* resultLine = nullptr);
    ScriptCommandContext createScriptCommandContext();
    std::uint64_t findObjectIdByName(const QString& objectName) const;
    QString generateUniqueScriptName(const QString& prefix) const;
    QString generateUniqueObjectName(const QString& baseName, std::uint64_t ignoreObjectId = 0) const;
    void logPrimitiveCreationToScriptEditor(PrimitiveMeshFactory::Type type, const QString& objectName);
    void logSelectionToScriptEditor(std::uint64_t objectId);
    void logChannelBoxChangeToScriptEditor(const Transform& transform);
    void logVisibilityChangeToScriptEditor(bool visible);
    void refreshScenePanels();
    void populateOutliner();
    void populateOutlinerItem(QTreeWidgetItem* parentItem, std::uint64_t objectId);
    bool shouldPromoteOutlinerNode(std::uint64_t objectId) const;
    void updateInspector(std::uint64_t objectId);
    void clearInspector();
    void handleOutlinerSelectionChanged();
    void frameSelectedObject();
    void selectObject(std::uint64_t objectId, bool syncOutliner);
    void syncOutlinerSelection(std::uint64_t objectId);
    void setViewCameraPreset(ViewCameraUiPreset preset);
    void setTransformUiMode(TransformUiMode mode);
    void setAxisUiOrientation(AxisUiOrientation orientation);
    void restoreDefaultWorkspaceLayout();
    void showPolygonPrimitivesWindow();
    void updateWindowTitle();
    void setCurrentFrame(int frame, bool logToScript = true);
    void setKeyForSelection(bool logToScript = true);
    void deleteKeyForSelection(bool logToScript = true);
    void duplicateCurrentKeyForSelection(bool logToScript = true);
    void shiftSelectedObjectKeyframes(int frameDelta, bool logToScript = true);
    void setAutoKeyEnabled(bool enabled, bool logToScript = true);
    void setPlaybackRange(int startFrame, int endFrame, bool logToScript = true);
    void stepFrame(int delta);
    void jumpToSelectedObjectKeyframe(bool forward, bool logToScript = true);
    void togglePlayback();
    void advancePlayback();
    void updateChannelBox(std::uint64_t objectId);
    void refreshAnimationTimelineUi();
    void setChannelBoxEnabled(bool enabled);
    void applyChannelBoxToSelection();
    void applyJointOrientationToSelection();
    void applyVisibilityToSelection(bool visible);
    PrimitiveMeshFactory::Type primitiveTypeFromItem(const QListWidgetItem* item) const;
    void restoreHistoryState(const EditorHistoryState& state);
    void recordUndoState();
    void undoLastChange();
    void redoLastChange();
    void updateUndoRedoActions();

    ViewportWorkspaceWidget* viewport_ = nullptr;
    QDockWidget* viewportDock_ = nullptr;
    QDockWidget* outlinerDock_ = nullptr;
    QDockWidget* inspectorDock_ = nullptr;
    QDockWidget* polygonPrimitivesDock_ = nullptr;
    QDockWidget* scriptEditorDock_ = nullptr;
    QDockWidget* timeSliderDock_ = nullptr;
    QTreeWidget* outlinerTree_ = nullptr;
    QListWidget* polygonPrimitivesList_ = nullptr;
    QPlainTextEdit* scriptHistoryTextEdit_ = nullptr;
    QPlainTextEdit* scriptInputTextEdit_ = nullptr;
    QSlider* timeSlider_ = nullptr;
    KeyframeTimelineWidget* keyframeTimelineWidget_ = nullptr;
    QSpinBox* currentFrameSpinBox_ = nullptr;
    QSpinBox* playbackStartSpinBox_ = nullptr;
    QSpinBox* playbackEndSpinBox_ = nullptr;
    QPushButton* setKeyButton_ = nullptr;
    QPushButton* deleteKeyButton_ = nullptr;
    QPushButton* duplicateKeyButton_ = nullptr;
    QPushButton* shiftKeysLeftButton_ = nullptr;
    QPushButton* shiftKeysRightButton_ = nullptr;
    QPushButton* autoKeyButton_ = nullptr;
    QLabel* timelineStatusLabel_ = nullptr;
    QPushButton* playPauseButton_ = nullptr;
    QPushButton* previousKeyButton_ = nullptr;
    QPushButton* nextKeyButton_ = nullptr;
    QTimer* playbackTimer_ = nullptr;
    QWidget* inspectorDetailsWidget_ = nullptr;
    QLabel* inspectorEmptyStateLabel_ = nullptr;
    QLabel* channelObjectNameLabel_ = nullptr;
    QDoubleSpinBox* translateXSpinBox_ = nullptr;
    QDoubleSpinBox* translateYSpinBox_ = nullptr;
    QDoubleSpinBox* translateZSpinBox_ = nullptr;
    QDoubleSpinBox* rotateXSpinBox_ = nullptr;
    QDoubleSpinBox* rotateYSpinBox_ = nullptr;
    QDoubleSpinBox* rotateZSpinBox_ = nullptr;
    QDoubleSpinBox* scaleXSpinBox_ = nullptr;
    QDoubleSpinBox* scaleYSpinBox_ = nullptr;
    QDoubleSpinBox* scaleZSpinBox_ = nullptr;
    QWidget* jointToolsWidget_ = nullptr;
    QDoubleSpinBox* jointOrientXSpinBox_ = nullptr;
    QDoubleSpinBox* jointOrientYSpinBox_ = nullptr;
    QDoubleSpinBox* jointOrientZSpinBox_ = nullptr;
    QLabel* bindPoseStatusLabel_ = nullptr;
    QLabel* skinBindingStatusLabel_ = nullptr;
    QPushButton* resetJointOrientationButton_ = nullptr;
    QPushButton* alignJointOrientationButton_ = nullptr;
    QPushButton* captureBindPoseButton_ = nullptr;
    QPushButton* captureBindPoseRecursiveButton_ = nullptr;
    QCheckBox* visibilityCheckBox_ = nullptr;
    QPushButton* frameSelectedButton_ = nullptr;
    QToolBar* toolbar_ = nullptr;
    QAction* undoAction_ = nullptr;
    QAction* redoAction_ = nullptr;
    QAction* importFbxAction_ = nullptr;
    QAction* newSceneAction_ = nullptr;
    QAction* openSceneAction_ = nullptr;
    QAction* saveSceneAction_ = nullptr;
    QAction* saveSceneAsAction_ = nullptr;
    QAction* incrementAndSaveAction_ = nullptr;
    QAction* archiveSceneAction_ = nullptr;
    QAction* savePreferencesAction_ = nullptr;
    QAction* optimizeSceneSizeAction_ = nullptr;
    QAction* exportAllAction_ = nullptr;
    QAction* exportSelectionAction_ = nullptr;
    QAction* resetCameraAction_ = nullptr;
    QAction* frameSceneAction_ = nullptr;
    QAction* frameSelectedAction_ = nullptr;
    QAction* wireframeAction_ = nullptr;
    QAction* showAxisAction_ = nullptr;
    QAction* backfaceCullingAction_ = nullptr;
    QAction* perspectiveCameraAction_ = nullptr;
    QAction* frontCameraAction_ = nullptr;
    QAction* backCameraAction_ = nullptr;
    QAction* leftCameraAction_ = nullptr;
    QAction* rightCameraAction_ = nullptr;
    QAction* topCameraAction_ = nullptr;
    QAction* bottomCameraAction_ = nullptr;
    QAction* translateAction_ = nullptr;
    QAction* rotateAction_ = nullptr;
    QAction* scaleAction_ = nullptr;
    QAction* worldAxisAction_ = nullptr;
    QAction* localAxisAction_ = nullptr;
    QAction* restoreWorkspaceLayoutAction_ = nullptr;
    QAction* polygonPrimitivesAction_ = nullptr;
    QAction* createJointAction_ = nullptr;
    QAction* markHierarchyParentAction_ = nullptr;
    QAction* parentToMarkedParentAction_ = nullptr;
    QAction* unparentSelectedAction_ = nullptr;
    QAction* bindSkinAction_ = nullptr;
    QAction* resetJointOrientationAction_ = nullptr;
    QAction* alignJointOrientationAction_ = nullptr;
    QAction* captureBindPoseAction_ = nullptr;
    QAction* captureBindPoseRecursiveAction_ = nullptr;
    QAction* scriptEditorAction_ = nullptr;
    QAction* duplicateKeyAction_ = nullptr;
    QAction* shiftKeysLeftAction_ = nullptr;
    QAction* shiftKeysRightAction_ = nullptr;
    QAction* previousKeyAction_ = nullptr;
    QAction* nextKeyAction_ = nullptr;
    ScriptCommandRegistry scriptCommandRegistry_;
    QString currentSceneFilePath_;
    std::uint64_t selectedObjectId_ = 0;
    std::uint64_t markedHierarchyParentId_ = 0;
    std::uint64_t hierarchyDragSourceId_ = 0;
    QPoint hierarchyDragStartPos_;
    bool hierarchyDragActive_ = false;
    int currentFrame_ = 0;
    int playbackStartFrame_ = 0;
    int playbackEndFrame_ = 24;
    bool updatingChannelBox_ = false;
    bool updatingTimeSlider_ = false;
    bool restoringHistory_ = false;
    bool autoKeyEnabled_ = false;
    bool interactivePrimitiveCreationEnabled_ = true;
    bool exitPrimitiveToolOnCompletionEnabled_ = true;
    EditorHistoryController historyController_;
};
