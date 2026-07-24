#include "ScriptCommandSystem.h"

#include <QRegularExpression>

namespace
{
QString stripQuotes(QString value)
{
    value = value.trimmed();
    if (value.size() >= 2 && value.startsWith('"') && value.endsWith('"')) {
        value = value.mid(1, value.size() - 2);
    }
    return value;
}

QStringList tokenizeCommand(const QString& commandLine)
{
    QStringList tokens;
    QRegularExpression tokenPattern(R"cmd("([^"]*)"|(\S+))cmd");
    QRegularExpressionMatchIterator it = tokenPattern.globalMatch(commandLine);
    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        if (match.captured(1).isEmpty()) {
            tokens.append(match.captured(2));
        } else {
            tokens.append(QString("\"%1\"").arg(match.captured(1)));
        }
    }
    return tokens;
}
}

bool ScriptCommandRegistry::execute(QString commandLine, const ScriptCommandContext& context, ScriptCommandExecution* execution) const
{
    commandLine = commandLine.trimmed();
    if (commandLine.isEmpty()) {
        return false;
    }

    if (commandLine.endsWith(';')) {
        commandLine.chop(1);
    }

    const QStringList tokens = tokenizeCommand(commandLine);
    if (tokens.isEmpty()) {
        return false;
    }

    ScriptCommandExecution localExecution;
    ScriptCommandExecution& result = execution != nullptr ? *execution : localExecution;
    result.handled = true;
    result.resultLine.clear();

    const QString command = tokens.first();
    if (command == "select") {
        if (tokens.contains("-cl")) {
            if (context.clearSelection) {
                context.clearSelection();
            }
            result.resultLine = "// Result: selection cleared //";
            return true;
        }

        QString selectedObjectName;
        for (int index = 1; index < tokens.size(); ++index) {
            const QString token = tokens.at(index);
            if (token.startsWith('-')) {
                continue;
            }

            if (context.selectObjectByName && context.selectObjectByName(token)) {
                selectedObjectName = token;
            }
        }

        if (selectedObjectName.isEmpty()) {
            result.resultLine = "// Error: object not found //";
            return true;
        }

        result.resultLine = QString("// Result: %1 //").arg(selectedObjectName);
        return true;
    }

    if (command == "setAttr") {
        if (tokens.size() < 3) {
            result.resultLine = "// Error: invalid setAttr syntax //";
            return true;
        }

        const QString plugPath = stripQuotes(tokens.at(1));
        const int separatorIndex = plugPath.lastIndexOf('.');
        if (separatorIndex <= 0 || separatorIndex >= plugPath.size() - 1) {
            result.resultLine = "// Error: invalid attribute path //";
            return true;
        }

        QList<double> values;
        for (int index = 2; index < tokens.size(); ++index) {
            bool ok = false;
            const double value = tokens.at(index).toDouble(&ok);
            if (!ok) {
                result.resultLine = "// Error: invalid setAttr value //";
                return true;
            }
            values.append(value);
        }

        const QString objectName = plugPath.left(separatorIndex);
        const QString attributeName = plugPath.mid(separatorIndex + 1);
        const bool updated = context.setAttribute && context.setAttribute(objectName, attributeName, values);
        if (!updated) {
            result.resultLine = "// Error: setAttr failed //";
            return true;
        }

        result.resultLine = QString("// Result: %1 updated //").arg(plugPath);
        return true;
    }

    if (command == "rename") {
        if (tokens.size() < 3) {
            result.resultLine = "// Error: rename requires source and destination names //";
            return true;
        }

        const QString renamedObject = context.renameObject
            ? context.renameObject(stripQuotes(tokens.at(1)), stripQuotes(tokens.at(2)))
            : QString();
        if (renamedObject.isEmpty()) {
            result.resultLine = "// Error: rename failed //";
            return true;
        }

        result.resultLine = QString("// Result: %1 //").arg(renamedObject);
        return true;
    }

    if (command == "joint") {
        QString jointName;
        const int nameIndex = tokens.indexOf("-name");
        if (nameIndex >= 0 && nameIndex + 1 < tokens.size()) {
            jointName = stripQuotes(tokens.at(nameIndex + 1));
        }

        const QString createdJoint = context.createJoint ? context.createJoint(jointName) : QString();
        if (createdJoint.isEmpty()) {
            result.resultLine = "// Error: joint creation failed //";
            return true;
        }

        result.resultLine = QString("// Result: %1 //").arg(createdJoint);
        return true;
    }

    if (command == "duplicate") {
        if (tokens.size() < 2) {
            result.resultLine = "// Error: duplicate requires an object name //";
            return true;
        }

        const QString duplicatedObject = context.duplicateObject
            ? context.duplicateObject(stripQuotes(tokens.at(1)))
            : QString();
        if (duplicatedObject.isEmpty()) {
            result.resultLine = "// Error: duplicate failed //";
            return true;
        }

        result.resultLine = QString("// Result: %1 //").arg(duplicatedObject);
        return true;
    }

    if (command == "group") {
        if (tokens.size() < 2) {
            result.resultLine = "// Error: group requires an object name //";
            return true;
        }

        const QString groupName = context.groupObject
            ? context.groupObject(stripQuotes(tokens.at(1)))
            : QString();
        if (groupName.isEmpty()) {
            result.resultLine = "// Error: group failed //";
            return true;
        }

        result.resultLine = QString("// Result: %1 //").arg(groupName);
        return true;
    }

    if (command == "delete") {
        if (tokens.size() < 2) {
            result.resultLine = "// Error: delete requires an object name //";
            return true;
        }

        const bool deleted = context.deleteObject
            && context.deleteObject(stripQuotes(tokens.at(1)));
        result.resultLine = deleted ? "// Result: object deleted //" : "// Error: delete failed //";
        return true;
    }

    if (command == "parent") {
        if (tokens.size() < 3) {
            result.resultLine = "// Error: parent requires child and parent names //";
            return true;
        }

        const QString childName = stripQuotes(tokens.at(1));
        const QString parentName = stripQuotes(tokens.at(2));
        const bool parented = context.parentObject && context.parentObject(childName, parentName);
        result.resultLine = parented
            ? QString("// Result: parented %1 under %2 //").arg(childName, parentName)
            : "// Error: parent failed //";
        return true;
    }

    if (command == "unparent") {
        if (tokens.size() < 2) {
            result.resultLine = "// Error: unparent requires a child name //";
            return true;
        }

        const QString childName = stripQuotes(tokens.at(1));
        const bool unparented = context.unparentObject && context.unparentObject(childName);
        result.resultLine = unparented
            ? QString("// Result: unparented %1 //").arg(childName)
            : "// Error: unparent failed //";
        return true;
    }

    if (command == "jointOrient") {
        QString objectName;
        for (int index = 1; index < tokens.size(); ++index) {
            const QString token = stripQuotes(tokens.at(index));
            if (!token.startsWith('-')) {
                objectName = token;
                break;
            }
        }

        if (objectName.isEmpty()) {
            result.resultLine = "// Error: jointOrient requires an object name //";
            return true;
        }

        if (tokens.contains("-reset")) {
            const bool reset = context.resetJointOrientation && context.resetJointOrientation(objectName);
            result.resultLine = reset
                ? QString("// Result: reset joint orientation on %1 //").arg(objectName)
                : "// Error: jointOrient reset failed //";
            return true;
        }

        if (tokens.contains("-alignToChild")) {
            const bool aligned = context.alignJointOrientationToChild && context.alignJointOrientationToChild(objectName);
            result.resultLine = aligned
                ? QString("// Result: aligned joint orientation on %1 //").arg(objectName)
                : "// Error: jointOrient align failed //";
            return true;
        }

        const int eulerIndex = tokens.indexOf("-euler");
        if (eulerIndex < 0 || eulerIndex + 3 >= tokens.size()) {
            result.resultLine = "// Error: jointOrient requires -euler x y z, -reset, or -alignToChild //";
            return true;
        }

        bool okX = false;
        bool okY = false;
        bool okZ = false;
        const float x = tokens.at(eulerIndex + 1).toFloat(&okX);
        const float y = tokens.at(eulerIndex + 2).toFloat(&okY);
        const float z = tokens.at(eulerIndex + 3).toFloat(&okZ);
        if (!okX || !okY || !okZ) {
            result.resultLine = "// Error: invalid jointOrient euler values //";
            return true;
        }

        const bool updated = context.setJointOrientation
            && context.setJointOrientation(objectName, QVector3D(x, y, z));
        result.resultLine = updated
            ? QString("// Result: joint orientation updated on %1 //").arg(objectName)
            : "// Error: jointOrient failed //";
        return true;
    }

    if (command == "bindPose") {
        if (!tokens.contains("-capture")) {
            result.resultLine = "// Error: bindPose requires -capture //";
            return true;
        }

        QString objectName;
        for (int index = 1; index < tokens.size(); ++index) {
            const QString token = stripQuotes(tokens.at(index));
            if (!token.startsWith('-')) {
                objectName = token;
                break;
            }
        }

        if (objectName.isEmpty()) {
            result.resultLine = "// Error: bindPose requires an object name //";
            return true;
        }

        const bool recursive = tokens.contains("-recursive");
        const bool captured = context.captureBindPose && context.captureBindPose(objectName, recursive);
        result.resultLine = captured
            ? QString("// Result: captured bind pose on %1%2 //")
                  .arg(objectName, recursive ? " recursively" : "")
            : "// Error: bindPose capture failed //";
        return true;
    }

    if (command == "bindSkin") {
        if (tokens.size() < 3) {
            result.resultLine = "// Error: bindSkin requires mesh and joint names //";
            return true;
        }

        const QString meshName = stripQuotes(tokens.at(1));
        const QString jointName = stripQuotes(tokens.at(2));
        const bool bound = context.bindSkin && context.bindSkin(meshName, jointName);
        result.resultLine = bound
            ? QString("// Result: bound %1 to %2 //").arg(meshName, jointName)
            : "// Error: bindSkin failed //";
        return true;
    }

    if (command == "currentTime") {
        if (tokens.size() < 2) {
            result.resultLine = "// Error: invalid currentTime syntax //";
            return true;
        }

        bool ok = false;
        const int frame = tokens.at(1).toInt(&ok);
        if (!ok) {
            result.resultLine = "// Error: invalid frame value //";
            return true;
        }

        if (context.setCurrentFrame) {
            context.setCurrentFrame(frame);
        }
        result.resultLine = QString("// Result: current frame %1 //").arg(frame);
        return true;
    }

    if (command == "setKeyframe") {
        if (tokens.size() < 2) {
            result.resultLine = "// Error: setKeyframe requires an object name //";
            return true;
        }

        const QString objectName = stripQuotes(tokens.at(1));
        int frame = 0;
        bool hasFrame = false;
        const int timeIndex = tokens.indexOf("-t");
        if (timeIndex >= 0 && timeIndex + 1 < tokens.size()) {
            bool ok = false;
            frame = tokens.at(timeIndex + 1).toInt(&ok);
            if (!ok) {
                result.resultLine = "// Error: invalid keyframe time //";
                return true;
            }
            hasFrame = true;
        }

        const bool keyed = context.setKeyframe && context.setKeyframe(objectName, hasFrame ? frame : -1);
        if (!keyed) {
            result.resultLine = "// Error: setKeyframe failed //";
            return true;
        }

        result.resultLine = hasFrame
            ? QString("// Result: key set on %1 at frame %2 //").arg(objectName).arg(frame)
            : QString("// Result: key set on %1 //").arg(objectName);
        return true;
    }

    if (command == "autoKeyframe") {
        const int stateIndex = tokens.indexOf("-state");
        if (stateIndex < 0 || stateIndex + 1 >= tokens.size()) {
            result.resultLine = "// Error: autoKeyframe requires -state on|off //";
            return true;
        }

        const QString stateToken = tokens.at(stateIndex + 1).toLower();
        if (stateToken != "on" && stateToken != "off") {
            result.resultLine = "// Error: autoKeyframe state must be on or off //";
            return true;
        }

        if (context.setAutoKey) {
            context.setAutoKey(stateToken == "on");
        }
        result.resultLine = QString("// Result: auto key %1 //").arg(stateToken);
        return true;
    }

    if (command == "cutKey") {
        if (tokens.size() < 2) {
            result.resultLine = "// Error: cutKey requires an object name //";
            return true;
        }

        const QString objectName = stripQuotes(tokens.at(1));
        int frame = 0;
        bool hasFrame = false;
        const int timeIndex = tokens.indexOf("-t");
        if (timeIndex >= 0 && timeIndex + 1 < tokens.size()) {
            bool ok = false;
            frame = tokens.at(timeIndex + 1).toInt(&ok);
            if (!ok) {
                result.resultLine = "// Error: invalid cutKey time //";
                return true;
            }
            hasFrame = true;
        }

        const bool deleted = context.deleteKeyframe && context.deleteKeyframe(objectName, hasFrame ? frame : -1);
        if (!deleted) {
            result.resultLine = "// Error: cutKey failed //";
            return true;
        }

        result.resultLine = hasFrame
            ? QString("// Result: deleted key on %1 at frame %2 //").arg(objectName).arg(frame)
            : QString("// Result: deleted key on %1 //").arg(objectName);
        return true;
    }

    if (command == "copyKey") {
        if (tokens.size() < 2) {
            result.resultLine = "// Error: copyKey requires an object name //";
            return true;
        }

        const QString objectName = stripQuotes(tokens.at(1));
        const int timeIndex = tokens.indexOf("-t");
        const int toIndex = tokens.indexOf("-to");
        if (timeIndex < 0 || timeIndex + 1 >= tokens.size() || toIndex < 0 || toIndex + 1 >= tokens.size()) {
            result.resultLine = "// Error: copyKey requires -t and -to //";
            return true;
        }

        bool okSource = false;
        bool okTarget = false;
        const int sourceFrame = tokens.at(timeIndex + 1).toInt(&okSource);
        const int targetFrame = tokens.at(toIndex + 1).toInt(&okTarget);
        if (!okSource || !okTarget) {
            result.resultLine = "// Error: invalid copyKey frame value //";
            return true;
        }

        const bool copied = context.copyKeyframe && context.copyKeyframe(objectName, sourceFrame, targetFrame);
        result.resultLine = copied
            ? QString("// Result: copied key on %1 from frame %2 to %3 //").arg(objectName).arg(sourceFrame).arg(targetFrame)
            : "// Error: copyKey failed //";
        return true;
    }

    if (command == "shiftKey") {
        if (tokens.size() < 2) {
            result.resultLine = "// Error: shiftKey requires an object name //";
            return true;
        }

        const QString objectName = stripQuotes(tokens.at(1));
        const int byIndex = tokens.indexOf("-by");
        if (byIndex < 0 || byIndex + 1 >= tokens.size()) {
            result.resultLine = "// Error: shiftKey requires -by //";
            return true;
        }

        bool ok = false;
        const int frameDelta = tokens.at(byIndex + 1).toInt(&ok);
        if (!ok) {
            result.resultLine = "// Error: invalid shiftKey delta //";
            return true;
        }

        const bool shifted = context.shiftKeyframes && context.shiftKeyframes(objectName, frameDelta);
        result.resultLine = shifted
            ? QString("// Result: shifted keys on %1 by %2 //").arg(objectName).arg(frameDelta)
            : "// Error: shiftKey failed //";
        return true;
    }

    if (command == "playbackOptions") {
        int startFrame = 0;
        int endFrame = 0;
        bool hasStartFrame = false;
        bool hasEndFrame = false;

        for (int index = 1; index < tokens.size() - 1; ++index) {
            if (tokens.at(index) != "-min" && tokens.at(index) != "-max") {
                continue;
            }

            bool ok = false;
            const int value = tokens.at(index + 1).toInt(&ok);
            if (!ok) {
                result.resultLine = "// Error: invalid playback range value //";
                return true;
            }

            if (tokens.at(index) == "-min") {
                startFrame = value;
                hasStartFrame = true;
            } else {
                endFrame = value;
                hasEndFrame = true;
            }
        }

        if (!hasStartFrame || !hasEndFrame) {
            result.resultLine = "// Error: playbackOptions requires -min and -max //";
            return true;
        }

        if (context.setPlaybackRange) {
            context.setPlaybackRange(startFrame, endFrame);
        }
        result.resultLine = QString("// Result: playback range %1 to %2 //").arg(startFrame).arg(endFrame);
        return true;
    }

    if (command == "play") {
        int stateIndex = tokens.indexOf("-state");
        if (stateIndex < 0 || stateIndex + 1 >= tokens.size()) {
            result.resultLine = "// Error: play requires -state on|off //";
            return true;
        }

        const QString stateToken = tokens.at(stateIndex + 1).toLower();
        if (stateToken != "on" && stateToken != "off") {
            result.resultLine = "// Error: play state must be on or off //";
            return true;
        }

        if (context.setPlaybackState) {
            context.setPlaybackState(stateToken == "on");
        }
        result.resultLine = QString("// Result: playback %1 //").arg(stateToken == "on" ? "started" : "stopped");
        return true;
    }

    if (command == "viewFit") {
        const QString targetName = tokens.size() >= 2 ? stripQuotes(tokens.at(1)) : QString();
        const bool framed = context.frameView && context.frameView(targetName);
        if (!framed) {
            result.resultLine = targetName.isEmpty()
                ? "// Error: frame scene failed //"
                : QString("// Error: could not frame '%1' //").arg(targetName);
            return true;
        }

        result.resultLine = targetName.isEmpty()
            ? "// Result: scene framed //"
            : QString("// Result: framed %1 //").arg(targetName);
        return true;
    }

    if (command == "viewSet") {
        if (!tokens.contains("-home")) {
            result.resultLine = "// Error: unsupported viewSet syntax //";
            return true;
        }

        if (context.resetCamera) {
            context.resetCamera();
        }
        result.resultLine = "// Result: camera reset //";
        return true;
    }

    if (command == "setToolTo") {
        if (tokens.size() < 2) {
            result.resultLine = "// Error: setToolTo requires a tool context //";
            return true;
        }

        const QString toolName = stripQuotes(tokens.at(1));
        const bool activated = context.activateTool && context.activateTool(toolName);
        if (!activated) {
            result.resultLine = QString("// Error: unsupported tool '%1' //").arg(toolName);
            return true;
        }

        if (toolName == "MoveSuperContext") {
            result.resultLine = "// Result: move tool //";
        } else if (toolName == "RotateSuperContext") {
            result.resultLine = "// Result: rotate tool //";
        } else if (toolName == "ScaleSuperContext") {
            result.resultLine = "// Result: scale tool //";
        } else {
            result.resultLine = QString("// Result: %1 //").arg(toolName);
        }
        return true;
    }

    if (command == "file") {
        if (tokens.contains("-f") && tokens.contains("-new")) {
            if (context.newScene) {
                context.newScene();
            }
            result.resultLine = "// Result: new scene //";
            return true;
        }

        if (tokens.contains("-save")) {
            QString filePath;
            for (int index = 1; index < tokens.size(); ++index) {
                if (tokens.at(index).startsWith('-')) {
                    continue;
                }
                filePath = stripQuotes(tokens.at(index));
            }

            const bool saved = context.saveSceneFile && context.saveSceneFile(filePath);
            result.resultLine = saved
                ? QString("// Result: saved %1 //").arg(filePath.isEmpty() ? "scene" : filePath)
                : "// Error: save failed //";
            return true;
        }

        if (tokens.contains("-o")) {
            QString filePath;
            for (int index = 1; index < tokens.size(); ++index) {
                if (tokens.at(index).startsWith('-')) {
                    continue;
                }
                filePath = stripQuotes(tokens.at(index));
            }

            if (filePath.isEmpty()) {
                result.resultLine = "// Error: file -o requires a path //";
                return true;
            }

            const bool opened = context.openSceneFile && context.openSceneFile(filePath);
            result.resultLine = opened
                ? QString("// Result: opened %1 //").arg(filePath)
                : "// Error: open failed //";
            return true;
        }

        if (tokens.contains("-import")) {
            QString filePath;
            for (int index = 1; index < tokens.size(); ++index) {
                if (tokens.at(index).startsWith('-')) {
                    continue;
                }
                filePath = stripQuotes(tokens.at(index));
            }

            if (filePath.isEmpty()) {
                result.resultLine = "// Error: file -import requires a path //";
                return true;
            }

            const bool imported = context.importSceneFile && context.importSceneFile(filePath);
            result.resultLine = imported
                ? QString("// Result: imported %1 //").arg(filePath)
                : "// Error: import failed //";
            return true;
        }

        result.resultLine = "// Error: unsupported file syntax //";
        return true;
    }

    const struct CommandMapEntry {
        const char* commandName;
        PrimitiveMeshFactory::Type type;
    } primitiveCommands[] = {
        { "polySphere", PrimitiveMeshFactory::Type::Sphere },
        { "polyCube", PrimitiveMeshFactory::Type::Cube },
        { "polyCylinder", PrimitiveMeshFactory::Type::Cylinder },
        { "polyCone", PrimitiveMeshFactory::Type::Cone },
        { "polyTorus", PrimitiveMeshFactory::Type::Torus },
        { "polyPlane", PrimitiveMeshFactory::Type::Plane },
        { "polyDisc", PrimitiveMeshFactory::Type::Disc },
        { "polyPyramid", PrimitiveMeshFactory::Type::Pyramid },
        { "polyPrism", PrimitiveMeshFactory::Type::Prism }
    };

    for (const CommandMapEntry& entry : primitiveCommands) {
        if (command != entry.commandName) {
            continue;
        }

        const QString objectName = context.createPrimitive ? context.createPrimitive(entry.type) : QString();
        if (objectName.isEmpty()) {
            result.resultLine = "// Error: primitive creation failed //";
            return true;
        }

        result.resultLine = QString("// Result: %1 %2Shape //").arg(objectName, objectName);
        return true;
    }

    result.resultLine = QString("// Error: unsupported command '%1' //").arg(command);
    return true;
}
