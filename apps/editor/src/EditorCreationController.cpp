#include "EditorCreationController.h"

namespace
{
QString primitivePrefix(PrimitiveMeshFactory::Type type)
{
    switch (type) {
    case PrimitiveMeshFactory::Type::Sphere: return "pSphere";
    case PrimitiveMeshFactory::Type::Cube: return "pCube";
    case PrimitiveMeshFactory::Type::Cylinder: return "pCylinder";
    case PrimitiveMeshFactory::Type::Cone: return "pCone";
    case PrimitiveMeshFactory::Type::Torus: return "pTorus";
    case PrimitiveMeshFactory::Type::Plane: return "pPlane";
    case PrimitiveMeshFactory::Type::Disc: return "pDisc";
    case PrimitiveMeshFactory::Type::Pyramid: return "pPyramid";
    case PrimitiveMeshFactory::Type::Prism: return "pPrism";
    }

    return "pPrimitive";
}

QString defaultJointPrefix()
{
    return "joint";
}
}

namespace EditorCreationController
{
QVector<PrimitivePaletteEntry> primitivePaletteEntries()
{
    return {
        { "Sphere", PrimitiveMeshFactory::Type::Sphere, true },
        { "Cube", PrimitiveMeshFactory::Type::Cube, true },
        { "Cylinder", PrimitiveMeshFactory::Type::Cylinder, true },
        { "Cone", PrimitiveMeshFactory::Type::Cone, true },
        { "Torus", PrimitiveMeshFactory::Type::Torus, true },
        { "Plane", PrimitiveMeshFactory::Type::Plane, true },
        { "Disc", PrimitiveMeshFactory::Type::Disc, true },
        { "Platonic Solid", PrimitiveMeshFactory::Type::Cube, false },
        { "Pyramid", PrimitiveMeshFactory::Type::Pyramid, true },
        { "Prism", PrimitiveMeshFactory::Type::Prism, true },
        { "Pipe", PrimitiveMeshFactory::Type::Cylinder, false },
        { "Helix", PrimitiveMeshFactory::Type::Cylinder, false },
        { "Gear", PrimitiveMeshFactory::Type::Cylinder, false },
        { "Soccer Ball", PrimitiveMeshFactory::Type::Sphere, false },
        { "Super Ellipse", PrimitiveMeshFactory::Type::Sphere, false },
        { "Spherical Harmonics", PrimitiveMeshFactory::Type::Sphere, false },
        { "Ultra Shape", PrimitiveMeshFactory::Type::Sphere, false },
    };
}

OperationResult createPrimitive(const Context& context, PrimitiveMeshFactory::Type type)
{
    OperationResult result;
    if (!PrimitiveMeshFactory::isImplemented(type)) {
        result.errorMessage = "This primitive is not implemented yet";
        return result;
    }

    result.success = true;
    result.primitiveType = type;
    result.objectName = context.generateUniqueScriptName(primitivePrefix(type));
    result.statusMessage = QString("Created %1").arg(PrimitiveMeshFactory::displayName(type));
    return result;
}

OperationResult createJoint(const Context& context, std::uint64_t selectedObjectId)
{
    OperationResult result;
    result.success = true;
    result.objectName = context.generateUniqueObjectName(defaultJointPrefix(), 0);
    result.objectId = selectedObjectId;
    result.statusMessage = QString("Created %1").arg(result.objectName);
    result.commandLine = QString("joint -name \"%1\";").arg(result.objectName);
    result.resultLine = QString("// Result: %1 //").arg(result.objectName);
    return result;
}

void bindScriptCommands(ScriptCommandContext& context, const ScriptBindings& bindings)
{
    context.createPrimitive = [bindings](PrimitiveMeshFactory::Type type) {
        const OperationResult creation = createPrimitive(
            Context{
                bindings.generateUniqueScriptName,
                bindings.generateUniqueObjectName,
            },
            type);
        if (!creation.success) {
            return QString();
        }

        const std::uint64_t objectId = bindings.createPrimitive(type, creation.objectName);
        if (objectId == 0) {
            return QString();
        }

        bindings.applyLiveMutation(objectId, false);
        return creation.objectName;
    };
    context.createJoint = [bindings](const QString& requestedName) {
        const QString jointBaseName = requestedName.trimmed().isEmpty() ? QString("joint") : requestedName.trimmed();
        const QString objectName = bindings.generateUniqueObjectName(jointBaseName, 0);
        const std::uint64_t parentId = bindings.selectedObjectId();
        const std::uint64_t objectId = bindings.createJoint(objectName, parentId);
        if (objectId == 0) {
            return QString();
        }

        bindings.applyLiveMutation(objectId, false);
        return objectName;
    };
}
}
