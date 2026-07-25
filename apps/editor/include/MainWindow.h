#pragma once

#include <QMainWindow>
#include <QPoint>
#include <QVector>

#include <cstdint>

#include "EditorAnimationController.h"
#include "EditorAnimationEngineFacade.h"
#include "EditorAnimationFlowController.h"
#include "EditorChannelBoxController.h"
#include "EditorCreationController.h"
#include "EditorDocumentController.h"
#include "EditorHistoryController.h"
#include "EditorInspectorController.h"
#include "EditorFileFlowController.h"
#include "EditorOutlinerController.h"
#include "EditorPlaybackController.h"
#include "EditorRiggingController.h"
#include "EditorSceneMutationController.h"
#include "EditorSceneRuntimeController.h"
#include "EditorScriptExecutionController.h"
#include "EditorSelectionController.h"
#include "EditorViewportCommandController.h"
#include "EditorViewportUiController.h"
#include "ScriptCommandSystem.h"
#include "scene/PrimitiveMeshFactory.h"
#include "scene/Scene.h"

class ViewportWorkspaceWidget;
class AnimationTimelinePanel;
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
    void createEditMenu();
    void createFileMenu();
    void createCreateMenu();
    void createRigMenu();
    void createWindowsMenu();
    void createAnimationMenu();
    void createViewMenu();
    void createTransformMenu();
    void createToolbar();
    void addImportCreateToolbarSection();
    void addRigToolbarSection();
    void addViewToolbarSection();
    void addTransformToolbarSection();
    void addDisplayToolbarSection();
    void createDocks();
    void createOutlinerDock();
    void createInspectorDock();
    void createPrimitivePaletteDock();
    void createScriptEditorDock();
    void createTimelineDock();
    QWidget* createPrimitivePalettePanel();
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
    void optimizeSceneStorage();
    void savePreferences();
    void loadPreferences();
    void handlePrimitivePaletteItemActivated(QListWidgetItem* item);
    void setInteractivePrimitiveCreationEnabled(bool enabled);
    void setExitPrimitiveToolOnCompletionEnabled(bool enabled);
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
    bool executeScriptCommand(QString commandLine, QString* resultLine = nullptr);
    ScriptCommandContext createScriptCommandContext();
    EditorAnimationEngineFacade::ScriptBindings createAnimationScriptBindings();
    EditorCreationController::ScriptBindings createCreationScriptBindings();
    EditorSceneMutationController::ScriptBindings createSceneScriptBindings();
    std::uint64_t findObjectIdByName(const QString& objectName) const;
    QString generateUniqueScriptName(const QString& prefix) const;
    QString generateUniqueObjectName(const QString& baseName, std::uint64_t ignoreObjectId = 0) const;
    void refreshScenePanels();
    void populateOutliner();
    void updateInspector(std::uint64_t objectId);
    void clearInspector();
    void handleOutlinerSelectionChanged();
    void frameSelectedObject();
    void selectObject(std::uint64_t objectId, bool syncOutliner);
    EditorAnimationTimelineViewModel buildAnimationTimelineViewModel() const;
    void resetSceneCamera();
    void frameEntireScene();
    void setWireframeDisplayEnabled(bool enabled);
    void setAxisVisibilityEnabled(bool enabled);
    void setBackfaceCullingEnabled(bool enabled);
    void setViewCameraPreset(ViewCameraUiPreset preset);
    void setTransformUiMode(TransformUiMode mode);
    void setAxisUiOrientation(AxisUiOrientation orientation);
    void restoreDefaultWorkspaceLayout();
    void showPolygonPrimitivesWindow();
    void updateWindowTitle();
    void applyDocumentSceneLoad(const Scene& scene, const QString& filePath, bool frameScene);
    bool showDocumentOperationFailure(
        const QString& dialogTitle,
        const QString& errorMessage,
        const QString& statusMessage,
        int timeoutMs = 3000);
    void applyAnimationState(const EditorAnimationState& state, bool logToScript = false);
    void setKeyForSelection(bool logToScript = true);
    void deleteKeyForSelection(bool logToScript = true);
    void duplicateCurrentKeyForSelection(bool logToScript = true);
    void shiftSelectedObjectKeyframes(int frameDelta, bool logToScript = true);
    void setAutoKeyEnabled(bool enabled, bool logToScript = true);
    void jumpToSelectedObjectKeyframe(bool forward, bool logToScript = true);
    void updateChannelBox(std::uint64_t objectId);
    void refreshAnimationTimelineUi();
    void setChannelBoxEnabled(bool enabled);
    void applyChannelBoxToSelection();
    void applyJointOrientationToSelection();
    void applyVisibilityToSelection(bool visible);
    EditorInspectorController::InspectorWidgets inspectorWidgets() const;
    EditorInspectorController::InspectorActions inspectorActions() const;
    EditorChannelBoxController::Context channelBoxContext() const;
    EditorCreationController::Context creationContext() const;
    EditorOutlinerController::SceneAccess outlinerSceneAccess() const;
    EditorRiggingController::Context riggingContext() const;
    EditorSceneRuntimeController::Context sceneRuntimeContext();
    EditorScriptExecutionController::ExecutionContext scriptExecutionContext();
    EditorSelectionController::Context selectionContext() const;
    EditorViewportUiController::ActionSet viewportUiActions() const;
    bool showErrorMessageIfPresent(const QString& errorMessage, int timeoutMs = 1500);
    void appendScriptResultLogLines(const QString& commentLine, const QString& commandLine, const QString& resultLine);
    void showStatusMessageIfPresent(const QString& statusMessage, int timeoutMs);
    void applyFileFlowResult(const EditorFileFlowController::OperationResult& result, bool logToScript = true, int timeoutMs = 3000);
    EditorAnimationEngineFacade::Context animationEngineContext();
    EditorAnimationFlowController::Context animationFlowContext() const;
    void applyAnimationEngineResult(const EditorAnimationEngineFacade::OperationResult& result);
    void applyAnimationFlowResult(const EditorAnimationFlowController::OperationResult& result);
    void applyCreationResult(const EditorCreationController::OperationResult& result);
    void applyRiggingOperationResult(const EditorRiggingController::OperationResult& result, bool logToScript = true);
    void applyChannelBoxOperationResult(const EditorChannelBoxController::OperationResult& result);
    void applyViewportUiOperationResult(const EditorViewportUiController::OperationResult& result, int timeoutMs = 1500);
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
    AnimationTimelinePanel* animationTimelinePanel_ = nullptr;
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
    EditorAnimationState animationState_;
    bool updatingChannelBox_ = false;
    bool updatingTimeSlider_ = false;
    bool restoringHistory_ = false;
    bool interactivePrimitiveCreationEnabled_ = true;
    bool exitPrimitiveToolOnCompletionEnabled_ = true;
    EditorHistoryController historyController_;
    EditorPlaybackController playbackController_;
};
