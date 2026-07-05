#pragma once

#include <memory>

class QOpenGLFunctions_3_3_Core;
class QOpenGLShaderProgram;
class QString;

namespace ShaderUtils
{
std::unique_ptr<QOpenGLShaderProgram> buildProgram(
    const QString& vertexSource,
    const QString& fragmentSource);

void logOpenGlError(
    QOpenGLFunctions_3_3_Core* functions,
    const char* stage);
}

