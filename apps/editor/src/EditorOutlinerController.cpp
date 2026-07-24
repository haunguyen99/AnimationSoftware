#include "EditorOutlinerController.h"

#include <QSignalBlocker>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>

#include "EditorSceneQueryController.h"

namespace
{
using EditorSceneQueryController::objectDisplayName;

bool shouldPromoteNode(const EditorOutlinerController::SceneAccess& sceneAccess, std::uint64_t objectId)
{
    const SceneObject* object = sceneAccess.findObject(objectId);
    if (object == nullptr) {
        return false;
    }

    return object->name() == "RootNode"
        && object->meshHandles().isEmpty()
        && object->childIds().size() == 1;
}

void populateItem(
    QTreeWidget* outlinerTree,
    QTreeWidgetItem* parentItem,
    std::uint64_t objectId,
    const EditorOutlinerController::SceneAccess& sceneAccess)
{
    const SceneObject* object = sceneAccess.findObject(objectId);
    if (object == nullptr) {
        return;
    }

    if (shouldPromoteNode(sceneAccess, objectId)) {
        for (SceneObject::Id childId : object->childIds()) {
            populateItem(outlinerTree, parentItem, childId, sceneAccess);
        }
        return;
    }

    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, objectDisplayName(*object));
    item->setData(0, Qt::UserRole, QVariant::fromValue<qulonglong>(objectId));

    if (parentItem == nullptr) {
        outlinerTree->addTopLevelItem(item);
    } else {
        parentItem->addChild(item);
    }

    for (SceneObject::Id childId : object->childIds()) {
        populateItem(outlinerTree, item, childId, sceneAccess);
    }
}
}

namespace EditorOutlinerController
{
void populateTree(QTreeWidget* outlinerTree, const SceneAccess& sceneAccess)
{
    if (outlinerTree == nullptr || !sceneAccess.rootObjectIds || !sceneAccess.findObject) {
        return;
    }

    outlinerTree->clear();

    for (SceneObject::Id rootId : sceneAccess.rootObjectIds()) {
        populateItem(outlinerTree, nullptr, rootId, sceneAccess);
    }

    outlinerTree->expandToDepth(1);

    if (outlinerTree->topLevelItemCount() == 0) {
        QTreeWidgetItem* emptyItem = new QTreeWidgetItem();
        emptyItem->setText(0, "No scene loaded");
        outlinerTree->addTopLevelItem(emptyItem);
    }
}

void syncSelection(QTreeWidget* outlinerTree, std::uint64_t objectId)
{
    if (outlinerTree == nullptr) {
        return;
    }

    QSignalBlocker blocker(outlinerTree);

    if (objectId == 0) {
        outlinerTree->clearSelection();
        return;
    }

    for (QTreeWidgetItemIterator it(outlinerTree); *it != nullptr; ++it) {
        QTreeWidgetItem* item = *it;
        if (item->data(0, Qt::UserRole).toULongLong() == objectId) {
            outlinerTree->setCurrentItem(item);
            item->setSelected(true);
            return;
        }
    }

    outlinerTree->clearSelection();
}

std::uint64_t selectedObjectId(const QTreeWidget* outlinerTree)
{
    if (outlinerTree == nullptr || outlinerTree->selectedItems().isEmpty()) {
        return 0;
    }

    const QVariant objectIdData = outlinerTree->selectedItems().first()->data(0, Qt::UserRole);
    return objectIdData.isValid() ? objectIdData.toULongLong() : 0;
}

QString selectedObjectLabel(const QTreeWidget* outlinerTree)
{
    if (outlinerTree == nullptr || outlinerTree->selectedItems().isEmpty()) {
        return {};
    }

    return outlinerTree->selectedItems().first()->text(0);
}
}
