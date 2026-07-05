#pragma once

#include <QMainWindow>

class ViewportWidget;
class QAction;
class QToolBar;
class QStatusBar;

class MainWindow : public QMainWindow
{
public:
    MainWindow();

private:
    void createMenus();
    void createToolbar();
    void importFbx();

    ViewportWidget* viewport_ = nullptr;
    QToolBar* toolbar_ = nullptr;
    QAction* importFbxAction_ = nullptr;
    QAction* resetCameraAction_ = nullptr;
    QAction* frameSceneAction_ = nullptr;
    QAction* wireframeAction_ = nullptr;
    QAction* showAxisAction_ = nullptr;
    QAction* backfaceCullingAction_ = nullptr;
};
