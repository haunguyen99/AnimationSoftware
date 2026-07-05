#include "MainWindow.h"

#include <QAction>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>

#include "ViewportWidget.h"

MainWindow::MainWindow()
{
    setWindowTitle("Phoenix Editor Beta");
    resize(1280, 720);

    viewport_ = new ViewportWidget(this);
    setCentralWidget(viewport_);

    createMenus();
    createToolbar();
    statusBar()->showMessage("Ready");
}

void MainWindow::createMenus()
{
    QMenu* fileMenu = menuBar()->addMenu("&File");
    importFbxAction_ = fileMenu->addAction("Import FBX");
    QObject::connect(importFbxAction_, &QAction::triggered, this, &MainWindow::importFbx);

    QMenu* viewMenu = menuBar()->addMenu("&View");

    resetCameraAction_ = viewMenu->addAction("Reset Camera");
    QObject::connect(resetCameraAction_, &QAction::triggered, this, [this]() {
        viewport_->resetCamera();
        statusBar()->showMessage("Camera reset", 2000);
    });

    frameSceneAction_ = viewMenu->addAction("Frame Scene");
    QObject::connect(frameSceneAction_, &QAction::triggered, this, [this]() {
        viewport_->frameScene();
        statusBar()->showMessage("Scene framed", 2000);
    });

    wireframeAction_ = viewMenu->addAction("Wireframe");
    wireframeAction_->setCheckable(true);
    QObject::connect(wireframeAction_, &QAction::toggled, this, [this](bool enabled) {
        viewport_->setWireframeEnabled(enabled);
        statusBar()->showMessage(enabled ? "Wireframe on" : "Wireframe off", 2000);
    });

    showAxisAction_ = viewMenu->addAction("Show Axis");
    showAxisAction_->setCheckable(true);
    showAxisAction_->setChecked(true);
    QObject::connect(showAxisAction_, &QAction::toggled, this, [this](bool enabled) {
        viewport_->setAxisVisible(enabled);
        statusBar()->showMessage(enabled ? "Axis visible" : "Axis hidden", 2000);
    });

    backfaceCullingAction_ = viewMenu->addAction("Backface Culling");
    backfaceCullingAction_->setCheckable(true);
    QObject::connect(backfaceCullingAction_, &QAction::toggled, this, [this](bool enabled) {
        viewport_->setBackfaceCullingEnabled(enabled);
        statusBar()->showMessage(enabled ? "Backface culling on" : "Backface culling off", 2000);
    });
}

void MainWindow::createToolbar()
{
    toolbar_ = addToolBar("Viewport");
    toolbar_->setMovable(false);
    toolbar_->addAction(importFbxAction_);
    toolbar_->addSeparator();
    toolbar_->addAction(resetCameraAction_);
    toolbar_->addAction(frameSceneAction_);
    toolbar_->addSeparator();
    toolbar_->addAction(wireframeAction_);
    toolbar_->addAction(showAxisAction_);
    toolbar_->addAction(backfaceCullingAction_);
}

void MainWindow::importFbx()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        "Import FBX",
        QString(),
        "FBX Files (*.fbx)");

    if (filePath.isEmpty()) {
        statusBar()->showMessage("Import cancelled", 2000);
        return;
    }

    if (!viewport_->importFbx(filePath)) {
        const QString errorMessage = viewport_->lastImportMessage().isEmpty()
            ? "Unknown FBX import error."
            : viewport_->lastImportMessage();
        statusBar()->showMessage(QString("Import failed: %1").arg(errorMessage), 5000);
        QMessageBox::warning(this, "Import FBX", errorMessage);
        return;
    }

    statusBar()->showMessage(QString("Imported: %1").arg(viewport_->lastImportMessage()), 4000);
}
