#include "EditorScriptLogController.h"

#include <QPlainTextEdit>
#include <QTextCursor>

#include "EditorSceneQueryController.h"

namespace
{
QString mayaCommandName(PrimitiveMeshFactory::Type type)
{
    switch (type) {
    case PrimitiveMeshFactory::Type::Sphere: return "polySphere";
    case PrimitiveMeshFactory::Type::Cube: return "polyCube";
    case PrimitiveMeshFactory::Type::Cylinder: return "polyCylinder";
    case PrimitiveMeshFactory::Type::Cone: return "polyCone";
    case PrimitiveMeshFactory::Type::Torus: return "polyTorus";
    case PrimitiveMeshFactory::Type::Plane: return "polyPlane";
    case PrimitiveMeshFactory::Type::Disc: return "polyDisc";
    case PrimitiveMeshFactory::Type::Pyramid: return "polyPyramid";
    case PrimitiveMeshFactory::Type::Prism: return "polyPrism";
    }

    return "polyPrimitive";
}
}

namespace EditorScriptLogController
{
void appendHistoryLine(QPlainTextEdit* historyTextEdit, const QString& line)
{
    if (historyTextEdit == nullptr || line.trimmed().isEmpty()) {
        return;
    }

    historyTextEdit->appendPlainText(line);
    QTextCursor cursor = historyTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    historyTextEdit->setTextCursor(cursor);
}

void appendComment(QPlainTextEdit* historyTextEdit, const QString& line)
{
    if (line.trimmed().isEmpty()) {
        return;
    }

    appendHistoryLine(historyTextEdit, QString("// %1 //").arg(line));
}

void clearHistory(QPlainTextEdit* historyTextEdit)
{
    if (historyTextEdit == nullptr) {
        return;
    }

    historyTextEdit->clear();
    appendComment(historyTextEdit, "Script history cleared");
}

void logPrimitiveCreation(QPlainTextEdit* historyTextEdit, PrimitiveMeshFactory::Type type, const QString& objectName)
{
    appendHistoryLine(historyTextEdit, "select -cl ;");
    appendHistoryLine(historyTextEdit, QString("%1 -ch 1;").arg(mayaCommandName(type)));
    appendHistoryLine(historyTextEdit, QString("// Result: %1 %1Shape //").arg(objectName));
}

void logSelection(QPlainTextEdit* historyTextEdit, const SceneObject* object)
{
    if (object == nullptr) {
        logSelectionCleared(historyTextEdit);
        return;
    }

    const QString objectName = EditorSceneQueryController::objectDisplayName(*object);
    appendHistoryLine(historyTextEdit, QString("select -r %1;").arg(objectName));
    appendHistoryLine(historyTextEdit, QString("// Result: %1 //").arg(objectName));
}

void logSelectionCleared(QPlainTextEdit* historyTextEdit)
{
    appendHistoryLine(historyTextEdit, "select -cl;");
    appendHistoryLine(historyTextEdit, "// Result: selection cleared //");
}

void logTransformChange(QPlainTextEdit* historyTextEdit, const SceneObject* object, const Transform& transform)
{
    if (object == nullptr) {
        return;
    }

    const QString objectName = EditorSceneQueryController::objectDisplayName(*object);
    const QVector3D eulerDegrees = transform.rotation.toEulerAngles();
    appendHistoryLine(historyTextEdit, QString("setAttr \"%1.translate\" %2 %3 %4;")
                                           .arg(objectName)
                                           .arg(transform.translation.x(), 0, 'f', 3)
                                           .arg(transform.translation.y(), 0, 'f', 3)
                                           .arg(transform.translation.z(), 0, 'f', 3));
    appendHistoryLine(historyTextEdit, QString("setAttr \"%1.rotate\" %2 %3 %4;")
                                           .arg(objectName)
                                           .arg(eulerDegrees.x(), 0, 'f', 3)
                                           .arg(eulerDegrees.y(), 0, 'f', 3)
                                           .arg(eulerDegrees.z(), 0, 'f', 3));
    appendHistoryLine(historyTextEdit, QString("setAttr \"%1.scale\" %2 %3 %4;")
                                           .arg(objectName)
                                           .arg(transform.scale.x(), 0, 'f', 3)
                                           .arg(transform.scale.y(), 0, 'f', 3)
                                           .arg(transform.scale.z(), 0, 'f', 3));
    appendHistoryLine(historyTextEdit, QString("// Result: updated %1 transform //").arg(objectName));
}

void logVisibilityChange(QPlainTextEdit* historyTextEdit, const SceneObject* object, bool visible)
{
    if (object == nullptr) {
        return;
    }

    const QString objectName = EditorSceneQueryController::objectDisplayName(*object);
    appendHistoryLine(historyTextEdit, QString("setAttr \"%1.visibility\" %2;").arg(objectName).arg(visible ? 1 : 0));
    appendHistoryLine(historyTextEdit, QString("// Result: %1 visibility %2 //").arg(objectName, visible ? "on" : "off"));
}
}
