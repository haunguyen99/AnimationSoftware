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

#include "MainWindow.h"
#include "ScriptCommandSystem.h"

class EditorUiTests : public QObject
{
    Q_OBJECT

private slots:
    void restoreDefaultLayoutResetsFloatingDocks();
    void scriptEditorExecutesPrimitiveCommand();
    void scriptEditorLogsChannelBoxChanges();
    void scriptEditorExecutesTimelineCommands();
    void scriptEditorExecutesObjectCommands();
    void scriptEditorExecutesViewToolAndFileCommands();
    void scriptEditorExecutesSceneFileCommands();
    void scriptCommandRegistryDispatchesFileImportCommand();
    void scriptCommandRegistryDispatchesSetKeyframeCommand();
    void scriptCommandRegistryDispatchesDeleteKeyCommand();
    void scriptCommandRegistryDispatchesAutoKeyCommand();
    void scriptCommandRegistryDispatchesJointHierarchyCommands();
    void scriptCommandRegistryDispatchesJointOrientationAndBindPoseCommands();
    void hierarchyActionsParentAndUnparentJoints();
    void hierarchyActionsExposeMayaLikeShortcuts();
    void leftMouseDragReparentsOutlinerItems();
    void jointInspectorEditsOrientationAndCapturesBindPose();
    void timelineUiShowsKeyframeFeedback();

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

    QVERIFY(viewportDock != nullptr);
    QVERIFY(outlinerDock != nullptr);
    QVERIFY(inspectorDock != nullptr);
    QVERIFY(restoreAction != nullptr);

    outlinerDock->setFloating(true);
    inspectorDock->hide();
    QVERIFY(outlinerDock->isFloating());
    QVERIFY(!inspectorDock->isVisible());

    restoreAction->trigger();

    QTRY_VERIFY(!outlinerDock->isFloating());
    QTRY_VERIFY(!viewportDock->isFloating());
    QTRY_VERIFY(!inspectorDock->isFloating());
    QTRY_VERIFY(outlinerDock->isVisible());
    QTRY_VERIFY(viewportDock->isVisible());
    QTRY_VERIFY(inspectorDock->isVisible());
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
    QVERIFY(timelineStatusLabel->text().contains("frame 10 keyed"));

    deleteKeyButton->click();
    QTRY_VERIFY(!deleteKeyButton->isEnabled());
    QVERIFY(timelineStatusLabel->text().contains("frame 10 has no key"));

    translateXSpinBox->setValue(5.0);
    QTRY_VERIFY(deleteKeyButton->isEnabled());
    QVERIFY(timelineStatusLabel->text().contains("frame 10 keyed"));

    currentFrameSpinBox->setValue(0);
    QTRY_COMPARE(setKeyButton->text(), QString("Key Selected"));
    QTRY_VERIFY(deleteKeyButton->isEnabled());
    QVERIFY(timelineStatusLabel->text().contains("frame 0 keyed"));

    currentFrameSpinBox->setValue(10);
    QTRY_COMPARE(setKeyButton->text(), QString("Key Selected"));
    QTRY_VERIFY(deleteKeyButton->isEnabled());
    QVERIFY(timelineStatusLabel->text().contains("frame 10 keyed"));
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    EditorUiTests tests;
    return QTest::qExec(&tests, argc, argv);
}

#include "EditorUiTests.moc"
