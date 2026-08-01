#pragma once

#include <cstdint>
#include <functional>

#include <QString>
#include <QVector>

#include "core/commands/ScriptCommandSystem.h"
#include "scene/PrimitiveMeshFactory.h"

namespace EditorCreationController
{
struct Context
{
    std::function<QString(const QString&)> generateUniqueScriptName;
    std::function<QString(const QString&, std::uint64_t)> generateUniqueObjectName;
};

struct OperationResult
{
    bool success = false;
    PrimitiveMeshFactory::Type primitiveType = PrimitiveMeshFactory::Type::Cube;
    std::uint64_t objectId = 0;
    QString objectName;
    QString errorMessage;
    QString statusMessage;
    QString commandLine;
    QString resultLine;
};

struct PrimitivePaletteEntry
{
    QString label;
    PrimitiveMeshFactory::Type type = PrimitiveMeshFactory::Type::Cube;
    bool implemented = false;
};

struct ScriptBindings
{
    std::function<QString(const QString&)> generateUniqueScriptName;
    std::function<QString(const QString&, std::uint64_t)> generateUniqueObjectName;
    std::function<std::uint64_t()> selectedObjectId;
    std::function<std::uint64_t(PrimitiveMeshFactory::Type, const QString&)> createPrimitive;
    std::function<std::uint64_t(const QString&, std::uint64_t)> createJoint;
    std::function<void(std::uint64_t, bool)> applyLiveMutation;
};

QVector<PrimitivePaletteEntry> primitivePaletteEntries();
OperationResult createPrimitive(const Context& context, PrimitiveMeshFactory::Type type);
OperationResult createJoint(const Context& context, std::uint64_t selectedObjectId);
void bindScriptCommands(ScriptCommandContext& context, const ScriptBindings& bindings);
}
