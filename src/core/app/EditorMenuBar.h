#pragma once

#include <QAction>
#include <QMenu>
#include <QObject>
#include <QStringList>
#include <QVector>

#include <functional>

class QMainWindow;
class WorkspaceManager;

// EditorMenuBar — builds and owns the full menu bar for EditorShell.
//
// Usage:
//   EditorMenuBar* mb = new EditorMenuBar(this, this);
//   mb->build(bindings);                  // creates all menus + wires connections
//   const auto& a = mb->actions();        // access QAction* pointers at runtime
//
// Design mirrors WorkspaceManager: takes QMainWindow*, accepts a Bindings struct
// of std::function<> callbacks so EditorShell logic is never called directly here.

class EditorMenuBar : public QObject
{
    Q_OBJECT

public:
    // ── Output: QAction pointers populated by build() ────────────────────────

    struct Actions
    {
        // File
        QAction* newScene             = nullptr;
        QAction* openScene            = nullptr;
        QAction* saveScene            = nullptr;
        QAction* saveSceneAs          = nullptr;
        QAction* incrementAndSave     = nullptr;
        QAction* archiveScene         = nullptr;
        QAction* savePreferences      = nullptr;
        QAction* optimizeSceneSize    = nullptr;
        QAction* importFbx            = nullptr;
        QAction* exportAll            = nullptr;
        QAction* exportSelection      = nullptr;
        // Edit
        QAction* undo                 = nullptr;
        QAction* redo                 = nullptr;
        // Create
        QAction* polygonPrimitives    = nullptr;
        QAction* createJoint          = nullptr;
        // Rig
        QAction* markHierarchyParent        = nullptr;
        QAction* parentToMarkedParent       = nullptr;
        QAction* unparentSelected           = nullptr;
        QAction* bindSkin                   = nullptr;
        QAction* resetJointOrientation      = nullptr;
        QAction* alignJointOrientation      = nullptr;
        QAction* captureBindPose            = nullptr;
        QAction* captureBindPoseRecursive   = nullptr;
        // Animation
        QAction* duplicateKey         = nullptr;
        QAction* shiftKeysLeft        = nullptr;
        QAction* shiftKeysRight       = nullptr;
        QAction* previousKey          = nullptr;
        QAction* nextKey              = nullptr;
        // View
        QAction* resetCamera          = nullptr;
        QAction* frameScene           = nullptr;
        QAction* frameSelected        = nullptr;
        QAction* wireframe            = nullptr;
        QAction* showAxis             = nullptr;
        QAction* backfaceCulling      = nullptr;
        QAction* perspectiveCamera    = nullptr;
        QAction* frontCamera          = nullptr;
        QAction* backCamera           = nullptr;
        QAction* leftCamera           = nullptr;
        QAction* rightCamera          = nullptr;
        QAction* topCamera            = nullptr;
        QAction* bottomCamera         = nullptr;
        QAction* restoreWorkspaceLayout = nullptr;
        // Transform
        QAction* translate            = nullptr;
        QAction* rotate               = nullptr;
        QAction* scale                = nullptr;
        QAction* worldAxis            = nullptr;
        QAction* localAxis            = nullptr;
        // Windows
        QAction* scriptEditor               = nullptr;
        QAction* graphEditor                = nullptr;
        QAction* workspaceDefault           = nullptr;
        QAction* workspaceAnimation         = nullptr;
        QAction* workspaceRigging           = nullptr;
        QAction* workspaceSaveLayout        = nullptr;
        QAction* workspaceDeleteLayout      = nullptr;
        QMenu*   workspaceMenu              = nullptr;
        QVector<QAction*> workspaceUserLayoutActions;
    };

    // ── Input: callbacks wired to each menu action ────────────────────────────

    struct Bindings
    {
        // File
        std::function<void()> newScene;
        std::function<void()> openScene;
        std::function<void()> saveScene;
        std::function<void()> saveSceneAs;
        std::function<void()> incrementAndSave;
        std::function<void()> archiveScene;
        std::function<void()> savePreferences;
        std::function<void()> optimizeSceneSize;
        std::function<void()> importFbx;
        std::function<void()> exportAll;
        std::function<void()> exportSelection;
        // Edit
        std::function<void()> undo;
        std::function<void()> redo;
        // Create
        std::function<void()> showPolygonPrimitives;
        std::function<void()> createJoint;
        // Rig
        std::function<void()> markHierarchyParent;
        std::function<void()> parentToMarkedParent;
        std::function<void()> unparentSelected;
        std::function<void()> bindSkin;
        std::function<void()> resetJointOrientation;
        std::function<void()> alignJointOrientation;
        std::function<void()> captureBindPose;
        std::function<void()> captureBindPoseRecursive;
        // Animation
        std::function<void()> duplicateKey;
        std::function<void()> shiftKeysLeft;
        std::function<void()> shiftKeysRight;
        std::function<void()> previousKey;
        std::function<void()> nextKey;
        // View
        std::function<void()> resetCamera;
        std::function<void()> frameScene;
        std::function<void()> frameSelected;
        std::function<void(bool)> setWireframe;
        std::function<void(bool)> setAxisVisibility;
        std::function<void(bool)> setBackfaceCulling;
        std::function<void()> setCameraPerspective;
        std::function<void()> setCameraFront;
        std::function<void()> setCameraBack;
        std::function<void()> setCameraLeft;
        std::function<void()> setCameraRight;
        std::function<void()> setCameraTop;
        std::function<void()> setCameraBottom;
        std::function<void()> restoreDefaultLayout;
        // Transform
        std::function<void()> setTranslate;
        std::function<void()> setRotate;
        std::function<void()> setScale;
        std::function<void()> setWorldAxis;
        std::function<void()> setLocalAxis;
        // Windows
        std::function<void()> showScriptEditor;
        std::function<void()> showGraphEditor;
        // Workspace — pointer needed to sync preset checkmarks via signal
        WorkspaceManager* workspaceManager = nullptr;
        std::function<void()>              saveWorkspaceLayout;
        std::function<void()>              deleteWorkspaceLayout;
        std::function<QStringList()>       userLayoutNames;
        std::function<void(const QString&)> restoreUserLayout;
        // Status bar message (for layout save/delete confirmations)
        std::function<void(const QString&, int)> showStatusMessage;
    };

    explicit EditorMenuBar(QMainWindow* mainWindow, QObject* parent = nullptr);

    // Build all menus and wire bindings. Call once after construction.
    void build(const Bindings& bindings);

    Actions&       actions()       { return actions_; }
    const Actions& actions() const { return actions_; }

private:
    void buildFileMenu(const Bindings& b);
    void buildEditMenu(const Bindings& b);
    void buildCreateMenu(const Bindings& b);
    void buildRigMenu(const Bindings& b);
    void buildAnimationMenu(const Bindings& b);
    void buildViewMenu(const Bindings& b);
    void buildTransformMenu(const Bindings& b);
    void buildWindowsMenu(const Bindings& b);
    void rebuildUserLayoutActions();
    void openSaveLayoutDialog();
    void openDeleteLayoutDialog();

    QMainWindow* mainWindow_ = nullptr;
    Actions      actions_;
    Bindings     bindings_; // retained for rebuildUserLayoutActions + dialogs
};
