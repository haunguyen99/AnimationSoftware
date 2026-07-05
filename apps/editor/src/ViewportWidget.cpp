#include "ViewportWidget.h"

#include <QDebug>
#include <QMouseEvent>
#include <QOpenGLContext>

#include "logging/LogCategories.h"

ViewportWidget::ViewportWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
    setUpdateBehavior(QOpenGLWidget::PartialUpdate);
    setFocusPolicy(Qt::StrongFocus);
    camera_.setZoomSensitivity(0.9f);
    renderOptions_.showAxis = true;
    renderOptions_.wireframe = false;
    renderOptions_.backfaceCulling = false;
}

void ViewportWidget::requestRender()
{
    update();
}

void ViewportWidget::resetCamera()
{
    camera_.reset();
    requestRender();
}

void ViewportWidget::frameScene()
{
    if (!scene_.isEmpty() && scene_.sceneBounds().isValid()) {
        camera_.frameBounds(scene_.sceneBounds().center(), qMax(1.0f, scene_.sceneBounds().radius()));
    } else {
        camera_.frameScene();
    }
    requestRender();
}

void ViewportWidget::setWireframeEnabled(bool enabled)
{
    renderOptions_.wireframe = enabled;
    requestRender();
}

void ViewportWidget::setAxisVisible(bool visible)
{
    renderOptions_.showAxis = visible;
    requestRender();
}

void ViewportWidget::setBackfaceCullingEnabled(bool enabled)
{
    renderOptions_.backfaceCulling = enabled;
    requestRender();
}

bool ViewportWidget::importFbx(const QString& filePath)
{
    const FbxImportResult result = importer_.importFile(filePath);
    if (!result.success) {
        lastImportSucceeded_ = false;
        lastImportMessage_ = result.errorMessage;
        qCWarning(logFbx) << "import failed:" << filePath << result.errorMessage;
        return false;
    }

    if (scene_.isEmpty()) {
        scene_ = result.scene;
    } else {
        scene_.appendScene(result.scene);
    }

    makeCurrent();
    renderer_.syncScene(scene_);
    doneCurrent();

    frameScene();

    lastImportSucceeded_ = true;
    lastImportMessage_ = QString("%1 | sceneObjects=%2")
                             .arg(result.infoMessage)
                             .arg(scene_.allObjectIds().size());
    qCInfo(logFbx) << "import ok:" << result.infoMessage;
    return true;
}

QString ViewportWidget::lastImportMessage() const
{
    return lastImportMessage_;
}

bool ViewportWidget::lastImportSucceeded() const
{
    return lastImportSucceeded_;
}

void ViewportWidget::initializeGL()
{
    initializeOpenGLFunctions();

    QOpenGLContext* context = QOpenGLContext::currentContext();
    if (context == nullptr) {
        viewportInitialized_ = false;
        viewportInitFailureReason_ = "No current OpenGL context.";
        qCCritical(logViewport) << "init failed:" << viewportInitFailureReason_;
        return;
    }

    qCInfo(logViewport) << "startup:"
            << "vendor=" << reinterpret_cast<const char*>(glGetString(GL_VENDOR))
            << "renderer=" << reinterpret_cast<const char*>(glGetString(GL_RENDERER))
            << "version=" << reinterpret_cast<const char*>(glGetString(GL_VERSION));

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearColor(0.12f, 0.14f, 0.17f, 1.0f);

    viewportInitialized_ = renderer_.initialize(this);
    if (!viewportInitialized_) {
        viewportInitFailureReason_ = "Viewport renderer initialization failed.";
        qCCritical(logViewport) << "init failed:" << viewportInitFailureReason_;
        return;
    }

    viewportInitFailureReason_.clear();
    qCInfo(logViewport) << "init ok";
}

void ViewportWidget::resizeGL(int width, int height)
{
    viewportWidth_ = width;
    viewportHeight_ = height;

    glViewport(0, 0, width, height);
    camera_.setViewportSize(width, height);
    renderer_.resize(width, height);
    requestRender();
}

void ViewportWidget::paintGL()
{
    if (!viewportInitialized_) {
        if (!viewportInitFailureReason_.isEmpty()) {
            qCWarning(logViewport) << "paint skipped:" << viewportInitFailureReason_;
        }
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderer_.render(camera_, renderOptions_);
}

void ViewportWidget::mousePressEvent(QMouseEvent* event)
{
    lastMousePosition_ = event->pos();
    activeButtons_ = event->buttons();
    event->accept();
}

void ViewportWidget::mouseMoveEvent(QMouseEvent* event)
{
    const QPoint delta = event->pos() - lastMousePosition_;
    lastMousePosition_ = event->pos();

    if ((event->modifiers() & Qt::AltModifier) && (event->buttons() & Qt::LeftButton)) {
        camera_.orbit(delta.x() * -0.35f, delta.y() * -0.35f);
        requestRender();
    } else if ((event->modifiers() & Qt::AltModifier) && (event->buttons() & Qt::MiddleButton)) {
        camera_.pan(static_cast<float>(delta.x()), static_cast<float>(delta.y()));
        requestRender();
    }

    event->accept();
}

void ViewportWidget::mouseReleaseEvent(QMouseEvent* event)
{
    activeButtons_ = event->buttons();
    event->accept();
}

void ViewportWidget::wheelEvent(QWheelEvent* event)
{
    camera_.zoom(static_cast<float>(event->angleDelta().y()) / 120.0f);
    requestRender();
    event->accept();
}
