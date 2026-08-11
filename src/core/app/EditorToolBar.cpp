#include "EditorToolBar.h"

#include <QMainWindow>
#include <QToolBar>

namespace EditorToolBar
{

QToolBar* build(QMainWindow* mainWindow, const EditorMenuBar::Actions& a)
{
    QToolBar* toolbar = mainWindow->addToolBar("Viewport");
    toolbar->setMovable(false);

    // Import / Create
    toolbar->addAction(a.importFbx);
    toolbar->addAction(a.polygonPrimitives);
    toolbar->addAction(a.createJoint);

    // Rig
    toolbar->addSeparator();
    toolbar->addAction(a.markHierarchyParent);
    toolbar->addAction(a.parentToMarkedParent);
    toolbar->addAction(a.unparentSelected);
    toolbar->addAction(a.alignJointOrientation);
    toolbar->addAction(a.captureBindPose);

    // View
    toolbar->addSeparator();
    toolbar->addAction(a.resetCamera);
    toolbar->addAction(a.frameScene);
    toolbar->addAction(a.frameSelected);

    // Transform
    toolbar->addSeparator();
    toolbar->addAction(a.translate);
    toolbar->addAction(a.rotate);
    toolbar->addAction(a.scale);
    toolbar->addAction(a.worldAxis);
    toolbar->addAction(a.localAxis);

    // Display
    toolbar->addSeparator();
    toolbar->addAction(a.wireframe);
    toolbar->addAction(a.showAxis);
    toolbar->addAction(a.backfaceCulling);

    return toolbar;
}

} // namespace EditorToolBar
