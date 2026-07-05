#include <QApplication>
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QSurfaceFormat>

#include "MainWindow.h"
#include "logging/LogCategories.h"

int main(int argc, char* argv[])
{
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(3, 3);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("Phoenix Editor Beta");
    QCoreApplication::setOrganizationName("Project Phoenix");

    qCInfo(logApp) << "startup:"
            << "name=" << QCoreApplication::applicationName()
            << "timestamp=" << QDateTime::currentDateTime().toString(Qt::ISODate)
            << "qt=" << QT_VERSION_STR
            << "argv=" << QCoreApplication::arguments();
    qCInfo(logApp) << "startup:"
            << "OpenGL format"
            << format.majorVersion() << "." << format.minorVersion()
            << "profile=" << format.profile()
            << "depth=" << format.depthBufferSize()
            << "stencil=" << format.stencilBufferSize();

    MainWindow window;
    window.show();

    return app.exec();
}
