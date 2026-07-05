#include "rendering/ShaderUtils.h"

#include "logging/LogCategories.h"

#include <QDebug>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>

std::unique_ptr<QOpenGLShaderProgram> ShaderUtils::buildProgram(
    const QString& vertexSource,
    const QString& fragmentSource)
{
    auto program = std::make_unique<QOpenGLShaderProgram>();

    if (!program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexSource)) {
        qCWarning(logViewport) << "vertex shader compile failed:" << program->log();
        return {};
    }

    if (!program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentSource)) {
        qCWarning(logViewport) << "fragment shader compile failed:" << program->log();
        return {};
    }

    if (!program->link()) {
        qCWarning(logViewport) << "shader link failed:" << program->log();
        return {};
    }

    return program;
}

void ShaderUtils::logOpenGlError(
    QOpenGLFunctions_3_3_Core* functions,
    const char* stage)
{
#ifndef NDEBUG
    if (functions == nullptr) {
        return;
    }

    GLenum error = functions->glGetError();
    while (error != GL_NO_ERROR) {
        qCWarning(logViewport) << "OpenGL error at" << stage << ":" << Qt::hex << static_cast<unsigned int>(error);
        error = functions->glGetError();
    }
#else
    Q_UNUSED(functions);
    Q_UNUSED(stage);
#endif
}
