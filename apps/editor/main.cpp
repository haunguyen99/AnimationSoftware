#include <QApplication>
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QTextStream>
#include <QSurfaceFormat>

#include "MainWindow.h"
#include "logging/LogCategories.h"

namespace
{
QMutex gLogFileMutex;
QString gLogFilePath;

void phoenixMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    QByteArray localMessage = message.toLocal8Bit();
    const char* level = "INFO";
    switch (type) {
    case QtDebugMsg:
        level = "DEBUG";
        break;
    case QtInfoMsg:
        level = "INFO";
        break;
    case QtWarningMsg:
        level = "WARN";
        break;
    case QtCriticalMsg:
        level = "ERROR";
        break;
    case QtFatalMsg:
        level = "FATAL";
        break;
    }

    const QString category = context.category != nullptr ? QString::fromUtf8(context.category) : QString("default");
    const QString line = QString("[%1] [%2] [%3] %4")
            .arg(QDateTime::currentDateTime().toString(Qt::ISODate))
            .arg(level)
            .arg(category)
            .arg(message);

    fprintf(stderr, "%s\n", line.toLocal8Bit().constData());
    fflush(stderr);

    if (!gLogFilePath.isEmpty()) {
        QMutexLocker locker(&gLogFileMutex);
        QFile file(gLogFilePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << line << '\n';
        }
    }

    if (type == QtFatalMsg) {
        abort();
    }
}
}

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

    const QString logDirPath = QCoreApplication::applicationDirPath() + "/logs";
    QDir().mkpath(logDirPath);
    gLogFilePath = logDirPath + "/phoenix_editor.log";
    qInstallMessageHandler(phoenixMessageHandler);

    qCInfo(logApp) << "startup:"
            << "name=" << QCoreApplication::applicationName()
            << "timestamp=" << QDateTime::currentDateTime().toString(Qt::ISODate)
            << "qt=" << QT_VERSION_STR
            << "argv=" << QCoreApplication::arguments();
    qCInfo(logApp) << "startup:"
            << "logFile=" << QFileInfo(gLogFilePath).absoluteFilePath();
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
