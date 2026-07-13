#pragma once

#include <QList>
#include <QString>
#include <QVector3D>

#include <functional>

#include "scene/PrimitiveMeshFactory.h"

struct ScriptCommandExecution
{
    bool handled = false;
    QString resultLine;
};

struct ScriptCommandContext
{
    std::function<void()> clearSelection;
    std::function<bool(const QString& objectName)> selectObjectByName;
    std::function<QString(PrimitiveMeshFactory::Type type)> createPrimitive;
    std::function<QString(const QString& name)> createJoint;
    std::function<QString(const QString& sourceName, const QString& newName)> renameObject;
    std::function<QString(const QString& sourceName)> duplicateObject;
    std::function<QString(const QString& sourceName)> groupObject;
    std::function<bool(const QString& sourceName)> deleteObject;
    std::function<bool(const QString& childName, const QString& parentName)> parentObject;
    std::function<bool(const QString& childName)> unparentObject;
    std::function<bool(const QString& objectName, const QVector3D& eulerDegrees)> setJointOrientation;
    std::function<bool(const QString& objectName)> resetJointOrientation;
    std::function<bool(const QString& objectName)> alignJointOrientationToChild;
    std::function<bool(const QString& objectName, bool recursive)> captureBindPose;
    std::function<bool(const QString& objectName, const QString& attributeName, const QList<double>& values)> setAttribute;
    std::function<void()> newScene;
    std::function<bool(const QString& filePath)> openSceneFile;
    std::function<bool(const QString& filePath)> importSceneFile;
    std::function<bool(const QString& filePath)> saveSceneFile;
    std::function<bool(const QString& targetName)> frameView;
    std::function<void()> resetCamera;
    std::function<bool(const QString& toolName)> activateTool;
    std::function<void(int frame)> setCurrentFrame;
    std::function<bool(const QString& objectName, int frame)> setKeyframe;
    std::function<bool(const QString& objectName, int frame)> deleteKeyframe;
    std::function<void(bool enabled)> setAutoKey;
    std::function<void(int startFrame, int endFrame)> setPlaybackRange;
    std::function<void(bool playing)> setPlaybackState;
};

class ScriptCommandRegistry
{
public:
    bool execute(QString commandLine, const ScriptCommandContext& context, ScriptCommandExecution* execution = nullptr) const;
};
