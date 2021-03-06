#include <osg/Group>
#include <osgEarthUtil/ObjectPlacer>
#include "maplayergroup.h"
#include "../nodemaplayer.h"
#include "updraft.h"
#include "scenemanager.h"
#include "ui/mainwindow.h"

namespace Updraft {
namespace Core {

MapLayerGroup::MapLayerGroup(const QString &title)
  : MapLayer(title), nodeGroup(new osg::Group) {}

MapLayerGroup::MapLayerGroup(QTreeWidgetItem *item)
  : MapLayer(item), nodeGroup(new osg::Group) {}

MapLayerGroup::~MapLayerGroup() {
  foreach(MapLayerInterface* layer, mapLayers) {
    removeMapLayer(layer);
    delete layer;
  }

  // The following code must be in the same class that defines
  // removeFromScene instead of in the common ancestor,
  // because of how virtual calls are disabled in destructors.
  // Pure magic :-)
  if (parent) {
    parent->removeMapLayer(this);
  }
}

MapLayerInterface* MapLayerGroup::createMapLayer
  (osg::Node* mapLayer, const QString& title, int pos) {
  MapLayerInterface* layer = new NodeMapLayer(title, mapLayer);
  insertMapLayer(layer, pos);
  return layer;
}

MapLayerGroupInterface* MapLayerGroup::createMapLayerGroup(
  const QString &title, int pos) {
  MapLayerGroup* group = new MapLayerGroup(title);
  insertMapLayer(group, pos);
  return group;
}

void MapLayerGroup::insertMapLayer(MapLayerInterface* layer, int pos) {
  if (pos > 0) {
    treeItem->insertChild(pos, layer->getTreeItem());
  } else {
    treeItem->addChild(layer->getTreeItem());
  }
  mapLayers.insert(layer);
  layer->addToScene(this);
  layer->inserted(this);
  if ((treeItem->data(0, Qt::CheckStateRole).isValid() &&
    treeItem->checkState(0) == Qt::Unchecked) ||
    treeItem->isDisabled()) {
    layer->setDisabled(true);
  }

  updraft->mainWindow->registerMapLayer(layer);

  treeItem->setHidden(!mapLayers.count());
  if (mapLayers.count() == 1 &&
    treeItem->data(0, Qt::CheckStateRole).isValid()) {
    treeItem->setCheckState(0, Qt::Checked);
  }
}

// TODO(Kuba): Create a specialised subclass of MapLayer and avoid code
// duplication within this method.
MapLayerInterface* MapLayerGroup::createMapLayerNoInsert(osg::Node* mapLayer,
  const QString& title, int pos) {
  MapLayerInterface* layer = new NodeMapLayer(title, mapLayer);

  if (pos > 0) {
    treeItem->insertChild(pos, layer->getTreeItem());
  } else {
    treeItem->addChild(layer->getTreeItem());
  }
  mapLayers.insert(layer);
  // don't add
  layer->inserted(this);
  if ((treeItem->data(0, Qt::CheckStateRole).isValid() &&
    treeItem->checkState(0) == Qt::Unchecked) ||
    treeItem->isDisabled()) {
    layer->setDisabled(true);
  }

  updraft->mainWindow->registerMapLayer(layer);

  treeItem->setHidden(!mapLayers.count());
  if (mapLayers.count() == 1 &&
    treeItem->data(0, Qt::CheckStateRole).isValid()) {
    treeItem->setCheckState(0, Qt::Checked);
  }

  return layer;
}

void MapLayerGroup::removeMapLayer(MapLayerInterface* layer) {
  treeItem->removeChild(layer->getTreeItem());
  mapLayers.remove(layer);
  updraft->mainWindow->unregisterMapLayer(layer);
  layer->removeFromScene(this);
  // If this layer was added using insertExistingMapLayer, then
  // the previous call should silently fail.

  layer->inserted(0);

  treeItem->setHidden(!mapLayers.count());
}

osg::Group* MapLayerGroup::getNodeGroup() {
  return nodeGroup;
}

osgEarth::Util::ObjectPlacer* MapLayerGroup::getObjectPlacer() {
  return updraft->sceneManager->getObjectPlacer();
}

void MapLayerGroup::setVisibility(bool value) {
  foreach(MapLayerInterface* layer, mapLayers) {
    layer->setDisabled(!value);
  }

  nodeGroup->setNodeMask(value ? 0xffffffff : 0x0);
}

void MapLayerGroup::setDisabled(bool disabled) {
  foreach(MapLayerInterface* layer, mapLayers) {
    layer->setDisabled(disabled);
  }
  MapLayer::setDisabled(disabled);
}

bool MapLayerGroup::isVisible() {
  return nodeGroup->getNodeMask() != 0x0;
}

void MapLayerGroup::addToScene(MapLayerGroup* group) {
  group->getNodeGroup()->addChild(nodeGroup);
}

void MapLayerGroup::removeFromScene(MapLayerGroup* group) {
  group->getNodeGroup()->removeChild(nodeGroup);
}

void MapLayerGroup::inserted(Core::MapLayerGroup* parent) {
  MapLayer::inserted(parent);

  treeItem->setExpanded(true);
  treeItem->setHidden(!mapLayers.count());
}

QByteArray MapLayerGroup::saveState() {
  QByteArray array;
  QDataStream stream(&array, QIODevice::WriteOnly);

  foreach(MapLayerInterface* layer, mapLayers) {
    stream << layer->getId();
    stream << layer->saveState();
  }

  stream << QByteArray();
  stream << MapLayer::saveState();

  return array;
}

bool MapLayerGroup::restoreState(const QByteArray &state) {
  QDataStream stream(state);

  QMap<QByteArray, MapLayerInterface*> map;
  foreach(MapLayerInterface* layer, mapLayers) {
    map[layer->getId()] = layer;
  }

  QByteArray id;  // yes, this shadows this->id.
  stream >> id;

  while (!id.isNull() && !stream.atEnd()) {
    QByteArray childState;
    stream >> childState;

    if (map.contains(id)) {
      bool result = map[id]->restoreState(childState);
      if (!result) {
        return false;
      }
    }

    stream >> id;
  };

  QByteArray ownState;
  stream >> ownState;

  return MapLayer::restoreState(ownState);
}

const osg::Node* MapLayerGroup::getNode() {
  return getNodeGroup();
}

}  // End namespace Core
}  // End namespace Updraft
