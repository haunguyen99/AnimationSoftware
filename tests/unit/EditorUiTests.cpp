#include <QAction>
#include <QCheckBox>
#include <QDir>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QFileInfo>
#include <QLabel>
#include <QListWidget>
#include <QOpenGLWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QApplication>
#include <QtTest>

#include "EditorPlaybackController.h"
#include "EditorAnimationEngineFacade.h"
#include "EditorSceneRuntimeController.h"
#include "EditorViewportSceneController.h"
#include "MainWindow.h"
#include "ScriptCommandSystem.h"
#include "ViewportWorkspaceWidget.h"

class EditorUiTests : public QObject
{
    Q_OBJECT

private slots:
    void restoreDefaultLayoutResetsFloatingDocks();
    void transformToolbarUpdatesViewportToolState();
    void viewMenuSwitchesViewportCameraPresets();
    void spaceTogglesQuadViewAndMaximizesActiveViewport();
    void editMenuUndoRedoRestoresCreatedPrimitive();
    void scriptEditorExecutesPrimitiveCommand();
    void scriptEditorLogsChannelBoxChanges();
    void scriptEditorExecutesTimelineCommands();
    void scriptEditorExecutesObjectCommands();
    void scriptEditorExecutesViewToolAndFileCommands();
    void scriptEditorExecutesSceneFileCommands();
    void scriptCommandRegistryDispatchesFileImportCommand();
    void scriptCommandRegistryDispatchesSetKeyframeCommand();
    void scriptCommandRegistryDispatchesDeleteKeyCommand();
    void scriptCommandRegistryDispatchesCopyAndShiftKeyCommands();
    void scriptCommandRegistryDispatchesAutoKeyCommand();
    void scriptCommandRegistryDispatchesJointHierarchyCommands();
    void scriptCommandRegistryDispatchesJointOrientationAndBindPoseCommands();
    void scriptCommandRegistryDispatchesBindSkinCommand();
    void hierarchyActionsParentAndUnparentJoints();
    void bindSkinActionBindsMeshToMarkedJoint();
    void hierarchyActionsExposeMayaLikeShortcuts();
    void leftMouseDragReparentsOutlinerItems();
    void jointInspectorEditsOrientationAndCapturesBindPose();
    void timelineUiShowsKeyframeFeedback();
    void playbackControllerRoutesTimeIntentsAndTicks();
    void animationEngineFacadeAppliesKeyEditsAndScriptBindings();
    void sceneRuntimeControllerAppliesSceneFrameAndSelection();
    void viewportSceneControllerRoutesSceneMutations();

private:
    void createCubeUpdatesOutlinerAndChannelBox();
};

void EditorUiTests::createCubeUpdatesOutlinerAndChannelBox()
{
    MainWindow window;
    window.show();
    QTRY_VERIFY(window.isVisible());
    QTest::qWait(200);

    auto* openPrimitivesAction = window.findChild<QAction*>("polygonPrimitivesAction");
    auto* primitiveList = window.findChild<QListWidget*>("polygonPrimitivesList");
    auto* createPrimitiveButton = window.findChild<QPushButton*>("createPrimitiveButton");
    auto* outlinerTree = window.findChild<QTreeWidget*>("outlinerTree");
    auto* channelObjectNameLabel = window.findChild<QLabel*>("channelObjectNameLabel");
    auto* translateXSpinBox = window.findChild<QDoubleSpinBox*>("translateXSpinBox");
    auto* visibilityCheckBox = window.findChild<QCheckBox*>("visibilityCheckBox");
    auto* polygonDock = window.findChild<QDockWidget*>("PolygonPrimitivesDock");

    QVERIFY(openPrimitivesAction != nullptr);
    QVERIFY(primitiveList != nullptr);
    QVERIFY(createPrimitiveButton != nullptr);
    QVERIFY(outlinerTree != nullptr);
    QVERIFY(channelObjectNameLabel != nullptr);
    QVERIFY(translateXSpinBox != nullptr);
    QVERIFY(visibilityCheckBox != nullptr);
    QVERIFY(polygonDock != nullptr);

    openPrimitivesAction->trigger();
    QTRY_VERIFY(polygonDock->isVisible());

    const QList<QListWidgetItem*> matches = primitiveList->findItems("Cube", Qt::MatchExactly);
    QVERIFY(!matches.isEmpty());
    primitiveList->setCurrentItem(matches.first());
    createPrimitiveButton->click();

    QTRY_COMPARE(outlinerTree->topLevelItemCount(), 1);
    QVERIFY(outlinerTree->topLevelItem(0) != nullptr);
    QCOMPARE(outlinerTree->topLevelItem(0)->text(0), QString("pCube1"));
    QTRY_COMPARE(channelObjectNameLabel->text(), QString("pCube1"));
    QCOMPARE(translateXSpinBox->value(), 0.0);
    QVERIFY(visibilityCheckBox->isChecked());
    QVERIFY(!polygonDock->isVisible());
}

void EditorUiTests::restoreDefaultLayoutResetsFloatingDocks()
{
    MainWindow window;
    window.show();
    QTRY_VERIFY(window.isVisible());
    QTest::qWait(200);

    auto* viewportDock = window.findChild<QDockWidget*>("ViewportDock");
    auto* outlinerDock = window.findChild<QDockWidget*>("OutlinerDock");
    auto* inspectorDock = window.findChild<QDockWidget*>("InspectorDock");
    auto* restoreAction = window.findChild<QAction*>("restoreWorkspaceLayoutAction");

    QVERIFY(window.centralWidget() != nullptr);
    QVERIFY(outlinerDock != nullptr);
    QVERIFY(inspectorDock != nullptr);
    QVERIFY(restoreAction != nullptr);

    outlinerDock->setFloating(true);
    inspectorDock->hide();
    QVERIFY(outlinerDock->isFloating());
    QVERIFY(!inspectorDock->isVisible());

    restoreAction->trigger();

    QTRY_VERIFY(!outlinerDock->isFloating());
    QTRY_VERIFY(!inspectorDock->isFloating());
    QTRY_VERIFY(outlinerDock->isVisible());
    QTRY_VERIFY(inspectorDock->isVisible());
}

void EditorUiTests::transformToolbarUpdatesViewportToolState()
{
    MainWindow window;
    window.show();
    QTRY_VERIFY(window.isVisible());
    QTest::qWait(200);

    auto* viewport = dynamic_cast<ViewportWorkspaceWidget*>(window.findChild<QWidget*>("viewportWidget"));
    auto* translateAction = window.findChild<QAction*>("translateAction");
    auto* rotateAction = window.findChild<QAction*>("rotateAction");
    auto* scaleAction = window.findChild<QAction*>("scaleAction");
    auto* worldAxisAction = window.findChild<QAction*>("worldAxisAction");
    auto* localAxisAction = window.findChild<QAction*>("localAxisAction");

    QVERIFY(viewport != nullptr);
    QVERIFY(translateAction != nullptr);
    QVERIFY(rotateAction != nullptr);
    QVERIFY(scaleAction != nullptr);
    QVERIFY(worldAxisAction != nullptr);
    QVERIFY(localAxisAction != nullptr);

    QCOMPARE(viewport->transformMode(), ViewportWorkspaceWidget::TransformMode::Translate);
    QCOMPARE(viewport->axisOrientation(), ViewportWorkspaceWidget::AxisOrientation::World);
    QVERIFY(translateAction->isChecked());
    QVERIFY(worldAxisAction->isChecked());

    rotateAction->trigger();
    QCOMPARE(viewport->transformMode(), ViewportWorkspaceWidget::TransformMode::Rotate);
    QVERIFY(rotateAction->isChecked());
    QVERIFY(!translateAction->isChecked());

    scaleAction->trigger();
    QCOMPARE(viewport->transformMode(), ViewportWorkspaceWidget::TransformMode::Scale);
    QVERIFY(scaleAction->isChecked());
    QVERIFY(!rotateAction->isChecked());

    localAxisAction->trigger();
    QCOMPARE(viewport->axisOrientation(), ViewportWorkspaceWidget::AxisOrientation::Local);
    QVERIFY(localAxisAction->isChecked());
    QVERIFY(!worldAxisAction->isChecked());

    worldAxisAction->trigger();
    QCOMPARE(viewport->axisOrientation(), ViewportWorkspaceWidget::AxisOrientation::World);
    QVERIFY(worldAxisAction->isChecked());
    QVERIFY(!localAxisAction->isChecked());
}

void EditorUiTests::viewMenuSwitchesViewportCameraPresets()
{
    MainWindow window;
    window.show();
    QTRY_VERIFY(window.isVisible());
    QTest::qWait(200);

    auto* viewport = dynamic_cast<ViewportWorkspaceWidget*>(window.findChild<QWidget*>("viewportWidget"));
    auto* perspectiveCameraAction = window.findChild<QAction*>("perspectiveCameraAction");
    auto* frontCameraAction = window.findChild<QAction*>("frontCameraAction");
    auto* backCameraAction = window.findChild<QAction*>("backCameraAction");
    auto* leftCameraAction = window.findChild<QAction*>("leftCameraAction");
    auto* rightCameraAction = window.findChild<QAction*>("rightCameraAction");
    auto* topCameraAction = window.findChild<QAction*>("topCameraAction");
    auto* bottomCameraAction = window.findChild<QAction*>("bottomCameraAction");

    QVERIFY(viewport != nullptr);
    QVERIFY(perspectiveCameraAction != nullptr);
    QVERIFY(frontCameraAction != nullptr);
    QVERIFY(backCameraAction != nullptr);
    QVERIFY(leftCameraAction != nullptr);
    QVERIFY(rightCameraAction != nullptr);
    QVERIFY(topCameraAction != nullptr);
    QVERIFY(bottomCameraAction != nullptr);

    QCOMPARE(viewport->cameraViewPreset(), ViewportWorkspaceWidget::CameraViewPreset::Perspective);
    QCOMPARE(viewport->cameraViewLabel(), QString("persp"));
    QVERIFY(perspectiveCameraAction->isChecked());

    frontCameraAction->trigger();
    QCOMPARE(viewport->cameraViewPreset(), ViewportWorkspaceWidget::CameraViewPreset::Front);
    QCOMPARE(viewport->cameraViewLabel(), QString("front"));
    QVERIFY(frontCameraAction->isChecked());
    QVERIFY(!perspectiveCameraAction->isChecked());

    topCameraAction->trigger();
    QCOMPARE(viewport->cameraViewPreset(), ViewportWorkspaceWidget::CameraViewPreset::Top);
    QCOMPARE(viewport->cameraViewLabel(), QString("top"));
    QVERIFY(topCameraAction->isChecked());
    QVERIFY(!frontCameraAction->isChecked());

    backCameraAction->trigger();
    QCOMPARE(viewport->cameraViewPreset(), ViewportWorkspaceWidget::CameraViewPreset::Back);
    QCOMPARE(viewport->cameraViewLabel(), QString("back"));
    QVERIFY(backCameraAction->isChecked());

    leftCameraAction->trigger();
    QCOMPARE(viewport->cameraViewPreset(), ViewportWorkspaceWidget::CameraViewPreset::Left);
    QCOMPARE(viewport->cameraViewLabel(), QString("left"));
    QVERIFY(leftCameraAction->isChecked());

    rightCameraAction->trigger();
    QCOMPARE(viewport->cameraViewPreset(), ViewportWorkspaceWidget::CameraViewPreset::Right);
    QCOMPARE(viewport->cameraViewLabel(), QString("side"));
    QVERIFY(rightCameraAction->isChecked());

    bottomCameraAction->trigger();
    QCOMPARE(viewport->cameraViewPreset(), ViewportWorkspaceWidget::CameraViewPreset::Bottom);
    QCOMPARE(viewport->cameraViewLabel(), QString("bottom"));
    QVERIFY(bottomCameraAction->isChecked());

    perspectiveCameraAction->trigger();
    QCOMPARE(viewport->cameraViewPreset(), ViewportWorkspaceWidget::CameraViewPreset::Perspective);
    QCOMPARE(viewport->cameraViewLabel(), QString("persp"));
    QVERIFY(perspectiveCameraAction->isChecked());
    QVERIFY(!bottomCameraAction->isChecked());
}

void EditorUiTests::spaceTogglesQuadViewAndMaximizesActiveViewport()
{
    MainWindow window;
    window.show();
    QTRY_VERIFY(window.isVisible());
    QTest::qWait(200);

    auto* viewportWorkspace = dynamic_cast<ViewportWorkspaceWidget*>(window.findChild<QWidget*>("viewportWidget"));
    auto* perspectiveViewport = window.findChild<QWidget*>("perspectiveViewportWidget");
    auto* frontViewport = window.findChild<QWidget*>("frontViewportWidget");

    QVERIFY(viewportWorkspace != nullptr);
    QVERIFY(perspectiveViewport != nullptr);
    QVERIFY(frontViewport != nullptr);

    QVERIFY(!viewportWorkspace->quadViewEnabled());
    QCOMPARE(viewportWorkspace->activeCameraViewPreset(), ViewportWorkspaceWidget::CameraViewPreset::Perspective);

    QTest::keyClick(viewportWorkspace, Qt::Key_Space);
    QTRY_VERIFY(viewportWorkspace->quadViewEnabled());
    QVERIFY(perspectiveViewport->isVisible());
    QVERIFY(frontViewport->isVisible());

    QTest::mouseClick(frontViewport, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QCOMPARE(viewportWorkspace->activeCameraViewPreset(), ViewportWorkspaceWidget::CameraViewPreset::Front);

    QTest::keyClick(viewportWorkspace, Qt::Key_Space);
    QTRY_VERIFY(!viewportWorkspace->quadViewEnabled());
    QCOMPARE(viewportWorkspace->activeCameraViewPreset(), ViewportWorkspaceWidget::CameraViewPreset::Front);
    QTRY_VERIFY(viewportWorkspace->isCameraViewVisible(ViewportWorkspaceWidget::CameraViewPreset::Front));
    QVERIFY(!viewportWorkspace->isCameraViewVisible(ViewportWorkspaceWidget::CameraViewPreset::Perspective));
}

void EditorUiTests::editMenuUndoRedoRestoresCreatedPrimitive()
{
    MainWindow window;
    window.show();
    QTRY_VERIFY(window.isVisible());
    QTest::qWait(200);

    auto* openPrimitivesAction = window.findChild<QAction*>("polygonPrimitivesAction");
    auto* primitiveList = window.findChild<QListWidget*>("polygonPrimitivesList");
    auto* createPrimitiveButton = window.findChild<QPushButton*>("createPrimitiveButton");
    auto* outlinerTree = window.findChild<QTreeWidget*>("outlinerTree");
    auto* undoAction = window.findChild<QAction*>("undoAction");
    auto* redoAction = window.findChild<QAction*>("redoAction");

    QVERIFY(openPrimitivesAction != nullptr);
    QVERIFY(primitiveList != nullptr);
    QVERIFY(createPrimitiveButton != nullptr);
    QVERIFY(outlinerTree != nullptr);
    QVERIFY(undoAction != nullptr);
    QVERIFY(redoAction != nullptr);

    const QList<QListWidgetItem*> matches = primitiveList->findItems("Cube", Qt::MatchExactly);
    QVERIFY(!matches.isEmpty());

    openPrimitivesAction->trigger();
    primitiveList->setCurrentItem(matches.first());
    createPrimitiveButton->click();

    QTRY_COMPARE(outlinerTree->topLevelItemCount(), 1);
    QCOMPARE(outlinerTree->topLevelItem(0)->text(0), QString("pCube1"));
    QVERIFY(undoAction->isEnabled());

    undoAction->trigger();
    QTRY_COMPARE(outlinerTree->topLevelItemCount(), 1);
    QCOMPARE(outlinerTree->topLevelItem(0)->text(0), QString("No scene loaded"));
    QVERIFY(redoAction->isEnabled());

    redoAction->trigger();
    QTRY_COMPARE(outlinerTree->topLevelItemCount(), 1);
    QCOMPARE(outlinerTree->topLevelItem(0)->text(0), QString("pCube1"));
}

void EditorUiTests::scriptEditorExecutesPrimitiveCommand()
{
    MainWindow window;
    window.show();
    QTRY_VERIFY(window.isVisible());
    QTest::qWait(200);

    auto* openScriptEditorAction = window.findChild<QAction*>("scriptEditorAction");
    auto* scriptDock = window.findChild<QDockWidget*>("ScriptEditorDock");
    auto* scriptInput = window.findChild<QPlainTextEdit*>("scriptInputTextEdit");
    auto* scriptHistory = window.findChild<QPlainTextEdit*>("scriptHistoryTextEdit");
    auto* outlinerTree = window.findChild<QTreeWidget*>("outlinerTree");
    auto* channelObjectNameLabel = window.findChild<QLabel*>("channelObjectNameLabel");
    auto* executeAllAction = window.findChild<QAction*>("scriptExecuteAllAction");

    QVERIFY(openScriptEditorAction != nullptr);
    QVERIFY(scriptDock != nullptr);
    QVERIFY(scriptInput != nullptr);
    QVERIFY(scriptHistory != nullptr);
    QVERIFY(outlinerTree != nullptr);
    QVERIFY(channelObjectNameLabel != nullptr);
    QVERIFY(executeAllAction != nullptr);

    openScriptEditorAction->trigger();
    QTRY_VERIFY(scriptDock->isVisible());

    scriptInput->setPlainText("select -cl;\npolyCube -w 1 -h 1 -d 1;");
    executeAllAction->trigger();

    QTRY_COMPARE(outlinerTree->topLevelItemCount(), 1);
    QVERIFY(outlinerTree->topLevelItem(0) != nullptr);
    QCOMPARE(outlinerTree->topLevelItem(0)->text(0), QString("pCube1"));
    QTRY_COMPARE(channelObjectNameLabel->text(), QString("pCube1"));
    QVERIFY(scriptHistory->toPlainText().contains("polyCube -w 1 -h 1 -d 1;"));
    QVERIFY(scriptHistory->toPlainText().contains("// Result: pCube1 pCube1Shape //"));
}

void EditorUiTests::scriptEditorLogsChannelBoxChanges()
{
    ScriptCommandRegistry registry;
    ScriptCommandContext context;
    QString objectName;
    QString attributeName;
    QList<double> values;
    context.setAttribute = [&objectName, &attributeName, &values](const QString& incomingObjectName, const QString& incomingAttributeName, const QList<double>& incomingValues) {
        objectName = incomingObjectName;
        attributeName = incomingAttributeName;
        values = incomingValues;
        return true;
    };

    ScriptCommandExecution execution;
    QVERIFY(registry.execute("setAttr \"pCube1.translate\" 3.000 0.000 0.000;", context, &execution));
    QCOMPARE(objectName, QString("pCube1"));
    QCOMPARE(attributeName, QString("translate"));
    QCOMPARE(values, QList<double>({ 3.0, 0.0, 0.0 }));
    QCOMPARE(execution.resultLine, QString("// Result: pCube1.translate updated //"));
}

void EditorUiTests::scriptEditorExecutesTimelineCommands()
{
    ScriptCommandRegistry registry;
    ScriptCommandContext context;
    int currentFrame = 0;
    int startFrame = 0;
    int endFrame = 0;
    bool playing = false;
    context.setCurrentFrame = [&currentFrame](int frame) { currentFrame = frame; };
    context.setPlaybackRange = [&startFrame, &endFrame](int start, int end) {
        startFrame = start;
        endFrame = end;
    };
    context.setPlaybackState = [&playing](bool state) { playing = state; };

    ScriptCommandExecution execution;
    QVERIFY(registry.execute("playbackOptions -min 5 -max 48;", context, &execution));
    QCOMPARE(startFrame, 5);
    QCOMPARE(endFrame, 48);
    QCOMPARE(execution.resultLine, QString("// Result: playback range 5 to 48 //"));

    QVERIFY(registry.execute("currentTime 12;", context, &execution));
    QCOMPARE(currentFrame, 12);
    QCOMPARE(execution.resultLine, QString("// Result: current frame 12 //"));

    QVERIFY(registry.execute("play -state on;", context, &execution));
    QVERIFY(playing);
    QCOMPARE(execution.resultLine, QString("// Result: playback started //"));

    QVERIFY(registry.execute("play -state off;", context, &execution));
    QVERIFY(!playing);
    QCOMPARE(execution.resultLine, QString("// Result: playback stopped //"));
}

void EditorUiTests::scriptEditorExecutesObjectCommands()
{
    ScriptCommandRegistry registry;
    ScriptCommandContext context;
    QString renamedObject;
    QString duplicatedObject;
    QString groupedObject;
    QString deletedObject;
    context.renameObject = [&renamedObject](const QString& sourceName, const QString& newName) {
        renamedObject = QString("%1->%2").arg(sourceName, newName);
        return newName;
    };
    context.duplicateObject = [&duplicatedObject](const QString& sourceName) {
        duplicatedObject = sourceName;
        return QString("%1Copy").arg(sourceName);
    };
    context.groupObject = [&groupedObject](const QString& sourceName) {
        groupedObject = sourceName;
        return "group1";
    };
    context.deleteObject = [&deletedObject](const QString& sourceName) {
        deletedObject = sourceName;
        return true;
    };

    ScriptCommandExecution execution;
    QVERIFY(registry.execute("rename pCube1 heroCube;", context, &execution));
    QCOMPARE(renamedObject, QString("pCube1->heroCube"));
    QCOMPARE(execution.resultLine, QString("// Result: heroCube //"));

    QVERIFY(registry.execute("duplicate heroCube;", context, &execution));
    QCOMPARE(duplicatedObject, QString("heroCube"));
    QCOMPARE(execution.resultLine, QString("// Result: heroCubeCopy //"));

    QVERIFY(registry.execute("group heroCubeCopy;", context, &execution));
    QCOMPARE(groupedObject, QString("heroCubeCopy"));
    QCOMPARE(execution.resultLine, QString("// Result: group1 //"));

    QVERIFY(registry.execute("delete heroCube;", context, &execution));
    QCOMPARE(deletedObject, QString("heroCube"));
    QCOMPARE(execution.resultLine, QString("// Result: object deleted //"));
}

void EditorUiTests::scriptEditorExecutesViewToolAndFileCommands()
{
    ScriptCommandRegistry registry;
    ScriptCommandContext context;
    QString activatedTool;
    QString framedTarget;
    bool resetCameraCalled = false;
    bool newSceneCalled = false;
    context.activateTool = [&activatedTool](const QString& toolName) {
        activatedTool = toolName;
        return true;
    };
    context.frameView = [&framedTarget](const QString& targetName) {
        framedTarget = targetName;
        return true;
    };
    context.resetCamera = [&resetCameraCalled]() { resetCameraCalled = true; };
    context.newScene = [&newSceneCalled]() { newSceneCalled = true; };

    ScriptCommandExecution execution;
    QVERIFY(registry.execute("setToolTo RotateSuperContext;", context, &execution));
    QCOMPARE(activatedTool, QString("RotateSuperContext"));
    QCOMPARE(execution.resultLine, QString("// Result: rotate tool //"));

    QVERIFY(registry.execute("viewFit pCube1;", context, &execution));
    QCOMPARE(framedTarget, QString("pCube1"));
    QCOMPARE(execution.resultLine, QString("// Result: framed pCube1 //"));

    QVERIFY(registry.execute("viewSet -home;", context, &execution));
    QVERIFY(resetCameraCalled);
    QCOMPARE(execution.resultLine, QString("// Result: camera reset //"));

    QVERIFY(registry.execute("file -f -new;", context, &execution));
    QVERIFY(newSceneCalled);
    QCOMPARE(execution.resultLine, QString("// Result: new scene //"));
}

void EditorUiTests::scriptEditorExecutesSceneFileCommands()
{
    ScriptCommandRegistry registry;
    ScriptCommandContext context;
    QString savedPath;
    QString openedPath;
    context.saveSceneFile = [&savedPath](const QString& filePath) {
        savedPath = filePath;
        return true;
    };
    context.openSceneFile = [&openedPath](const QString& filePath) {
        openedPath = filePath;
        return true;
    };

    const QString scenePath = "E:/Animation Software/script editor scene.phoenixscene";
    ScriptCommandExecution execution;
    QVERIFY(registry.execute(QString("file -save \"%1\";").arg(scenePath), context, &execution));
    QCOMPARE(savedPath, scenePath);
    QCOMPARE(execution.resultLine, QString("// Result: saved %1 //").arg(scenePath));

    QVERIFY(registry.execute(QString("file -o \"%1\";").arg(scenePath), context, &execution));
    QCOMPARE(openedPath, scenePath);
    QCOMPARE(execution.resultLine, QString("// Result: opened %1 //").arg(scenePath));
}

void EditorUiTests::scriptCommandRegistryDispatchesFileImportCommand()
{
    ScriptCommandRegistry registry;
    ScriptCommandContext context;
    QString importedPath;
    context.importSceneFile = [&importedPath](const QString& filePath) {
        importedPath = filePath;
        return true;
    };

    ScriptCommandExecution execution;
    QVERIFY(registry.execute("file -import \"E:/Animation Software/assets/sample/box static.fbx\";", context, &execution));
    QCOMPARE(importedPath, QString("E:/Animation Software/assets/sample/box static.fbx"));
    QCOMPARE(execution.resultLine, QString("// Result: imported E:/Animation Software/assets/sample/box static.fbx //"));
}

void EditorUiTests::scriptCommandRegistryDispatchesSetKeyframeCommand()
{
    ScriptCommandRegistry registry;
    ScriptCommandContext context;
    QString keyedObject;
    int keyedFrame = -1;
    context.setKeyframe = [&keyedObject, &keyedFrame](const QString& objectName, int frame) {
        keyedObject = objectName;
        keyedFrame = frame;
        return true;
    };

    ScriptCommandExecution execution;
    QVERIFY(registry.execute("setKeyframe pCube1 -t 12;", context, &execution));
    QCOMPARE(keyedObject, QString("pCube1"));
    QCOMPARE(keyedFrame, 12);
    QCOMPARE(execution.resultLine, QString("// Result: key set on pCube1 at frame 12 //"));
}

void EditorUiTests::scriptCommandRegistryDispatchesDeleteKeyCommand()
{
    ScriptCommandRegistry registry;
    ScriptCommandContext context;
    QString keyedObject;
    int keyedFrame = -1;
    context.deleteKeyframe = [&keyedObject, &keyedFrame](const QString& objectName, int frame) {
        keyedObject = objectName;
        keyedFrame = frame;
        return true;
    };

    ScriptCommandExecution execution;
    QVERIFY(registry.execute("cutKey pCube1 -t 12;", context, &execution));
    QCOMPARE(keyedObject, QString("pCube1"));
    QCOMPARE(keyedFrame, 12);
    QCOMPARE(execution.resultLine, QString("// Result: deleted key on pCube1 at frame 12 //"));
}

void EditorUiTests::scriptCommandRegistryDispatchesCopyAndShiftKeyCommands()
{
    ScriptCommandRegistry registry;
    ScriptCommandContext context;
    QString copiedObject;
    int copySourceFrame = -1;
    int copyTargetFrame = -1;
    QString shiftedObject;
    int shiftedDelta = 0;
    context.copyKeyframe = [&copiedObject, &copySourceFrame, &copyTargetFrame](const QString& objectName, int sourceFrame, int targetFrame) {
        copiedObject = objectName;
        copySourceFrame = sourceFrame;
        copyTargetFrame = targetFrame;
        return true;
    };
    context.shiftKeyframes = [&shiftedObject, &shiftedDelta](const QString& objectName, int frameDelta) {
        shiftedObject = objectName;
        shiftedDelta = frameDelta;
        return true;
    };

    ScriptCommandExecution execution;
    QVERIFY(registry.execute("copyKey pCube1 -t 10 -to 11;", context, &execution));
    QCOMPARE(copiedObject, QString("pCube1"));
    QCOMPARE(copySourceFrame, 10);
    QCOMPARE(copyTargetFrame, 11);
    QCOMPARE(execution.resultLine, QString("// Result: copied key on pCube1 from frame 10 to 11 //"));

    QVERIFY(registry.execute("shiftKey pCube1 -by -1;", context, &execution));
    QCOMPARE(shiftedObject, QString("pCube1"));
    QCOMPARE(shiftedDelta, -1);
    QCOMPARE(execution.resultLine, QString("// Result: shifted keys on pCube1 by -1 //"));
}

void EditorUiTests::scriptCommandRegistryDispatchesAutoKeyCommand()
{
    ScriptCommandRegistry registry;
    ScriptCommandContext context;
    bool autoKeyEnabled = false;
    context.setAutoKey = [&autoKeyEnabled](bool enabled) {
        autoKeyEnabled = enabled;
    };

    ScriptCommandExecution execution;
    QVERIFY(registry.execute("autoKeyframe -state on;", context, &execution));
    QVERIFY(autoKeyEnabled);
    QCOMPARE(execution.resultLine, QString("// Result: auto key on //"));

    QVERIFY(registry.execute("autoKeyframe -state off;", context, &execution));
    QVERIFY(!autoKeyEnabled);
    QCOMPARE(execution.resultLine, QString("// Result: auto key off //"));
}

void EditorUiTests::scriptCommandRegistryDispatchesJointHierarchyCommands()
{
    ScriptCommandRegistry registry;
    ScriptCommandContext context;
    QString createdJoint;
    QString parentedChild;
    QString parentedParent;
    QString unparentedChild;
    context.createJoint = [&createdJoint](const QString& name) {
        createdJoint = name.isEmpty() ? QString("joint1") : name;
        return createdJoint;
    };
    context.parentObject = [&parentedChild, &parentedParent](const QString& childName, const QString& parentName) {
        parentedChild = childName;
        parentedParent = parentName;
        return true;
    };
    context.unparentObject = [&unparentedChild](const QString& childName) {
        unparentedChild = childName;
        return true;
    };

    ScriptCommandExecution execution;
    QVERIFY(registry.execute("joint -name shoulder_jnt;", context, &execution));
    QCOMPARE(createdJoint, QString("shoulder_jnt"));
    QCOMPARE(execution.resultLine, QString("// Result: shoulder_jnt //"));

    QVERIFY(registry.execute("parent wrist_jnt elbow_jnt;", context, &execution));
    QCOMPARE(parentedChild, QString("wrist_jnt"));
    QCOMPARE(parentedParent, QString("elbow_jnt"));
    QCOMPARE(execution.resultLine, QString("// Result: parented wrist_jnt under elbow_jnt //"));

    QVERIFY(registry.execute("unparent wrist_jnt;", context, &execution));
    QCOMPARE(unparentedChild, QString("wrist_jnt"));
    QCOMPARE(execution.resultLine, QString("// Result: unparented wrist_jnt //"));
}

void EditorUiTests::scriptCommandRegistryDispatchesJointOrientationAndBindPoseCommands()
{
    ScriptCommandRegistry registry;
    ScriptCommandContext context;
    QString orientedJoint;
    QVector3D orientedEuler;
    QString resetJoint;
    QString alignedJoint;
    QString bindPoseJoint;
    bool bindPoseRecursive = false;
    context.setJointOrientation = [&orientedJoint, &orientedEuler](const QString& objectName, const QVector3D& eulerDegrees) {
        orientedJoint = objectName;
        orientedEuler = eulerDegrees;
        return true;
    };
    context.resetJointOrientation = [&resetJoint](const QString& objectName) {
        resetJoint = objectName;
        return true;
    };
    context.alignJointOrientationToChild = [&alignedJoint](const QString& objectName) {
        alignedJoint = objectName;
        return true;
    };
    context.captureBindPose = [&bindPoseJoint, &bindPoseRecursive](const QString& objectName, bool recursive) {
        bindPoseJoint = objectName;
        bindPoseRecursive = recursive;
        return true;
    };

    ScriptCommandExecution execution;
    QVERIFY(registry.execute("jointOrient shoulder_jnt -euler 0 45 90;", context, &execution));
    QCOMPARE(orientedJoint, QString("shoulder_jnt"));
    QCOMPARE(orientedEuler, QVector3D(0.0f, 45.0f, 90.0f));
    QCOMPARE(execution.resultLine, QString("// Result: joint orientation updated on shoulder_jnt //"));

    QVERIFY(registry.execute("jointOrient -reset shoulder_jnt;", context, &execution));
    QCOMPARE(resetJoint, QString("shoulder_jnt"));
    QCOMPARE(execution.resultLine, QString("// Result: reset joint orientation on shoulder_jnt //"));

    QVERIFY(registry.execute("jointOrient shoulder_jnt -alignToChild;", context, &execution));
    QCOMPARE(alignedJoint, QString("shoulder_jnt"));
    QCOMPARE(execution.resultLine, QString("// Result: aligned joint orientation on shoulder_jnt //"));

    QVERIFY(registry.execute("bindPose -capture -recursive shoulder_jnt;", context, &execution));
    QCOMPARE(bindPoseJoint, QString("shoulder_jnt"));
    QVERIFY(bindPoseRecursive);
    QCOMPARE(execution.resultLine, QString("// Result: captured bind pose on shoulder_jnt recursively //"));
}

void EditorUiTests::scriptCommandRegistryDispatchesBindSkinCommand()
{
    ScriptCommandRegistry registry;
    ScriptCommandContext context;
    QString boundMesh;
    QString boundJoint;
    context.bindSkin = [&boundMesh, &boundJoint](const QString& meshName, const QString& jointName) {
        boundMesh = meshName;
        boundJoint = jointName;
        return true;
    };

    ScriptCommandExecution execution;
    QVERIFY(registry.execute("bindSkin pCube1 root_jnt;", context, &execution));
    QCOMPARE(boundMesh, QString("pCube1"));
    QCOMPARE(boundJoint, QString("root_jnt"));
    QCOMPARE(execution.resultLine, QString("// Result: bound pCube1 to root_jnt //"));
}

void EditorUiTests::hierarchyActionsParentAndUnparentJoints()
{
    MainWindow window;
    window.show();
    QTRY_VERIFY(window.isVisible());
    QTest::qWait(200);

    auto* createJointAction = window.findChild<QAction*>("createJointAction");
    auto* markHierarchyParentAction = window.findChild<QAction*>("markHierarchyParentAction");
    auto* parentToMarkedParentAction = window.findChild<QAction*>("parentToMarkedParentAction");
    auto* unparentSelectedAction = window.findChild<QAction*>("unparentSelectedAction");
    auto* outlinerTree = window.findChild<QTreeWidget*>("outlinerTree");

    QVERIFY(createJointAction != nullptr);
    QVERIFY(markHierarchyParentAction != nullptr);
    QVERIFY(parentToMarkedParentAction != nullptr);
    QVERIFY(unparentSelectedAction != nullptr);
    QVERIFY(outlinerTree != nullptr);

    createJointAction->trigger();
    createJointAction->trigger();
    QTRY_COMPARE(outlinerTree->topLevelItemCount(), 1);

    QTreeWidgetItem* rootItem = outlinerTree->topLevelItem(0);
    QVERIFY(rootItem != nullptr);
    QTRY_COMPARE(rootItem->childCount(), 1);
    QTreeWidgetItem* childItem = rootItem->child(0);
    QVERIFY(childItem != nullptr);

    outlinerTree->setCurrentItem(childItem);
    QTRY_VERIFY(unparentSelectedAction->isEnabled());
    unparentSelectedAction->trigger();

    QTRY_COMPARE(outlinerTree->topLevelItemCount(), 2);
    rootItem = outlinerTree->topLevelItem(0);
    childItem = outlinerTree->topLevelItem(1);
    QVERIFY(rootItem != nullptr);
    QVERIFY(childItem != nullptr);

    outlinerTree->setCurrentItem(rootItem);
    QTRY_VERIFY(markHierarchyParentAction->isEnabled());
    markHierarchyParentAction->trigger();

    outlinerTree->setCurrentItem(childItem);
    QTRY_VERIFY(parentToMarkedParentAction->isEnabled());
    parentToMarkedParentAction->trigger();

    QTRY_COMPARE(outlinerTree->topLevelItemCount(), 1);
    rootItem = outlinerTree->topLevelItem(0);
    QVERIFY(rootItem != nullptr);
    QTRY_COMPARE(rootItem->childCount(), 1);
}

void EditorUiTests::bindSkinActionBindsMeshToMarkedJoint()
{
    MainWindow window;
    window.show();
    QTRY_VERIFY(window.isVisible());
    QTest::qWait(200);

    auto* createJointAction = window.findChild<QAction*>("createJointAction");
    auto* markHierarchyParentAction = window.findChild<QAction*>("markHierarchyParentAction");
    auto* bindSkinAction = window.findChild<QAction*>("bindSkinAction");
    auto* openPrimitivesAction = window.findChild<QAction*>("polygonPrimitivesAction");
    auto* primitiveList = window.findChild<QListWidget*>("polygonPrimitivesList");
    auto* createPrimitiveButton = window.findChild<QPushButton*>("createPrimitiveButton");
    auto* outlinerTree = window.findChild<QTreeWidget*>("outlinerTree");
    auto* skinBindingStatusLabel = window.findChild<QLabel*>("skinBindingStatusLabel");

    QVERIFY(createJointAction != nullptr);
    QVERIFY(markHierarchyParentAction != nullptr);
    QVERIFY(bindSkinAction != nullptr);
    QVERIFY(openPrimitivesAction != nullptr);
    QVERIFY(primitiveList != nullptr);
    QVERIFY(createPrimitiveButton != nullptr);
    QVERIFY(outlinerTree != nullptr);
    QVERIFY(skinBindingStatusLabel != nullptr);

    createJointAction->trigger();
    QTRY_COMPARE(outlinerTree->topLevelItemCount(), 1);
    QTreeWidgetItem* jointItem = outlinerTree->topLevelItem(0);
    QVERIFY(jointItem != nullptr);
    outlinerTree->setCurrentItem(jointItem);
    markHierarchyParentAction->trigger();

    openPrimitivesAction->trigger();
    const QList<QListWidgetItem*> matches = primitiveList->findItems("Cube", Qt::MatchExactly);
    QVERIFY(!matches.isEmpty());
    primitiveList->setCurrentItem(matches.first());
    createPrimitiveButton->click();

    QTRY_VERIFY(bindSkinAction->isEnabled());
    bindSkinAction->trigger();

    QTRY_VERIFY(skinBindingStatusLabel->text().contains("Skin binding:"));
    QVERIFY(!skinBindingStatusLabel->text().contains("not bound"));
}

void EditorUiTests::jointInspectorEditsOrientationAndCapturesBindPose()
{
    MainWindow window;
    window.show();
    QTRY_VERIFY(window.isVisible());
    QTest::qWait(200);

    auto* createJointAction = window.findChild<QAction*>("createJointAction");
    auto* outlinerTree = window.findChild<QTreeWidget*>("outlinerTree");
    auto* channelObjectNameLabel = window.findChild<QLabel*>("channelObjectNameLabel");
    auto* jointOrientYSpinBox = window.findChild<QDoubleSpinBox*>("jointOrientYSpinBox");
    auto* alignJointOrientationButton = window.findChild<QPushButton*>("alignJointOrientationButton");
    auto* captureBindPoseRecursiveButton = window.findChild<QPushButton*>("captureBindPoseRecursiveButton");
    auto* bindPoseStatusLabel = window.findChild<QLabel*>("bindPoseStatusLabel");
    auto* resetJointOrientationAction = window.findChild<QAction*>("resetJointOrientationAction");

    QVERIFY(createJointAction != nullptr);
    QVERIFY(outlinerTree != nullptr);
    QVERIFY(channelObjectNameLabel != nullptr);
    QVERIFY(jointOrientYSpinBox != nullptr);
    QVERIFY(alignJointOrientationButton != nullptr);
    QVERIFY(captureBindPoseRecursiveButton != nullptr);
    QVERIFY(bindPoseStatusLabel != nullptr);
    QVERIFY(resetJointOrientationAction != nullptr);

    createJointAction->trigger();
    createJointAction->trigger();
    QTRY_COMPARE(outlinerTree->topLevelItemCount(), 1);

    QTreeWidgetItem* rootItem = outlinerTree->topLevelItem(0);
    QVERIFY(rootItem != nullptr);
    outlinerTree->setCurrentItem(rootItem);
    QTRY_COMPARE(channelObjectNameLabel->text(), rootItem->text(0));

    QTRY_VERIFY(jointOrientYSpinBox->isEnabled());
    jointOrientYSpinBox->setValue(45.0);
    QTRY_COMPARE(jointOrientYSpinBox->value(), 45.0);

    alignJointOrientationButton->click();
    QVERIFY(resetJointOrientationAction->isEnabled());

    captureBindPoseRecursiveButton->click();
    QTRY_VERIFY(bindPoseStatusLabel->text().contains("captured", Qt::CaseInsensitive));
}

void EditorUiTests::hierarchyActionsExposeMayaLikeShortcuts()
{
    MainWindow window;

    auto* parentToMarkedParentAction = window.findChild<QAction*>("parentToMarkedParentAction");
    auto* unparentSelectedAction = window.findChild<QAction*>("unparentSelectedAction");

    QVERIFY(parentToMarkedParentAction != nullptr);
    QVERIFY(unparentSelectedAction != nullptr);
    QCOMPARE(parentToMarkedParentAction->shortcut(), QKeySequence(Qt::Key_P));
    QCOMPARE(unparentSelectedAction->shortcut(), QKeySequence(Qt::SHIFT | Qt::Key_P));
}

void EditorUiTests::leftMouseDragReparentsOutlinerItems()
{
    MainWindow window;
    window.show();
    QTRY_VERIFY(window.isVisible());
    QTest::qWait(200);

    auto* createJointAction = window.findChild<QAction*>("createJointAction");
    auto* outlinerTree = window.findChild<QTreeWidget*>("outlinerTree");

    QVERIFY(createJointAction != nullptr);
    QVERIFY(outlinerTree != nullptr);

    createJointAction->trigger();
    createJointAction->trigger();
    QTRY_COMPARE(outlinerTree->topLevelItemCount(), 1);

    QTreeWidgetItem* rootItem = outlinerTree->topLevelItem(0);
    QVERIFY(rootItem != nullptr);
    QTRY_COMPARE(rootItem->childCount(), 1);
    QTreeWidgetItem* childItem = rootItem->child(0);
    QVERIFY(childItem != nullptr);

    QWidget* viewport = outlinerTree->viewport();
    QVERIFY(viewport != nullptr);

    const QPoint childPoint = outlinerTree->visualItemRect(childItem).center();
    const QPoint emptyPoint(viewport->rect().right() - 10, viewport->rect().bottom() - 10);
    QTest::mousePress(viewport, Qt::LeftButton, Qt::NoModifier, childPoint);
    QTest::mouseMove(viewport, emptyPoint, 30);
    QTest::mouseRelease(viewport, Qt::LeftButton, Qt::NoModifier, emptyPoint);

    QTRY_COMPARE(outlinerTree->topLevelItemCount(), 2);
    rootItem = outlinerTree->topLevelItem(0);
    childItem = outlinerTree->topLevelItem(1);
    QVERIFY(rootItem != nullptr);
    QVERIFY(childItem != nullptr);

    const QPoint detachedChildPoint = outlinerTree->visualItemRect(childItem).center();
    const QPoint rootPoint = outlinerTree->visualItemRect(rootItem).center();
    QTest::mousePress(viewport, Qt::LeftButton, Qt::NoModifier, detachedChildPoint);
    QTest::mouseMove(viewport, rootPoint, 30);
    QTest::mouseRelease(viewport, Qt::LeftButton, Qt::NoModifier, rootPoint);

    QTRY_COMPARE(outlinerTree->topLevelItemCount(), 1);
    rootItem = outlinerTree->topLevelItem(0);
    QVERIFY(rootItem != nullptr);
    QTRY_COMPARE(rootItem->childCount(), 1);
}

void EditorUiTests::timelineUiShowsKeyframeFeedback()
{
    MainWindow window;
    window.show();
    QTRY_VERIFY(window.isVisible());
    QTest::qWait(200);

    auto* openPrimitivesAction = window.findChild<QAction*>("polygonPrimitivesAction");
    auto* primitiveList = window.findChild<QListWidget*>("polygonPrimitivesList");
    auto* createPrimitiveButton = window.findChild<QPushButton*>("createPrimitiveButton");
    auto* autoKeyButton = window.findChild<QPushButton*>("autoKeyButton");
    auto* setKeyButton = window.findChild<QPushButton*>("setKeyButton");
    auto* deleteKeyButton = window.findChild<QPushButton*>("deleteKeyButton");
    auto* duplicateKeyButton = window.findChild<QPushButton*>("duplicateKeyButton");
    auto* shiftKeysRightButton = window.findChild<QPushButton*>("shiftKeysRightButton");
    auto* previousKeyButton = window.findChild<QPushButton*>("previousKeyButton");
    auto* nextKeyButton = window.findChild<QPushButton*>("nextKeyButton");
    auto* timelineStatusLabel = window.findChild<QLabel*>("timelineStatusLabel");
    auto* currentFrameSpinBox = window.findChild<QSpinBox*>("currentFrameSpinBox");
    auto* keyframeTimelineWidget = window.findChild<QWidget*>("keyframeTimelineWidget");
    auto* translateXSpinBox = window.findChild<QDoubleSpinBox*>("translateXSpinBox");

    QVERIFY(openPrimitivesAction != nullptr);
    QVERIFY(primitiveList != nullptr);
    QVERIFY(createPrimitiveButton != nullptr);
    QVERIFY(autoKeyButton != nullptr);
    QVERIFY(setKeyButton != nullptr);
    QVERIFY(deleteKeyButton != nullptr);
    QVERIFY(duplicateKeyButton != nullptr);
    QVERIFY(shiftKeysRightButton != nullptr);
    QVERIFY(previousKeyButton != nullptr);
    QVERIFY(nextKeyButton != nullptr);
    QVERIFY(timelineStatusLabel != nullptr);
    QVERIFY(currentFrameSpinBox != nullptr);
    QVERIFY(keyframeTimelineWidget != nullptr);
    QVERIFY(translateXSpinBox != nullptr);

    openPrimitivesAction->trigger();
    const QList<QListWidgetItem*> matches = primitiveList->findItems("Cube", Qt::MatchExactly);
    QVERIFY(!matches.isEmpty());
    primitiveList->setCurrentItem(matches.first());
    createPrimitiveButton->click();

    QTRY_VERIFY(setKeyButton->isEnabled());
    QVERIFY(!autoKeyButton->isChecked());
    QCOMPARE(setKeyButton->text(), QString("Key Selected"));
    QVERIFY(!deleteKeyButton->isEnabled());
    QVERIFY(timelineStatusLabel->text().contains("no keys yet"));

    autoKeyButton->click();
    QTRY_VERIFY(autoKeyButton->isChecked());
    QVERIFY(timelineStatusLabel->text().contains("Auto Key on"));

    currentFrameSpinBox->setValue(10);
    QTRY_COMPARE(setKeyButton->text(), QString("Key Selected"));
    translateXSpinBox->setValue(3.0);
    QTRY_COMPARE(setKeyButton->text(), QString("Key Selected"));
    QTRY_VERIFY(deleteKeyButton->isEnabled());
    QTRY_VERIFY(duplicateKeyButton->isEnabled());
    QVERIFY(timelineStatusLabel->text().contains("frame 10 keyed"));

    duplicateKeyButton->click();
    QTRY_COMPARE(currentFrameSpinBox->value(), 11);
    QVERIFY(timelineStatusLabel->text().contains("frame 11 keyed"));

    previousKeyButton->click();
    QTRY_COMPARE(currentFrameSpinBox->value(), 10);
    nextKeyButton->click();
    QTRY_COMPARE(currentFrameSpinBox->value(), 11);

    shiftKeysRightButton->click();
    QTRY_COMPARE(currentFrameSpinBox->value(), 12);
    QVERIFY(timelineStatusLabel->text().contains("frame 12 keyed"));

    deleteKeyButton->click();
    QTRY_VERIFY(!deleteKeyButton->isEnabled());
    QVERIFY(timelineStatusLabel->text().contains("frame 12 has no key"));

    translateXSpinBox->setValue(5.0);
    QTRY_VERIFY(deleteKeyButton->isEnabled());
    QVERIFY(timelineStatusLabel->text().contains("frame 12 keyed"));

    currentFrameSpinBox->setValue(1);
    QTRY_COMPARE(setKeyButton->text(), QString("Key Selected"));
    QTRY_VERIFY(deleteKeyButton->isEnabled());
    QVERIFY(timelineStatusLabel->text().contains("frame 1 keyed"));

    currentFrameSpinBox->setValue(12);
    QTRY_COMPARE(setKeyButton->text(), QString("Key Selected"));
    QTRY_VERIFY(deleteKeyButton->isEnabled());
    QVERIFY(timelineStatusLabel->text().contains("frame 12 keyed"));
}

void EditorUiTests::playbackControllerRoutesTimeIntentsAndTicks()
{
    EditorPlaybackController controller;
    EditorAnimationState state;
    state.currentFrame = 5;
    state.playbackStartFrame = 5;
    state.playbackEndFrame = 7;

    int applyCount = 0;
    controller.bind(EditorPlaybackController::Context {
        [&state]() {
            return state;
        },
        [&state, &applyCount](const EditorAnimationFlowController::OperationResult& result) {
            ++applyCount;
            state = result.state;
        },
    });

    controller.setCurrentFrame(6, false);
    QCOMPARE(state.currentFrame, 6);

    controller.stepFrame(1);
    QCOMPARE(state.currentFrame, 7);

    controller.togglePlayback();
    QVERIFY(state.playing);

    const int applyCountAfterToggle = applyCount;
    QTRY_VERIFY(applyCount > applyCountAfterToggle);

    controller.togglePlayback();
    QVERIFY(!state.playing);
}

void EditorUiTests::animationEngineFacadeAppliesKeyEditsAndScriptBindings()
{
    Scene runtimeScene;
    const SceneObject::Id objectId = runtimeScene.createObject("pCube1");

    EditorAnimationState state;
    state.currentFrame = 5;
    state.playbackStartFrame = 0;
    state.playbackEndFrame = 24;

    int recordUndoCount = 0;
    std::uint64_t selectedObjectId = 0;
    bool selectedSyncOutliner = false;

    const auto applyState = [&state, &runtimeScene](const EditorAnimationState& nextState) {
        state = nextState;
        runtimeScene.setCurrentFrame(nextState.currentFrame);
    };

    EditorAnimationEngineFacade::Context context;
    context.flow.findObject = [&runtimeScene](std::uint64_t incomingObjectId) {
        return runtimeScene.findObject(incomingObjectId);
    };
    context.flow.sceneSnapshot = [&runtimeScene]() {
        return runtimeScene;
    };
    context.runtime.recordUndoState = [&recordUndoCount]() {
        ++recordUndoCount;
    };
    context.runtime.animationState = [&state]() {
        return state;
    };
    context.runtime.applyAnimationState = applyState;
    context.runtime.replaceScene = [&runtimeScene](const Scene& scene) {
        runtimeScene = scene;
    };
    context.runtime.refreshScenePanels = []() {};
    context.runtime.containsObject = [&runtimeScene](std::uint64_t incomingObjectId) {
        return runtimeScene.contains(incomingObjectId);
    };
    context.runtime.selectObject = [&selectedObjectId, &selectedSyncOutliner](std::uint64_t incomingObjectId, bool syncOutliner) {
        selectedObjectId = incomingObjectId;
        selectedSyncOutliner = syncOutliner;
    };
    context.runtime.clearInspector = [&selectedObjectId]() {
        selectedObjectId = 0;
    };
    context.runtime.frameScene = []() {};
    context.animationState = [&state]() {
        return state;
    };
    context.applyAnimationState = applyState;

    const EditorAnimationEngineFacade::OperationResult setKeyResult =
        EditorAnimationEngineFacade::setKeyForSelection(context, objectId, true);
    QVERIFY(setKeyResult.success);
    QCOMPARE(recordUndoCount, 1);
    QCOMPARE(state.currentFrame, 5);
    QVERIFY(runtimeScene.findObject(objectId)->hasTransformKeyframe(5));
    QCOMPARE(selectedObjectId, objectId);
    QVERIFY(selectedSyncOutliner);

    EditorAnimationEngineFacade::ScriptBindings bindings;
    bindings.context = context;
    bindings.findObjectIdByName = [objectId](const QString& objectName) {
        return objectName == "pCube1" ? objectId : 0;
    };
    bindings.selectObjectById = [&selectedObjectId](SceneObject::Id incomingObjectId) {
        selectedObjectId = incomingObjectId;
    };

    ScriptCommandContext scriptContext;
    EditorAnimationEngineFacade::bindScriptCommands(scriptContext, bindings);

    QVERIFY(scriptContext.copyKeyframe("pCube1", 5, 8));
    QCOMPARE(state.currentFrame, 8);
    QVERIFY(runtimeScene.findObject(objectId)->hasTransformKeyframe(8));
    QCOMPARE(recordUndoCount, 2);

    scriptContext.setPlaybackState(true);
    QVERIFY(state.playing);
}

void EditorUiTests::sceneRuntimeControllerAppliesSceneFrameAndSelection()
{
    EditorAnimationState state;
    state.currentFrame = 1;
    state.playbackStartFrame = 0;
    state.playbackEndFrame = 24;

    Scene scene;
    const SceneObject::Id objectId = scene.createObject("pCube1");

    int recordUndoCount = 0;
    int appliedFrame = -1;
    Scene appliedScene;
    bool refreshedPanels = false;
    bool framedScene = false;
    std::uint64_t selectedObjectId = 0;
    bool selectedSyncOutliner = false;

    EditorSceneRuntimeController::Context context;
    context.recordUndoState = [&recordUndoCount]() {
        ++recordUndoCount;
    };
    context.animationState = [&state]() {
        return state;
    };
    context.applyAnimationState = [&state, &appliedFrame](const EditorAnimationState& newState) {
        state = newState;
        appliedFrame = newState.currentFrame;
    };
    context.replaceScene = [&appliedScene](const Scene& newScene) {
        appliedScene = newScene;
    };
    context.refreshScenePanels = [&refreshedPanels]() {
        refreshedPanels = true;
    };
    context.containsObject = [objectId](std::uint64_t incomingObjectId) {
        return incomingObjectId == objectId;
    };
    context.selectObject = [&selectedObjectId, &selectedSyncOutliner](std::uint64_t incomingObjectId, bool syncOutliner) {
        selectedObjectId = incomingObjectId;
        selectedSyncOutliner = syncOutliner;
    };
    context.clearInspector = []() {};
    context.frameScene = [&framedScene]() {
        framedScene = true;
    };

    EditorSceneRuntimeController::ApplySceneRequest request;
    request.scene = scene;
    request.recordUndo = true;
    request.applyCurrentFrame = true;
    request.currentFrame = 12;
    request.selection.objectId = objectId;
    request.frameEntireScene = true;

    EditorSceneRuntimeController::applyScene(context, request);

    QCOMPARE(recordUndoCount, 1);
    QCOMPARE(appliedFrame, 12);
    QVERIFY(refreshedPanels);
    QVERIFY(framedScene);
    QCOMPARE(selectedObjectId, objectId);
    QVERIFY(selectedSyncOutliner);
    QCOMPARE(appliedScene.allObjectIds().size(), 1);
}

void EditorUiTests::viewportSceneControllerRoutesSceneMutations()
{
    Scene scene;
    SceneObject::Id selectedObjectId = 0;
    int beforeMutationCount = 0;
    int syncCount = 0;
    int renderCount = 0;
    int selectionCount = 0;

    EditorViewportSceneController::Context context {
        scene,
        selectedObjectId,
    };
    context.notifyBeforeSceneMutation = [&beforeMutationCount]() {
        ++beforeMutationCount;
    };
    context.syncSceneToRenderer = [&syncCount]() {
        ++syncCount;
    };
    context.requestRender = [&renderCount]() {
        ++renderCount;
    };
    context.setSelectedObject = [&scene, &selectedObjectId, &selectionCount](SceneObject::Id objectId) {
        selectedObjectId = scene.contains(objectId) ? objectId : 0;
        ++selectionCount;
    };

    const SceneObject::Id objectId =
        EditorViewportSceneController::createPrimitive(context, PrimitiveMeshFactory::Type::Cube);
    QVERIFY(objectId != 0);
    QVERIFY(scene.contains(objectId));
    QCOMPARE(selectedObjectId, objectId);
    QCOMPARE(selectionCount, 1);
    QCOMPARE(beforeMutationCount, 1);
    QVERIFY(syncCount >= 1);
    QVERIFY(renderCount >= 1);

    Transform transform;
    transform.translation = QVector3D(2.0f, 3.0f, 4.0f);
    QVERIFY(EditorViewportSceneController::setObjectLocalTransform(context, objectId, transform));

    const SceneObject* object = scene.findObject(objectId);
    QVERIFY(object != nullptr);
    QCOMPARE(object->localTransform().translation, QVector3D(2.0f, 3.0f, 4.0f));
    QCOMPARE(beforeMutationCount, 2);
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    EditorUiTests tests;
    return QTest::qExec(&tests, argc, argv);
}

#include "EditorUiTests.moc"
